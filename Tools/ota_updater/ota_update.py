#!/usr/bin/env python3
"""
OpenNerve IPG OTA Firmware Update Tool

Performs a complete over-the-air firmware update on an OpenNerve Gen2 IPG over BLE:
  1. Scans for the device and connects
  2. Authenticates as Admin (ECDSA P-256)
  3. Authorizes the firmware image with a signed SHA-256 hash
  4. Downloads the image in 128-byte packets
  5. Triggers the BSL to verify, flash, and reboot

Usage:
    python ota_update.py <firmware.bin> <admin_priv_d_hex.txt> [--device-name NAME]

The private key file must contain exactly 64 hex characters (the 32-byte ECDSA P-256
'd' parameter, no spaces or colons). See Docs/encryption-instructions.md for details.

Requirements:
    pip install -r requirements.txt
"""

import argparse
import asyncio
import hashlib
import math
import os
import struct
import sys
import crcmod
from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError
from bleak.backends.device import BLEDevice
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature

# ── Windows auto-pairing ─────────────────────────────────────────────────────

async def _pair_windows(device: "BLEDevice") -> None:
    """Windows only: pair via the WinRT custom-pairing API, injecting the
    passkey automatically so no system dialog is required.

    Mirrors the Windows App's ConnectAsync() pairing flow exactly:
      PairAsync(ProvidePin) → Accept("000000") → wait 500 ms
    """
    try:
        import winrt.windows.devices.bluetooth as _bt
        import winrt.windows.devices.enumeration as _enum
    except ImportError:
        # WinRT modules unavailable (shouldn't happen when bleak is installed on
        # Windows, but fall back gracefully to manual pairing if so).
        return

    bt_addr = int(device.address.replace(":", "").replace("-", ""), 16)
    ble_dev = await _bt.BluetoothLEDevice.from_bluetooth_address_async(bt_addr)
    if ble_dev is None:
        return

    pairing = ble_dev.device_information.pairing

    if pairing.is_paired:
        return  # already bonded — connection will be encrypted immediately

    custom = pairing.custom

    def _on_pairing_requested(sender, args):
        if args.pairing_kind == _enum.DevicePairingKinds.PROVIDE_PIN:
            args.accept("000000")
        else:
            args.accept()

    custom.pairing_requested += _on_pairing_requested

    try:
        result = await custom.pair_async(_enum.DevicePairingKinds.PROVIDE_PIN)
    finally:
        try:
            custom.pairing_requested -= _on_pairing_requested
        except Exception:
            pass

    if result.status not in (
        _enum.DevicePairingResultStatus.PAIRED,
        _enum.DevicePairingResultStatus.ALREADY_PAIRED,
    ):
        raise RuntimeError(
            f"Windows auto-pairing failed with status: {result.status}\n"
            "Ensure the IPG is in BLE advertising mode and retry."
        )

    await asyncio.sleep(0.5)  # ATT/GATT stabilisation (per Windows App)

# ── BLE UUIDs (Nordic UART Service) ──────────────────────────────────────────
NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
NUS_RX_CHAR_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  # write: client → device
NUS_TX_CHAR_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  # notify: device → client

# ── Opcodes (from app_func_command.h) ─────────────────────────────────────────
OP_AUTH              = 0xF0
OP_AUTH_FW_IMAGE     = 0xF3
OP_DOWNLOAD_FW_IMAGE = 0xF4
OP_VERIFY_FW_IMAGE   = 0xF5

# ── Status codes ──────────────────────────────────────────────────────────────
STATUS_SUCCESS         = 0x00
STATUS_INVALID         = 0x01
STATUS_CRC_ERR         = 0xF0
STATUS_PAYLOAD_LEN_ERR = 0xF1
STATUS_OPCODE_ERR      = 0xF2
STATUS_USER_CLASS_ERR  = 0xF3

USER_CLASS_ADMIN = 0xFF

