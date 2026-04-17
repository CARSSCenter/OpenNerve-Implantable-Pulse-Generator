# Over-the-Air (OTA) Firmware Update Guide
OpenNerve Gen2 IPG
March 2026

---

## Overview

The OpenNerve IPG supports wireless firmware updates for the main STM32U585 MCU over BLE, without any physical access to the device. This is the only way to update firmware on a packaged, implanted IPG.

> **Scope:** This guide covers updating the **MCU firmware only**. The nRF52810 BLE chip firmware cannot be updated wirelessly and requires a hardware programmer. Do not attempt to use this process for the BLE chip.

The update process has two stages that run in sequence:

1. **OAD (Over-the-Air Download):** Your software sends the new firmware image to the IPG over BLE. The IPG stores it in an external FRAM chip (not in flash), so the running firmware is never disturbed during this stage.

2. **BSL (Bootstrap Loader):** After the image is verified, the IPG copies it from FRAM into the inactive flash bank, verifies the copy, then swaps flash banks and reboots. The new firmware starts running automatically.

If anything goes wrong at either stage — bad data, interrupted transfer, verification failure — the device continues running its original firmware. It is not possible to brick the device through this process.

---

## Before You Begin

### What you will need

- **Admin private key** (`admin_priv_d_hex.txt` or equivalent). This is the ECDSA P-256 private key whose matching public key is baked into the IPG firmware. Without it, the device will reject any firmware update attempt. See `Docs/encryption-instructions.md` for key generation. The OpenNerve development key is available upon request.
- **STM32Cube IDE** to compile the firmware. Download free from st.com.
- **Software capable of sending BLE commands** using the OpenNerve command protocol. The OpenNerve Windows App (`OpenNerve-Windows-App` repository) is the reference implementation and already handles authentication and BLE communication.
- **A compiled firmware binary** (`.bin` file, not `.hex`) of the bug-fixed firmware. See the next section.

### Size constraint

The FRAM staging buffer holds a maximum of **262,144 bytes (256 KB)**. Your compiled firmware image must be smaller than this. Typical production builds are well under this limit, but be aware of it if you add significant new code.

---

## Step 1 — Fix the Bug and Compile

1. Open the project in STM32Cube IDE. See `Docs/MCU-Flashing-Guide.md` for import instructions.
2. Make your code changes.
3. Go to **Project → Build Configurations → Set Active → MCU**, then **Project → Build Project**.
4. The build output goes to `FW-NIH-MCU-H2/MCU/`. You need the **binary (`.bin`) file**, not the `.hex`. If STM32Cube IDE does not generate a `.bin` automatically, add a post-build step in the project properties:
   ```
   arm-none-eabi-objcopy -O binary "${BuildArtifactFileBaseName}.elf" "${BuildArtifactFileBaseName}.bin"
   ```
5. Note the **exact file size** of the `.bin` in bytes. You will need this number later.

> **Firmware version:** The firmware version string (`APP_FW_VER_STR` in `App/Config/app_config.h`) is automatically set to the build date in YYMMDD format. There is no manual version field to update.

---

## Step 2 — Sign the Firmware Image

Before the IPG will accept a firmware image, you must prove you hold the Admin private key by providing an ECDSA signature of the image's SHA-256 hash. This signature is checked *before* any image data is transferred.

In your update software, perform the following steps. The OpenNerve Windows App (`OpenNerve-Windows-App`) already implements this using .NET's `System.Security.Cryptography` library and is the recommended starting point.

**a. Hash the firmware binary**

Compute the SHA-256 hash of the entire `.bin` file:
```
HashMsg = SHA256(firmware.bin)   // 32 bytes
```

**b. Sign the hash with the Admin private key**

Using ECDSA P-256, sign `HashMsg` using the Admin private key. This produces two 32-byte components:
```
(RSign, SSign) = ECDSA_P256_Sign(HashMsg, admin_private_key)
```

In .NET:
```csharp
using var ecdsa = ECDsa.Create();
ecdsa.ImportECPrivateKey(adminPrivKeyBytes, out _);
byte[] der = ecdsa.SignHash(hashMsg, DSASignatureFormat.Rfc3279DerSequence);
// Parse DER to extract R and S (each 32 bytes)
```

In Python (using `cryptography` library):
```python
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes
import hashlib

hash_msg = hashlib.sha256(open("firmware.bin", "rb").read()).digest()
signature = private_key.sign(hash_msg, ec.ECDSA(hashes.SHA256()))
# decode_dss_signature() to get (r, s) integers, then format as 32-byte big-endian
```

You will now have three 32-byte values: `RSign`, `SSign`, and `HashMsg`. These are the payload for the AUTH command in Step 4.

---

## Step 3 — Connect to the IPG and Authenticate as Admin

The IPG must be in BLE Connected mode before firmware update commands are accepted. Connect using your software as normal and authenticate as Admin using opcode `0xF0` (`OP_AUTH`). Refer to `Docs/encryption-instructions.md` for details on the authentication handshake.

