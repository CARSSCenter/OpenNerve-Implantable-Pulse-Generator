# OpenNerve IPG OTA Firmware Update Tool — User Guide

## Overview

This tool updates the firmware on an OpenNerve Gen2 IPG wirelessly over Bluetooth (BLE). It automates the full update process: signing the firmware image, connecting to the device, transferring the image, and triggering the device to flash and reboot.

**What gets updated:** The STM32U585 main MCU firmware only. The nRF52810 BLE chip firmware cannot be updated wirelessly and requires a hardware programmer.

**Safety:** The device stores the new firmware in external FRAM before writing to flash. The running firmware is never touched during the transfer. If anything goes wrong — interrupted connection, corrupted data, wrong key — the device continues running its original firmware. It cannot be bricked through this process.

---

## Platform Support

| Platform | Status | Notes |
|---|---|---|
| **Windows 10/11** | Fully supported | Passkey injected automatically — no dialog |
| **macOS** | Supported | Requires a one-time pre-pairing step — see [macOS Pairing](#macos-pairing) below |

---

## Requirements

- **Python 3.9 or later**
- **Admin private key file** (`admin_priv_d_hex.txt`) — the ECDSA P-256 private key whose matching public key is baked into the IPG firmware. See [Getting the Private Key](#getting-the-private-key) below.
- **A compiled firmware binary** (`.bin` file, not `.hex` or `.elf`)
- **Bluetooth** enabled on your computer
- **The IPG powered on and in BLE advertising mode** (not asleep, not already connected to another device)

### Install dependencies

```bash
pip install -r requirements.txt
```

---

## Basic Usage

```bash
python3 ota_update.py FIRMWARE.BIN PRIVATE_KEY [--device-name NAME]
```

| Argument | Description |
|---|---|
| `FIRMWARE.BIN` | Path to the compiled firmware binary |
| `PRIVATE_KEY` | Path to the admin private key file |
| `--device-name` | BLE name to scan for (default: `CARSS`) |

### Examples

```bash
# Standard update with default device name
python3 ota_update.py firmware.bin secrets/admin_priv_d_hex.txt

# If your device advertises under a different name
python3 ota_update.py firmware.bin secrets/admin_priv_d_hex.txt --device-name "MyOpenNerve"

# Using absolute paths
python3 ota_update.py /path/to/build/FW-NIH-MCU-H2.bin ~/keys/admin_priv_d_hex.txt
```

---

## Step-by-Step Walkthrough

### 1. Prepare the firmware binary

Build the firmware in STM32Cube IDE (**Project → Build Project**). The output goes to `Gen2 PCBA/FW-MCU-H2/FW-NIH-MCU-H2/MCU/`. You need the `.bin` file — not the `.hex` or `.elf`.

If STM32Cube IDE doesn't generate a `.bin` automatically, add a post-build step in project properties:
```
arm-none-eabi-objcopy -O binary "${BuildArtifactFileBaseName}.elf" "${BuildArtifactFileBaseName}.bin"
```

The binary must be **under 256 KB (262,144 bytes)**. The tool will tell you if it's too large before connecting.

### 2. Put the IPG in BLE advertising mode

The device must be actively advertising for the tool to find it. On a freshly powered-on device this happens automatically. If the device went to sleep, press the magnet switch or power-cycle to wake it.

### 3. Pair the IPG to your computer

The IPG must be bonded to your computer before running the script. How to do this depends on your platform:

**Windows:** Open the OpenNerve Windows app, connect to the IPG, then press "Quit" to disconnect. The passkey (`000000`) is handled automatically — no system dialog should appear. Ensure that after quitting the IPG goes back to BLE advertising mode, and is not in sleep mode.

**macOS:** Open a BLE scanner such as [LightBlue](https://punchthrough.com/lightblue/), connect to the IPG (enter passkey `000000` if prompted). This bonds the device to macOS so the script can connect without re-pairing. You can run the OTA tool while the IPG is connected to the mac; the tool will identify the already-connected device and go straight to authentication.

### 4. Run the tool

```bash
python3 ota_update.py firmware.bin secrets/admin_priv_d_hex.txt
```

The tool will:
1. Print the firmware size and SHA-256 hash
2. Scan for nearby BLE devices and connect (if multiple IPGs are found, you'll be prompted to pick one)
3. Authenticate as Admin over BLE
4. Transfer the firmware in 128-byte packets, showing a progress bar
5. Verify the transferred image matches the original file
6. Trigger the device to flash the new firmware and reboot

A successful run looks like:
```
=== OpenNerve IPG OTA Firmware Update ===

Firmware:  /path/to/firmware.bin
Size:      98,304 bytes  (96.0 KB)
SHA-256:   a3f2c1...
Packets:   768  (128 bytes each)

Key file:  /path/to/admin_priv_d_hex.txt  [OK]

Checking for already-connected or previously-bonded 'CARSS'...
  Found via CoreBluetooth: XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
Connecting to CARSS (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX)...
  (device found via bonded/connected path — skipping pairing)
Connected to CARSS (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX)

[1/4] Authenticating as Admin...
      Admin authentication OK.

[2/4] Authorizing firmware image...
      Firmware image authorized. Device entered OAD mode.

[3/4] Downloading firmware (98,304 bytes, 768 packets)...
  [████████████████████████████████████████] 100.0%  768/768 packets
      Download complete.

[4/4] Verifying firmware image (device computing SHA-256)...
      Verification passed!

  The device is now flashing the new firmware and will reboot.
  The BLE connection will drop momentarily — this is expected.
  Wait ~5 seconds, then reconnect to confirm the firmware version.

OTA update complete.
```

### 5. Confirm the update

After the BLE connection drops (~5 seconds), reconnect with the Windows App or any other BLE tool and check the firmware version. The version string (`APP_FW_VER_STR`) is set to the build date in YYMMDD format.

**Note:** after restarting the IPG will default into Sleep mode. You can wake it up or power cycle using the magentic switch. If powering a development board via USB, the magnetic switch is disabled, so you will need to press the reset button or power cycle the board manually.

---

## macOS Pairing

macOS CoreBluetooth requires a device to be bonded before a third-party app can communicate with it over an encrypted link. The one-time pre-pairing step (Step 2 above) satisfies this requirement.

Once bonded, the script finds the device via CoreBluetooth's `retrieveConnectedPeripheralsWithServices` API (if currently connected to the OS) or from a UUID cache stored at `~/.config/carss/ble_uuid_cache.json` (written on first successful connection). This means the device does not need to be actively advertising for the script to find it on subsequent runs, as long as it has been seen before.

If the cache is stale or the device has been re-paired, run the pre-pairing step again with LightBlue and the script will repopulate the cache on the next successful connection.

---

## Getting the Private Key

The admin private key is a 32-byte ECDSA P-256 secret stored as 64 hexadecimal characters in a plain text file, one line, no spaces or colons:

```
a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2
```

The **OpenNerve development key** is available upon request from the CARSS team. If you are deploying OpenNerve for a new application, generate your own key pair — see `Docs/encryption-instructions.md` for instructions.

> **Keep the private key secure.** Anyone who holds it can update firmware on your device. Never commit it to git or share it over unencrypted channels. Store it outside the repository (the `secrets/` folder is excluded from git via `.gitignore`).

---

## Troubleshooting

### Device not found during scan

- Ensure the IPG is powered on and within BLE range (~10 m line-of-sight).
- Confirm the device is in BLE advertising mode. If it went to sleep, wake it first.
- If the device is already connected to another app, disconnect that app first — the IPG only supports one BLE connection at a time.
- Try specifying the exact device name: `--device-name "OpenNerve-123"` (check the name in a BLE scanner app if unsure).

### Pairing dialog appears on Windows

This should not happen — the tool injects the passkey automatically on Windows. If it does appear, enter `000000`. The connection should succeed after pairing.

### Device pairs, but OSError occurs

This can happen if there are issues pairing the IPG with the computer. If you see this, try connecting with the Windows app and then disconnecting. Then power cycle the board by holding the magnet for 15 seconds to power down, wait 5 minutes, then hold the magnet for 5 seconds to power back up.

### "ECDSA signature rejected" on Admin authentication

- You're using the wrong private key. Verify the key file corresponds to the public key baked into the running firmware (`ecc_publickey.h`).
- Check the key file has exactly 64 hex characters with no extra whitespace, comments, text, or newlines.

### "Firmware image authorization failed"

- Double-check that you're using the `.bin` file, not a `.hex` or `.elf`.
- Ensure the private key is correct (same key issue as above).

### Verify fails after download

The tool automatically retransmits all packets and retries verification up to 3 times. If all 3 fail, it usually means:
- RF interference caused persistent packet corruption — move closer to the device and retry.
- The firmware binary is corrupted on disk — re-build and retry.

### BLE disconnects mid-download

The tool is designed to handle this automatically. On disconnect it saves the last successfully acknowledged byte offset, waits 10 seconds for the device to re-advertise, reconnects, re-authenticates, and resumes the download from where it left off (up to 5 reconnect attempts). You should see a message like:

```
BLE disconnected at 45,056 B (45%). Waiting 10s for device to re-advertise...
```

If the tool exhausts all reconnect attempts, re-run it from the beginning. Any partially transferred data in FRAM is harmless.

### Device unresponsive after reboot

If the new firmware has a critical bug preventing boot, the device cannot be recovered via OTA. The old firmware bank remains in flash but there is no automatic rollback. Physical J-Link access is required to re-flash. **Test firmware thoroughly before deploying to implanted devices.**

Always try power cycling before concluding that the firmware is incorrect.

### General Advice

A combination of connecting / disconnecting to the Windows app and power cycling the IPG using the magnetic switch usually solves the problem.

Unconfirmed / probably superstition, but try clearing logs off of FRAM prior to OTA if there are issues.

---

## Limitations

- **MCU only.** The nRF52810 BLE chip firmware cannot be updated wirelessly.
- **256 KB image size limit.** The FRAM staging buffer is fixed at 262,144 bytes. The tool checks this before connecting.
- **No rollback.** After a successful bank swap and reboot, the old firmware is not automatically accessible. Guard the admin private key carefully — if it is lost, OTA updates on affected devices are no longer possible.
- **Resume on BLE disconnect.** If the BLE link drops mid-download, the tool automatically reconnects and resumes from the last acknowledged offset. If the process is manually interrupted (Ctrl+C) or the tool exits abnormally, the next run will retransmit the full image from the beginning.

---

## Related Documentation

| Document | Contents |
|---|---|
| `Docs/OTA-Firmware-Update-Guide.md` | Full technical protocol specification |
| `Docs/encryption-instructions.md` | Key generation, public key format, updating firmware keys |
| `Docs/MCU-Flashing-Guide.md` | Physical J-Link flashing (for recovery or first-time programming) |
| `Docs/BLE-Flashing-Guide.md` | nRF52810 BLE chip firmware update (requires hardware programmer) |