# ── Protocol constants ────────────────────────────────────────────────────────
FRAM_MAX_SIZE       = 262_144   # 256 KB maximum firmware image size
CHUNK_SIZE          = 128       # bytes per download packet (OP_DOWNLOAD_FW_IMAGE)
CMD_TIMEOUT_S       = 10.0      # seconds to wait for a response
VERIFY_TIMEOUT_S    = 30.0      # verify can be slow (device computes SHA-256 of FRAM)
MAX_VERIFY_ATTEMPTS = 3
DEFAULT_DEVICE_NAME = "CARSS"

# OP_AUTH payload structure (from app_mode_ble_active.c):
#   ECDSA_Data_t (96 bytes) + fw_version (6 bytes) + ble_id (4 bytes) = 106 bytes
# The firmware checks payload length == sizeof(ECDSA_Data_t) + datalen_fw_version + datalen_ble_id.
# We send zeroed padding for fw_version and ble_id (only stored by the device for
# Clinician/Patient auth; ignored for Admin).
AUTH_FW_VERSION_LEN = 6
AUTH_BLE_ID_LEN     = 4

# ── CRC-CCITT-16 (poly=0x1021, init=0xFFFF, no inversion) ────────────────────
# Computed over [Opcode][PayloadLen][Payload...], appended as little-endian uint16.
_crc16 = crcmod.mkCrcFun(0x11021, initCrc=0xFFFF, rev=False, xorOut=0x0000)

_STATUS_NAMES = {
    STATUS_SUCCESS:         "SUCCESS",
    STATUS_INVALID:         "INVALID",
    STATUS_CRC_ERR:         "CRC_ERROR",
    STATUS_PAYLOAD_LEN_ERR: "PAYLOAD_LEN_ERROR",
    STATUS_OPCODE_ERR:      "OPCODE_ERROR",
    STATUS_USER_CLASS_ERR:  "USER_CLASS_ERROR",
}


def _status_name(code: int) -> str:
    return _STATUS_NAMES.get(code, f"0x{code:02X}")


# ── Packet building / parsing ─────────────────────────────────────────────────

def _build_packet(opcode: int, payload: bytes) -> bytes:
    header = bytes([opcode, len(payload)])
    crc = _crc16(header + payload)
    return header + payload + struct.pack("<H", crc)


def _parse_response(data: bytes) -> tuple[int, int, bytes]:
    """
    Parse a response packet. Returns (opcode, status, payload).
    Raises ValueError on CRC mismatch or malformed packet.
    """
    if len(data) < 5:
        raise ValueError(f"Response too short: {len(data)} bytes (minimum 5)")
    #else:
        #print(" [dbg] parsing length: " + str(len(data)))
    opcode   = data[0]
    pay_len  = data[1]
    status   = data[2]
    if (pay_len > 0):
        payload  = data[3 : 3 + pay_len]
    else:
        payload = b'\0x00'
    crc_data = data[3 + pay_len : 3 + pay_len + 2]
    #print(" [dbg] payload: " + str(payload))
    if len(crc_data) < 2:
        raise ValueError("Response truncated: CRC bytes missing")
    expected = _crc16(data[: 3 + pay_len])
    received = struct.unpack("<H", crc_data)[0]
    if expected != received:
        #print(" [dbg] CRC expected: {expected} vs received: {received}")
        raise ValueError(
            f"CRC mismatch: expected 0x{expected:04X}, received 0x{received:04X}"
        )
    return opcode, status, payload


# ── Crypto helpers ────────────────────────────────────────────────────────────

def _load_private_key(path: str) -> ec.EllipticCurvePrivateKey:
    """Load an ECDSA P-256 private key from a 64-hex-char file."""
    with open(path) as f:
        hex_str = f.read().strip().replace(":", "").replace(" ", "").lower()
        #print(" [dbg] hex string: " + hex_str)
    if len(hex_str) != 64:
        raise ValueError(
            f"Private key file must contain exactly 64 hex characters (32 bytes). "
            f"Got {len(hex_str)} characters in '{path}'."
        )
    d = int(hex_str, 16)
    return ec.derive_private_key(d, ec.SECP256R1())