Once authenticated as Admin (`USER_CLASS_ADMIN = 0xFF`), the device is in state `STATE_ACT_MODE_BLE_CONN` and will accept firmware update commands.

---

## Step 4 — Authorize the Firmware Image (Enter OAD Mode)

Send opcode `0xF3` (`OP_AUTH_FW_IMAGE`) with the 96-byte payload assembled from Step 2:

| Bytes | Field | Description |
|---|---|---|
| 0–31 | `RSign` | ECDSA signature component r |
| 32–63 | `SSign` | ECDSA signature component s |
| 64–95 | `HashMsg` | SHA-256 hash of the firmware binary |

**Full command structure** (every OpenNerve command follows this format):
```
[Opcode: 1 byte] [PayloadLen: 1 byte] [Payload: N bytes] [CRC16: 2 bytes]
```

**Response structure:**
```
[Opcode: 1 byte] [Status: 1 byte] [PayloadLen: 1 byte] [Payload: N bytes] [CRC16: 2 bytes]
```

If the ECDSA signature is valid, the device responds with `STATUS_SUCCESS (0x00)` and transitions internally to OAD mode (`STATE_ACT_MODE_OAD`). The hash you provided is saved and will be used to verify the image after download.

If the response status is `STATUS_INVALID (0x01)`, the signature did not verify. Check that you are using the correct Admin private key and that the hash was computed over the correct binary file. **Do not proceed** to the next step — the device will reject all download packets.

If the response status is `STATUS_USER_CLASS_ERR (0xF3)`, you are not authenticated as Admin. Repeat Step 3.

---

## Step 5 — Download the Firmware Image

Send the firmware binary to the device in 132-byte packets using opcode `0xF4` (`OP_DOWNLOAD_FW_IMAGE`).

Each packet payload has this structure:

| Bytes | Field | Description |
|---|---|---|
| 0–3 | `ImageDataOffset` | Byte offset of this chunk within the firmware image (uint32, little-endian) |
| 4–131 | `ImageData` | 128 bytes of firmware data |

**Sending all packets:**

Iterate through the firmware binary in 128-byte chunks. For each chunk:
- Set `ImageDataOffset` to the byte position of the chunk (0, 128, 256, 384, …)
- Fill `ImageData` with the 128 bytes at that offset
- Send the packet and wait for `STATUS_SUCCESS` before sending the next

The last chunk may be shorter than 128 bytes. The firmware will reject any packet where `ImageDataOffset + 128 > 262144`, so do not send a packet that would write past the end of the FRAM buffer.

**Example (pseudocode):**
```
bin_data = read_file("firmware.bin")
image_size = len(bin_data)
offset = 0

while offset < image_size:
    chunk = bin_data[offset : offset + 128]
    chunk = chunk.ljust(128, b'\x00')  # pad last packet if needed
    packet = pack("<I", offset) + chunk  # 4-byte offset + 128 bytes data
    response = send_command(0xF4, packet)
    assert response.status == STATUS_SUCCESS
    offset += 128
```

**On retransmission:** Before writing each packet to FRAM, the firmware reads back the existing data at that offset and skips the write if it already matches. This means it is safe to retransmit any packet, and safe to restart the download from the beginning if interrupted — already-correct data won't be re-written. However, there is no "tell me what you're missing" command; if you need to restart, retransmit all packets.

**If BLE disconnects mid-download:** The OAD session is lost. Reconnect, re-authenticate (Steps 3 and 4), and retransmit all packets from the beginning. The FRAM data from the previous partial download may still be present, but the auth state is cleared on reconnect, so you must re-authorize before the verify step will succeed.

---

## Step 6 — Verify the Image

Once all packets have been sent, send opcode `0xF5` (`OP_VERIFY_FW_IMAGE`) with a 4-byte payload containing the firmware image size in bytes (uint32, little-endian):

```
payload = pack("<I", image_size)
response = send_command(0xF5, payload)
```

The device will:
1. Compute SHA-256 over the FRAM image (from offset 0 to `image_size`)
2. Compare it against the hash you provided in Step 4 (`HashMsg`)
3. If they match: write the image size and hash to a metadata region in FRAM, transition to BSL mode, and begin flashing

**If verify succeeds:** The response is `STATUS_SUCCESS` and the device immediately begins the BSL flash process (Step 7). The BLE connection will be terminated automatically when the device resets.

**If verify fails:** The response is `STATUS_INVALID` with a 1-byte payload containing the failure count. You can retry the verify up to **3 times**. A verify failure means the downloaded image is corrupted — some packets did not arrive correctly, or the wrong firmware binary was used. Go back to Step 5 and retransmit all packets. After 3 consecutive verify failures, the device returns to normal BLE Connected mode and you must restart from Step 4.

---

## Step 7 — BSL Flashes the New Firmware (Automatic)

You do not need to do anything for this step. After a successful verify, the device handles the rest autonomously:

1. Reads the image size and SHA-256 hash from FRAM metadata
2. Erases the **inactive** flash bank (the bank not currently running)
3. Copies the firmware image from FRAM into the inactive bank, 16 bytes at a time
4. Computes SHA-256 of the newly written flash and compares it to the stored hash
5. If it matches: swaps flash banks via STM32 option bytes and triggers a hardware reset
6. The device reboots into the new firmware

If the flash write verification fails (e.g. due to a flash hardware error), BSL retries up to 3 times. If all 3 attempts fail, the device returns to its original firmware and state — it is not bricked.

The reboot will terminate the BLE connection. After ~5 seconds, reconnect to the device and verify the update succeeded by checking the firmware version (build date) via your software.

---

## Troubleshooting

| Symptom | Likely cause | Action |
|---|---|---|
| `STATUS_INVALID` on `OP_AUTH_FW_IMAGE` | Wrong private key, or hash doesn't match binary | Verify key file path; recompute SHA-256; check you're using the `.bin` not the `.hex` |
| `STATUS_USER_CLASS_ERR` on any firmware update command | Not authenticated as Admin | Complete the normal Admin auth handshake (opcode `0xF0`) first |
| `STATUS_INVALID` on `OP_VERIFY_FW_IMAGE` | Image corrupted in transit | Retransmit all packets (up to 3 verify attempts); if still failing, start over from Step 4 |
| Device still running old firmware after BSL | BSL flash verification failed | Check FRAM integrity; try the full process again from Step 4 |
| BLE connection lost during download | RF interference or range | Reconnect, re-authenticate (Steps 3–4), retransmit all packets |
| Device unresponsive after reboot | New firmware has a critical bug | Not automatically recoverable via OTA; old bank is still in flash but there is no automatic rollback — physical access required |

---

## Key Limitations

- **MCU only.** The nRF52810 BLE firmware cannot be updated wirelessly. Any bug in BLE firmware requires physical J-Link access. See `Docs/BLE-Flashing-Guide.md`.
- **256 KB image size limit.** The FRAM staging buffer is fixed. If the firmware binary exceeds 262,144 bytes, the download will fail with `STATUS_INVALID` on the first oversized packet.
- **No rollback.** After a successful bank swap and reboot, there is no automatic mechanism to revert to the old firmware. The old bank remains in flash until the next OTA update, but recovering it requires physical reprogramming. Test your firmware thoroughly before deploying to implanted devices.
- **Admin key is permanent.** The Admin public key is baked into firmware at compile time and cannot be changed after a device is packaged. If the Admin private key is lost, no future OTA updates are possible on affected devices. Guard it carefully. See `Docs/encryption-instructions.md`.
- **No resume protocol.** If a download is interrupted, the entire image must be retransmitted from the beginning. There is no command to query which offsets are already correct.
- **Idle connection timeout.** The BLE connection has a configurable idle timeout (`HPID_BLE_IDLE_CONNECTION`). During a long download, ensure your software sends periodic commands or refreshes the idle timer, or configure a suitably long timeout before beginning the update.

---

## Summary of Opcodes Used

| Opcode | Name | Stage | Payload |
|---|---|---|---|
| `0xF0` | `OP_AUTH` | Pre-requisite | ECDSA auth for Admin session |
| `0xF3` | `OP_AUTH_FW_IMAGE` | Step 4 | 96 bytes: R(32) + S(32) + Hash(32) |
| `0xF4` | `OP_DOWNLOAD_FW_IMAGE` | Step 5 | 132 bytes: Offset(4) + Data(128) |
| `0xF5` | `OP_VERIFY_FW_IMAGE` | Step 6 | 4 bytes: image size (uint32 LE) |

All commands use the format: `[Opcode][PayloadLen][Payload...][CRC16]`
All responses use the format: `[Opcode][Status][PayloadLen][Payload...][CRC16]`

Status codes: `0x00` = success, `0x01` = invalid, `0xF0` = CRC error, `0xF1` = payload length error, `0xF2` = opcode error, `0xF3` = user class error.

---

## Relevant Source Files

For developers who want to understand or extend the OTA implementation:

| File | Description |
|---|---|
| `App/Src/app_mode_oad.c` | OAD mode handler; processes download and verify commands |
| `App/Src/app_mode_bsl.c` | BSL handler; erases flash, copies FRAM image, swaps banks |
| `App/Src/app_mode_ble_connection.c` | BLE connection mode; handles `OP_AUTH_FW_IMAGE` and triggers OAD |
| `App/Functions/Src/app_func_authentication.c` | ECDSA verification and SHA-256 hash comparison logic |
| `App/Functions/Inc/app_func_command.h` | All opcode definitions and command/response structs |
| `App/Functions/Inc/app_func_state_machine.h` | All state definitions (`STATE_ACT_MODE_OAD`, `STATE_ACT_MODE_BSL`, etc.) |
| `App/Config/ecc_publickey.h` | Admin public key stored in firmware |
| `Docs/encryption-instructions.md` | Key generation and public key update instructions |
