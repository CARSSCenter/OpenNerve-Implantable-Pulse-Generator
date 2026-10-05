# Building the BLE firmware (nRF52810)

This folder contains the firmware for the nRF52810 Bluetooth chip on the IPG board.
The nRF52810 handles BLE and talks to the main STM32 MCU over SPI/UART.

**Short version:** install SEGGER Embedded Studio and the nRF Command Line Tools, then run:

```bash
./build.sh            # produces FW-BLE-<YYMMDD>.hex, ready to flash
```

The rest of this document explains what that does and how to flash the result.

---

## 1. What goes into a flashable image

The chip runs two programs, which live in separate regions of its 192 KB of flash:

| Address range       | What                                    | Where it comes from                          |
|---------------------|-----------------------------------------|----------------------------------------------|
| `0x00000–0x00FFF`   | **MBR** (Master Boot Record)             | Part of the SoftDevice hex from Nordic        |
| `0x01000–0x18FFF`   | **S112 SoftDevice v7.2.0** (Nordic's BLE stack) | `nRF5_SDK_17.1.0/components/softdevice/s112/hex/` |
| `0x19000–0x2FFFF`   | **Application** (our code: `main.c`, `app/`) | Built by SEGGER Embedded Studio            |

RAM: the SoftDevice reserves `0x20000000–0x200022C7`. The application uses the rest, up to `0x20006000`.

A normal build produces **only the application** hex. To get a file you can flash onto a
blank chip, the application hex is **merged** with the SoftDevice hex using `mergehex`.
`build.sh` does both steps for you. The released image `FW-BLE-250409.hex` was made the same way.

## 2. Folder layout

```
FW-BLE/
  main.c, app/              Application source code (edit these)
  app/test/                 Unity unit tests (excluded from the build; kept for reference)
  carss_board/board/        custom_board.h – pin definitions for this board
  carss_board/s112/config/  sdk_config.h / app_config.h – SDK feature switches
  carss_board/s112/ses/     SEGGER Embedded Studio project (FW-NIH-BLE.emProject)
  nRF5_SDK_17.1.0/          Nordic nRF5 SDK 17.1.0, stripped of examples/docs, with MDK 8.43.0 (see §6)
  debugging/                UICR_APPROTECT_HwDisabled.hex – development only (see §5)
  FW-BLE-250409.hex         Released image from the original developer
  build.sh                  Build + merge script
```

The SDK is checked into the repo on purpose, so a fresh clone builds with no other downloads.
The project finds it through the `SDK_DIR` macro in `FW-NIH-BLE.emProject`
(`SDK_DIR=../../../nRF5_SDK_17.1.0`, relative to the `ses/` folder).

## 3. Tools to install

| Tool | Version | Used for | Get it from |
|------|---------|----------|-------------|
| SEGGER Embedded Studio (SES) for ARM | 8.26b tested (original developer used 5.42a) | Compiler/IDE. Includes its own ARM GCC and the `emBuild` command-line builder | segger.com → Embedded Studio. Free for use with Nordic chips |
| nRF Command Line Tools | 10.24 tested | `mergehex` (merge images), `nrfjprog` (flash) | nordicsemi.com → nRF Command Line Tools |
| SEGGER J-Link software | any recent | USB driver for the J-Link programmer | segger.com → J-Link Software |

You do **not** need a separate `arm-none-eabi-gcc`, `make`, the nRF Connect SDK/Zephyr, or Unity.

## 4. Building

### Option A – command line (recommended)

```bash
cd FW-BLE
./build.sh            # Debug configuration (what released images use)
./build.sh Release    # Release configuration
```

The output is `FW-BLE/FW-BLE-<YYMMDD>.hex`. `build.sh` uses the newest SES under
`/Applications/SEGGER/`. To pick a specific version, set
`SES_DIR="/Applications/SEGGER/SEGGER Embedded Studio 8.26b" ./build.sh`.

To run the two steps by hand:

```bash
"/Applications/SEGGER/SEGGER Embedded Studio 8.26b/bin/emBuild" -config Debug -rebuild \
    carss_board/s112/ses/FW-NIH-BLE.emProject

mergehex -m nRF5_SDK_17.1.0/components/softdevice/s112/hex/s112_nrf52_7.2.0_softdevice.hex \
            carss_board/s112/ses/Output/Debug/Exe/FW-NIH-BLE.hex \
         -o FW-BLE-<YYMMDD>.hex
```

### Option B – SES GUI (for editing and debugging)

1. Open `carss_board/s112/ses/FW-NIH-BLE.emProject` in SEGGER Embedded Studio.
2. Choose **Debug** from the configuration drop-down.
3. **Build → Build FW-NIH-BLE** (F7). The application hex is written to `carss_board/s112/ses/Output/Debug/Exe/FW-NIH-BLE.hex`.
4. Run the `mergehex` command above, or just run `./build.sh`, to get the flashable image.

**Debug → Go** (F5) with a J-Link attached downloads the SoftDevice, the application, and the
UICR unlock hex (see §5), then starts a debug session. Log output (`NRF_LOG_INFO`) goes to
SEGGER RTT. You can view it in the SES Debug Terminal or with `JLinkRTTViewer`.

Build products go into `carss_board/s112/ses/Output/`, which git ignores.

## 5. Flashing

Connect a J-Link to the nRF52810's SWD pins. Then:

```bash
nrfjprog -f NRF52 --recover                                   # unlock + full erase (see note)
nrfjprog -f NRF52 --program FW-BLE-<YYMMDD>.hex --chiperase --verify
nrfjprog -f NRF52 --reset
```

**APPROTECT (read-out protection):** production nRF52810 chips (revision 3, Errata 249) turn
debug-port protection **on** by default, so shipped boards are locked. `--recover` erases the
whole chip and unlocks it. That's expected and safe, because you flash the complete image straight afterwards.
After a reset, the newly flashed firmware locks the chip again. That is the production behaviour.

**For bench development only**, to keep the debug port open across resets, also program the UICR file:

```bash
nrfjprog -f NRF52 --program debugging/UICR_APPROTECT_HwDisabled.hex --verify
nrfjprog -f NRF52 --reset
```

That file writes `0x5A` ("HwDisabled") to `UICR.APPROTECT` (`0x10001208`). At startup, `main.c`
checks that value and then also unlocks the software side. **Never use it on production boards.**

## 6. Changes made to get the project building (Oct 2026)

The source package from the original developer only built on their own PC. These changes
let it build from this repo with current tools, and none of them affect the application logic:

1. **SDK vendored + MDK updated.** The nRF5 SDK 17.1.0 is in `nRF5_SDK_17.1.0/`. Its MDK
   (`modules/nrfx/mdk/`) was updated from 8.40.3 to **8.43.0**, which the original developer
   also used. `main.c` needs the nRF52810 APPROTECT definitions that were added in 8.43.0.
   Source: `NordicSemiconductor.nRF_DeviceFamilyPack.8.43.0.pack`. The SES startup files were left as shipped with the SDK.
2. **SDK paths.** The `../../../../../../` paths in `FW-NIH-BLE.emProject` were replaced with `$(SDK_DIR)/`.
3. **Unity tests excluded.** `app/test/` is excluded from the build, and `UNITY_INCLUDE_CONFIG_H`
   and the Unity include paths were removed, as the original developer recommended.
   `APP_UNITY_TEST_ENABLE` in `app_config.h` stays `false`.
4. **`retarget.c` excluded.** It redirects `printf` to the UART. The firmware never calls `printf`,
   and the file doesn't compile with SES ≥ 6 (`__printf_tag_ptr` was removed).
5. **`flash_placement.xml` updated for SES ≥ 6:**
   - `.tbss` now comes right after `.tdata_run`. Newer GNU ld requires the two thread-local sections to sit next to each other,
     otherwise the link fails with "TLS sections are not adjacent … map sections to segments failed".
   - Removed `size="0x4"` from `.text` and `.rodata`. SES 5 treated it as a minimum size;
     newer SES treats it as a maximum ("section is larger than specified size").

**Expect the output to differ slightly from `FW-BLE-250409.hex`.** SES 8 uses a newer compiler and
C library than SES 5.42a. As built with SES 8.26b on 2026-10-05, the merged image has a byte-identical
MBR and SoftDevice, the same vector table layout (stack top `0x20006000`, application at `0x19000`),
and a 80,347-byte application compared with 81,040 bytes in the release.
**Test any new build on hardware before releasing it** (see §8).

To reproduce the original developer's toolchain exactly, install SES **5.42a** from SEGGER's
older-versions download page and run `SES_DIR=".../SEGGER Embedded Studio for ARM 5.42a" ./build.sh`.

## 7. Making a release

The original developer's release practice, which is worth keeping:

1. Build the **Debug** configuration. `FW-BLE-250409.hex` was a Debug build.
2. Name the merged image `FW-BLE-<YYMMDD>.hex`.
3. Commit the hex **together with** the source it was built from, in the same commit, and tag it, e.g. `git tag fw-ble-261005`.

## 8. Checking a new build on hardware

1. Flash it (§5) and confirm the device advertises in nRF Connect (mobile or desktop).
2. Connect and check that the Nordic UART Service (NUS) is present and responds.
3. Check communication with the STM32 (SPI/UART) through the normal IPG commands.

## 9. Files you can ignore

- `carss_board/s112/ses/FW-NIH-BLE.mak` – Makefile auto-exported from the original developer's Windows SES install. Not used.
- `carss_board/s112/ses/genbdf.bat` – script for Parasoft C/C++test static analysis. Not used.
- `carss_board/s112/ses/Setup/`, `System/` – template files SES creates for new projects. Not referenced by this project.
- `*.jlink`, `*.emSession` – per-user SES/J-Link session state.