def _sign_message(private_key: ec.EllipticCurvePrivateKey, data: bytes) -> tuple[bytes, bytes, bytes]:
    """
    Sign `data` using ECDSA P-256 + SHA-256.

    Returns (r_bytes, s_bytes, hash_msg) where:
      - r_bytes, s_bytes are the 32-byte big-endian ECDSA signature components
      - hash_msg is SHA-256(data), the hash the firmware will verify the signature against

    The IPG's PKA hardware verifies ECDSA directly against the hash (no re-hashing).
    Using sign(data, ECDSA(SHA256)) ensures R,S is valid for SHA-256(data) = hash_msg.
    """
    sig_der = private_key.sign(data, ec.ECDSA(hashes.SHA256()))
    r, s = decode_dss_signature(sig_der)
    hash_msg = hashlib.sha256(data).digest()
    return r.to_bytes(32, "big"), s.to_bytes(32, "big"), hash_msg



# ── BLE session ───────────────────────────────────────────────────────────────

class OTASession:
    def __init__(self, client: BleakClient, private_key: ec.EllipticCurvePrivateKey):
        self._client = client
        self._private_key = private_key
        self._rx_queue: asyncio.Queue[bytes] = asyncio.Queue()

    def _on_notify(self, _sender: object, data: bytearray) -> None:
        #print(f"  [dbg] notification received: {bytes(data).hex()}")
        self._rx_queue.put_nowait(bytes(data))
        #print("[dbg]Put byte in queue")

    async def _reinitialize(self) -> None:
        """Re-subscribe to TX notifications after BLE pairing completes.

        CoreBluetooth invalidates its GATT service cache when pairing finishes
        and the link becomes encrypted.  bleak raises 'Service Discovery has not
        been performed yet' until the re-discovery completes.  We poll until
        start_notify succeeds (which also re-enables the CCCD the device may
        have reset during pairing).
        """
        #print("  [dbg] Pairing completed — waiting for GATT re-discovery...")
        try:
            await self._client.stop_notify(NUS_TX_CHAR_UUID)
        except Exception:
            pass
        for attempt in range(20):          # up to 10 s
            await asyncio.sleep(0.5)
            try:
                await self._client.start_notify(NUS_TX_CHAR_UUID, self._on_notify)
                #print("  [dbg] Re-subscribed to TX notifications.")
                return
            except Exception as e:
                err = str(e).lower()
                #print(f"  [dbg] start_notify attempt {attempt + 1}: {type(e).__name__}: {e}")
                # Explicit disconnect errors → device disconnected after pairing (expected)
                if any(k in err for k in ("disconnect", "not connected", "peripheral is not connected")):
                    raise _PairingDisconnectedError()
        # All attempts exhausted — device must have disconnected silently
        raise _PairingDisconnectedError()

    async def start(self) -> None:
        # Enumerate discovered services so we can confirm NUS is present
        services = self._client.services
        nus_service = services.get_service(NUS_SERVICE_UUID)
        if nus_service is None:
            raise RuntimeError(
                f"NUS service {NUS_SERVICE_UUID} not found on device.\n"
                f"Discovered services: {[str(s.uuid) for s in services]}"
            )
        tx_char = nus_service.get_characteristic(NUS_TX_CHAR_UUID)
        if tx_char is None:
            chars = [str(c.uuid) for c in nus_service.characteristics]
            raise RuntimeError(
                f"NUS TX characteristic {NUS_TX_CHAR_UUID} not found.\n"
                f"Characteristics on NUS service: {chars}"
            )
        #print(f"  [dbg] NUS TX properties: {tx_char.properties}")
        await self._client.start_notify(NUS_TX_CHAR_UUID, self._on_notify)
        #print(f"  [dbg] Subscribed to TX notifications. Settling...")
        await asyncio.sleep(1.0)
        #print(f"  [dbg] Ready.")

    async def _send(self, opcode: int, payload: bytes, timeout: float = CMD_TIMEOUT_S) -> tuple[int, bytes]:
        """
        Send a command and return (status, response_payload).
        Raises on timeout, CRC mismatch, or opcode mismatch.
        """
        packet = _build_packet(opcode, payload)
        #print(f"  [dbg] sending opcode=0x{opcode:02X} payload_len={len(payload)} packet={packet[:8].hex()}...")
        try:
            await self._client.write_gatt_char(NUS_RX_CHAR_UUID, packet, response=False)
        except BleakError as e:
            if "service discovery" in str(e).lower():
                # Pairing just completed and CoreBluetooth invalidated the GATT cache.
                # Reinitialize, then retry the write.
                await self._reinitialize()
                await self._client.write_gatt_char(NUS_RX_CHAR_UUID, packet, response=False)
            else:
                raise
        except asyncio.TimeoutError:
        # was delivered successfully.  The device may have already sent its
        # notification (visible in the queue), so fallthrough to the read.
            print("Asyncio error")
            pass
        try:
            raw = await asyncio.wait_for(self._rx_queue.get(), timeout=timeout)
            #print(" [dbg] received message from queue: " + str(raw))
        except asyncio.TimeoutError:
            print("asyncio.Timeout error!")
            raise TimeoutError(
                f"No response to opcode 0x{opcode:02X} within {timeout:.0f}s"
            )
        #print("  [dbg] parsing response")
        resp_opcode, status, resp_payload = _parse_response(raw)
        #print(" [dbg] parsed response, opcode: " + str(resp_opcode))
        if resp_opcode != opcode:
            raise ValueError(
                f"Opcode mismatch: sent 0x{opcode:02X}, received 0x{resp_opcode:02X}"
            )
        #print(" [dbg] status: " + str(status) + ", response: " + str(resp_payload))
        return status, resp_payload

    async def authenticate_admin(self) -> None:
        """
        Step 3: Authenticate as Admin using OP_AUTH (0xF0).

        Payload (106 bytes):
          R(32) + S(32) + HashMsg(32) + fw_version_padding(6) + ble_id_padding(4)

        The firmware verifies the ECDSA signature and transitions to BLE_CONN state.

        On macOS the IPG requests BLE pairing (passkey 000000) when we write to the
        NUS RX characteristic.  The write-without-response packet sent before the
        encrypted link is established is silently dropped, so we retry every few
        seconds until the pairing dialog is dismissed and the link is encrypted.
        """
        # Sign once; reuse the same payload for all retries.
        random_data = os.urandom(32)
        r_bytes, s_bytes, hash_msg = _sign_message(self._private_key, random_data)
        payload = (
            r_bytes + s_bytes + hash_msg
            + b"\x00" * AUTH_FW_VERSION_LEN
            + b"\x00" * AUTH_BLE_ID_LEN
        )

        AUTH_RETRY_INTERVAL = 3.0   # seconds between retries while waiting for pairing
        AUTH_TOTAL_TIMEOUT  = 90.0  # total seconds before giving up

        loop = asyncio.get_running_loop()
        deadline = loop.time() + AUTH_TOTAL_TIMEOUT
        attempt = 0
        status, resp = None, None
        while True:
            attempt += 1
            try:
                #print(" [dbg] auth attempt " + str(attempt))
                #print(" [dbg] payload: " + str(payload))
                status, resp = await self._send(OP_AUTH, payload, timeout=AUTH_RETRY_INTERVAL)
                #print(" [dbg] status in Auth_admin: " + str(status))
                break  # got a response — pairing complete and command processed
            except TimeoutError:
                if loop.time() >= deadline:
                    raise TimeoutError(
                        f"No response to OP_AUTH after {AUTH_TOTAL_TIMEOUT:.0f}s "
                        f"({attempt} attempts).\n"
                        "Ensure the IPG is in BLE advertising mode and enter "
                        "passkey 000000 when the system pairing dialog appears."
                    )
                print(f"  (attempt {attempt} — waiting for passkey entry if prompted, retrying...)")

        if status != STATUS_SUCCESS:
            if status == STATUS_INVALID:
                raise RuntimeError(
                    "Admin authentication failed: ECDSA signature rejected.\n"
                    "Verify you are using the correct admin private key."
                )
            if status == STATUS_USER_CLASS_ERR:
                raise RuntimeError(
                    "Admin authentication failed: device rejected the opcode.\n"
                    "Ensure the IPG is in BLE advertising mode (not already connected)."
                )
            raise RuntimeError(
                f"Admin authentication failed with status {_status_name(status)}"
            )
        user_class = resp[0] if resp else 0x00
        if user_class != USER_CLASS_ADMIN:
            raise RuntimeError(
                f"Authenticated but received unexpected user class 0x{user_class:02X} "
                f"(expected Admin 0xFF). Check that you're using the Admin key."
            )

    async def authorize_firmware(self, firmware_bytes: bytes) -> tuple[bytes, bytes, bytes]:
        """
        Step 4: Authorize firmware image with OP_AUTH_FW_IMAGE (0xF3).

        Payload (96 bytes): R(32) + S(32) + SHA-256(firmware)(32)

        The firmware verifies the ECDSA signature and saves the hash for post-download
        verification. Returns (r_bytes, s_bytes, firmware_hash) for potential retry.
        """
        r_bytes, s_bytes, firmware_hash = _sign_message(self._private_key, firmware_bytes)
        payload = r_bytes + s_bytes + firmware_hash
        status, _ = await self._send(OP_AUTH_FW_IMAGE, payload)
        if status != STATUS_SUCCESS:
            if status == STATUS_INVALID:
                raise RuntimeError(
                    "Firmware image authorization failed: ECDSA signature rejected.\n"
                    "Ensure you're using the correct .bin file and the matching private key."
                )
            if status == STATUS_USER_CLASS_ERR:
                raise RuntimeError(
                    "Firmware image authorization failed: not authenticated as Admin.\n"
                    "Reconnect and re-authenticate before retrying."
                )
            raise RuntimeError(
                f"OP_AUTH_FW_IMAGE failed with status {_status_name(status)}"
            )
        return r_bytes, s_bytes, firmware_hash

    async def download_firmware(self, firmware_bytes: bytes) -> None:
        """
        Step 5: Download firmware in 128-byte chunks using OP_DOWNLOAD_FW_IMAGE (0xF4).

        Packet payload (132 bytes): offset(4 LE) + data(128)
        The last chunk is zero-padded to 128 bytes.
        Each packet is retried once on failure before aborting.
        """
        image_size = len(firmware_bytes)
        total_packets = math.ceil(image_size / CHUNK_SIZE)
        offset = 0
        packet_num = 0
        while offset < image_size:
            chunk = firmware_bytes[offset : offset + CHUNK_SIZE]
            chunk = chunk.ljust(CHUNK_SIZE, b"\x00")
            pkt_payload = struct.pack("<I", offset) + chunk
            for attempt in range(2):
                status, _ = await self._send(OP_DOWNLOAD_FW_IMAGE, pkt_payload)
                if status == STATUS_SUCCESS:
                    break
                if attempt == 0:
                    print(
                        f"\n  Packet {packet_num} (offset {offset}) failed "
                        f"({_status_name(status)}), retrying..."
                    )
                else:
                    raise RuntimeError(
                        f"Download failed at byte offset {offset} "
                        f"after retry: {_status_name(status)}"
                    )
            offset += CHUNK_SIZE
            packet_num += 1
            _print_progress(packet_num, total_packets)
        print()  # newline after progress bar

    async def verify_firmware(self, image_size: int) -> None:
        """
        Step 6: Verify the downloaded image with OP_VERIFY_FW_IMAGE (0xF5).

        Payload (4 bytes): image_size as uint32 LE.
        On success, the device immediately begins BSL flashing and reboots.
        """
        payload = struct.pack("<I", image_size)
        status, _ = await self._send(OP_VERIFY_FW_IMAGE, payload, timeout=VERIFY_TIMEOUT_S)
        if status != STATUS_SUCCESS:
            fail_count = resp[0] if resp else "?"
            raise _VerifyFailedError(
                f"Image verification failed (device failure count: {fail_count})."
            )


class _VerifyFailedError(RuntimeError):
    pass

class _PairingDisconnectedError(Exception):
    """Raised when the device disconnects after BLE pairing (expected nRF behaviour).
    The caller should reconnect; the second connection will be bonded/encrypted."""
    pass


# ── Device discovery ──────────────────────────────────────────────────────────

async def _discover_device(device_name: str) -> BLEDevice:
    print(f"Scanning for BLE devices matching '{device_name}'...")
    devices = await BleakScanner.discover(timeout=5.0)
    matches = [d for d in devices if d.name and device_name.lower() in d.name.lower()]
    if not matches:
        raise RuntimeError(
            f"No BLE device matching '{device_name}' found.\n"
            f"Ensure the IPG is powered on and in BLE advertising mode."
        )
    if len(matches) == 1:
        d = matches[0]
        print(f"Found: {d.name} ({d.address})")
        return d
    print(f"Found {len(matches)} matching devices:")
    for i, d in enumerate(matches):
        print(f"  [{i + 1}] {d.name} ({d.address})")
    while True:
        raw = input("Select device number: ").strip()
        if raw.isdigit() and 1 <= int(raw) <= len(matches):
            return matches[int(raw) - 1]
        print("  Invalid — enter a number from the list above.")


# ── Progress bar ──────────────────────────────────────────────────────────────

def _print_progress(done: int, total: int, width: int = 40) -> None:
    pct = done / total
    filled = int(width * pct)
    bar = "█" * filled + "░" * (width - filled)
    print(f"\r  [{bar}] {pct * 100:5.1f}%  {done}/{total} packets", end="", flush=True)


# ── Main OTA flow ─────────────────────────────────────────────────────────────

async def run_ota(firmware_path: str, key_path: str, device_name: str) -> None:
    print("=== OpenNerve IPG OTA Firmware Update ===\n")

    # ── Pre-flight checks ─────────────────────────────────────────────────────
    if not os.path.isfile(firmware_path):
        raise FileNotFoundError(f"Firmware file not found: {firmware_path}")
    firmware_bytes = open(firmware_path, "rb").read()
    image_size = len(firmware_bytes)
    if image_size == 0:
        raise ValueError("Firmware file is empty.")
    if image_size > FRAM_MAX_SIZE:
        raise ValueError(
            f"Firmware too large: {image_size:,} bytes "
            f"(maximum {FRAM_MAX_SIZE:,} bytes / 256 KB)."
        )
    firmware_hash = hashlib.sha256(firmware_bytes).digest()
    total_packets = math.ceil(image_size / CHUNK_SIZE)

    print(f"Firmware:  {os.path.abspath(firmware_path)}")
    print(f"Size:      {image_size:,} bytes  ({image_size / 1024:.1f} KB)")
    print(f"SHA-256:   {firmware_hash.hex()}")
    print(f"Packets:   {total_packets}  ({CHUNK_SIZE} bytes each)\n")

    private_key = _load_private_key(key_path)
    print(f"Key file:  {os.path.abspath(key_path)}  [OK]\n")

    # ── Connect (with automatic reconnect after first-time BLE pairing) ─────────
    # On macOS the nRF52810 disconnects after pairing completes so the central
    # can reconnect on the newly-encrypted bonded link.  We allow one reconnect.
    device = await _discover_device(device_name)

    for connect_attempt in range(1, 3):
        if connect_attempt == 1:
            print(f"Connecting to {device.name} ({device.address})...")
            if sys.platform == "win32":
                print("  Pairing (passkey injected automatically)...")
                await _pair_windows(device)
                print("  Paired.")
            else:
                print("  Note: if a pairing dialog appears, enter passkey: 000000")
        else:
            print(f"\n  Pairing complete. Waiting for device to re-advertise...")
            await asyncio.sleep(5.0)   # give the nRF time to restart advertising
            device = await _discover_device(device_name)  # fresh scan by name

        try:
            async with BleakClient(device, timeout=30.0) as client:
                if not client.is_connected:
                    raise RuntimeError(f"Failed to connect to {device.name} ({device.address})")
                print(f"Connected to {device.name} ({device.address})\n")

                # Best-effort explicit pair() — on macOS this may return immediately
                # but at least primes the CoreBluetooth state machine.
                try:
                    await asyncio.wait_for(client.pair(), timeout=5.0)
                except Exception:
                    pass

                session = OTASession(client, private_key)
                await session.start()

                # ── Step 3: Admin auth ────────────────────────────────────────────
                print("[1/4] Authenticating as Admin...")
                await session.authenticate_admin()
                print("      Admin authentication OK.\n")

                # ── Step 4: Authorize image + steps 5-6 with retry loop ───────────
                for verify_attempt in range(1, MAX_VERIFY_ATTEMPTS + 1):
                    if verify_attempt == 1:
                        print("[2/4] Authorizing firmware image...")
                    else:
                        print(f"\n[2/4] Re-authorizing firmware image (attempt {verify_attempt}/{MAX_VERIFY_ATTEMPTS})...")
                    await session.authorize_firmware(firmware_bytes)
                    print("      Firmware image authorized. Device entered OAD mode.\n")

                    print(f"[3/4] Downloading firmware ({image_size:,} bytes, {total_packets} packets)...")
                    await session.download_firmware(firmware_bytes)
                    print("      Download complete.\n")

                    print("[4/4] Verifying firmware image (device computing SHA-256)...")
                    try:
                        await session.verify_firmware(image_size)
                        break
                    except _VerifyFailedError as e:
                        if verify_attempt == MAX_VERIFY_ATTEMPTS:
                            raise RuntimeError(
                                f"{e}\n"
                                f"All {MAX_VERIFY_ATTEMPTS} verify attempts failed. "
                                "The device has returned to normal BLE connected mode.\n"
                                "Check the firmware binary and retry from the beginning."
                            )
                        print(f"      {e} Retransmitting...\n")

                print("      Verification passed!")
                print("\n  The device is now flashing the new firmware and will reboot.")
                print("  The BLE connection will drop momentarily — this is expected.")
                print("  Wait ~5 seconds, then reconnect to confirm the firmware version.\n")
                break  # success — no reconnect needed

        except _PairingDisconnectedError:
            if connect_attempt < 2:
                continue   # reconnect loop handles this
            raise RuntimeError(
                "Device disconnected after pairing but failed to reconnect.\n"
                "Ensure the IPG is still powered on and retry."
            )
        except (TimeoutError, Exception) as conn_err:
            if isinstance(conn_err, (RuntimeError, ValueError, _VerifyFailedError)):
                raise
            raise RuntimeError(
                f"Connection to {device.name} ({device.address}) failed.\n"
                f"  {type(conn_err).__name__}: {conn_err}"
            ) from conn_err


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="ota_update.py",
        description="OpenNerve IPG OTA Firmware Update Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
examples:
  python ota_update.py firmware.bin secrets/admin_priv_d_hex.txt
  python ota_update.py build/FW-NIH-MCU-H2.bin ~/keys/admin_priv_d_hex.txt --device-name CARSS

private key format:
  A plain text file containing exactly 64 hexadecimal characters (no spaces, colons,
  or newlines) representing the 32-byte ECDSA P-256 'd' parameter.
  See Docs/encryption-instructions.md for key generation instructions.

notes:
  - The IPG must be in BLE advertising mode (not asleep or connected) before running.
  - The firmware .bin file must be under 256 KB. Do not use a .hex or .elf file.
  - The BLE connection will drop when the device reboots into new firmware (expected).
""",
    )
    parser.add_argument(
        "firmware",
        metavar="FIRMWARE.BIN",
        help="Path to the compiled STM32 firmware binary (.bin, not .hex)",
    )
    parser.add_argument(
        "key",
        metavar="PRIVATE_KEY",
        help="Path to the admin private key file (64 hex chars)",
    )
    parser.add_argument(
        "--device-name",
        default=DEFAULT_DEVICE_NAME,
        metavar="NAME",
        help=f"BLE device name to scan for (default: '{DEFAULT_DEVICE_NAME}')",
    )
    args = parser.parse_args()

    try:
        asyncio.run(run_ota(args.firmware, args.key, args.device_name))
        print("OTA update complete.")
    except (FileNotFoundError, ValueError, RuntimeError, OSError) as e:
        print(f"\nError: {e}", file=sys.stderr)
        sys.exit(1)
    except TimeoutError as e:
        print(f"\nTimeout: {e}", file=sys.stderr)
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n\nAborted by user.", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
