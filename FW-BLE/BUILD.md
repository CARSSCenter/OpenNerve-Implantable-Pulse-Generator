# Building the BLE firmware (nRF52810)

This guide gives more detail in how to build firmware for the nRF52810 Bluetooth chip on the OpenNerve PCBA.
The nRF52810 handles BLE and talks to the main STM32 MCU over SPI, with two handshake lines
(RDY, REQ). Its UART is wired only to the test-fixture USB-serial chip, not to the STM32.

## Quick start guide

Install SEGGER Embedded Studio (8.26b was used for this build) and the nRF Command Line Tools.

On a Mac, run:
```bash
./build.sh            # produces FW-BLE-<YYMMDD>.hex, ready to flash
```

On Windows, run:

  ```bat
  cd FW-BLE
  "C:\Program Files\SEGGER\SEGGER Embedded Studio 8.26b\bin\emBuild.exe" -config Debug -rebuild carss_board\s112\ses\FW-NIH-BLE.emProject
  mergehex -m nRF5_SDK_17.1.0\components\softdevice\s112\hex\s112_nrf52_7.2.0_softdevice.hex carss_board\s112\ses\Output\Debug\Exe\FW-NIH-BLE.hex -o FW-BLE-YYMMDD.hex
  ```

Details below.
---

## 1. What goes into a flashable image

The chip runs two programs, which live in separate regions of its 192 KB of flash:

| Address range       | What                                    | Where it comes from                          |
|---------------------|-----------------------------------------|----------------------------------------------|
| `0x00000–0x00FFF`   | **MBR** (Master Boot Record)             | Part of the SoftDevice hex from Nordic        |
| `0x01000–0x18FFF`   | **S112 SoftDevice v7.2.0** (Nordic's BLE stack) | `nRF5_SDK_17.1.0/components/softdevice/s112/hex/` |
| `0x19000–0x2FFFF`   | **Application** (OpenNerve code: `main.c`, `app/`) | Built by SEGGER Embedded Studio            |

RAM: the SoftDevice reserves `0x20000000–0x200022C7`. The application uses the rest, up to `0x20006000`.

A normal build produces **only the application** hex. To get a file you can flash onto a
blank chip, the application hex is **merged** with the SoftDevice hex using `mergehex`.
`build.sh` does both steps for you. The original released image `FW-BLE-250409.hex` was made in this way.

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
  build.sh                  Build + merge script (macOS / Linux / Git Bash; see §9 for Windows)
  TEST-PLAN.md              Bench checklist for qualifying a new image (see §8)
```

The SDK is checked into the repo on purpose, so a fresh clone builds with no other downloads.
The project finds it through the `SDK_DIR` macro in `FW-NIH-BLE.emProject`
(`SDK_DIR=../../../nRF5_SDK_17.1.0`, relative to the `ses/` folder).

## 3. Tools to install

| Tool | Version | Used for | Get it from |
|------|---------|----------|-------------|
| SEGGER Embedded Studio (SES) for ARM | 8.26b tested on macOS (original developer used 5.42a on Windows). Other versions: see §9 | Compiler/IDE. Includes its own ARM GCC and the `emBuild` command-line builder | segger.com → Embedded Studio. Free for use with Nordic chips |
| nRF Command Line Tools | 10.24 tested | `mergehex` (merge images), `nrfjprog` (flash) | nordicsemi.com → nRF Command Line Tools |
| SEGGER J-Link software | any recent | USB driver for the J-Link programmer | segger.com → J-Link Software |
| nRF Connect for Desktop + Programmer app | any recent | Optional graphical alternative to `nrfjprog` for flashing (§5) | nordicsemi.com → nRF Connect for Desktop |

You do **not** need a separate `arm-none-eabi-gcc`, `make`, the nRF Connect SDK/Zephyr, or Unity.

## 4. Building

### Option A – command line

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
4. Run the `mergehex` command above.

**Debug → Go** (F5) with a J-Link attached downloads the SoftDevice, the application, and the
UICR unlock hex (see §5), then starts a debug session. Log output (`NRF_LOG_INFO`) goes to
SEGGER RTT. You can view it in the SES Debug Terminal or with `JLinkRTTViewer`.

Build products go into `carss_board/s112/ses/Output/`, which git ignores.

### Building on Windows

This guide was developed and firmware was originally compiled on MacOS. The project file uses relative, forward-slash paths and the bundled compiler, so it builds on
Windows mostly the same way. Some differences:

- **`build.sh` is a bash script**, and it looks for SES only under `/Applications/SEGGER/`.
  On Windows, you must run the two steps by hand in Command Prompt or PowerShell (adjust the folder name to your SES version):

  ```bat
  cd FW-BLE
  "C:\Program Files\SEGGER\SEGGER Embedded Studio 8.26b\bin\emBuild.exe" -config Debug -rebuild carss_board\s112\ses\FW-NIH-BLE.emProject
  mergehex -m nRF5_SDK_17.1.0\components\softdevice\s112\hex\s112_nrf52_7.2.0_softdevice.hex carss_board\s112\ses\Output\Debug\Exe\FW-NIH-BLE.hex -o FW-BLE-YYMMDD.hex
  ```

  Or run `build.sh` from **Git Bash** with
  `SES_DIR="/c/Program Files/SEGGER/SEGGER Embedded Studio 8.26b" ./build.sh`. This is untested;
  if the script says `emBuild` is not found, use the manual commands above.
- **Install folder names:** SES 5.x installs as `SEGGER Embedded Studio for ARM 5.xx`, and newer
  versions as `SEGGER Embedded Studio 8.xx`.
- **Long paths:** the SDK has deep folders, and object files add more depth under `Output/`. If the
  build fails with "file not found" for a file that exists, clone the repo to a short path
  (e.g. `C:\src\OpenNerve`) or enable Windows long-path support.
- **J-Link driver:** install the J-Link software with the "legacy USB driver" option, as described
  in `docs/BLE-Flashing-Guide.md`. `nrfjprog`, `mergehex` and nRF Connect Programmer work the
  same on Windows.
- **Line endings:** Git may convert line endings on checkout. SES doesn't care, and neither do the
  `.hex` files.

## 5. Flashing

Use the **merged** image (`FW-BLE-<YYMMDD>.hex` from `build.sh`), never the application-only
`Output/.../FW-NIH-BLE.hex`. Connect a J-Link to the nRF's 10-pin debug header J400.

**The nRF must be powered while you flash it.** The STM32 switches the nRF's power on and off
(BLE_PWRn) and controls its reset line, so with production STM32 firmware the nRF may be off or
being reset. `docs/BLE-Flashing-Guide.md` (repo root) gives the cable setup and three workarounds,
e.g. running the DVT STM32 firmware while you flash BLE.

There are two ways to flash. Both erase the whole chip first.

### Option A – nRF Connect Programmer

Described in detail with screenshots in `docs/BLE-Flashing-Guide.md`. Summary:

1. Install **nRF Connect for Desktop**, then install the **Programmer** app from inside it.
2. Open Programmer and select the J-Link in the device list (top left).
3. **Add file** → choose `FW-BLE-<YYMMDD>.hex`. The memory map should show the MBR and SoftDevice
   from `0x0` and the application from `0x19000`.
4. Click **Erase & write**. Production boards are read-protected (see APPROTECT below). If
   Programmer says the device is protected, accept the full erase it offers.
5. Disconnect or reset the board. For bench debugging only, also add
   `debugging/UICR_APPROTECT_HwDisabled.hex` in step 3. Programmer writes both files together.

### Option B – command line

```bash
nrfjprog -f NRF52 --recover                                   # unlock + full erase (see note)
nrfjprog -f NRF52 --program FW-BLE-<YYMMDD>.hex --chiperase --verify
nrfjprog -f NRF52 --reset
```

### Removing APPROTECT for debugging 

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

To remove APPROTECT when flashing using the nRF Connect Programmer, first select "Clear Files" to reset, then "Add File" with FW-BLE-<TTMMDD>.hex, then select "Add File" again and add UICR_APPROTECT_HwDisabled.hex on top of the previous hex file.

## 6. Changes made to get the project building (Oct 2026)

The source package from the original developer only built on their own PC. The following changes were made to 
let the firmware build in this repo with current tools:

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
   - `.tbss` now comes **immediately before** `.tdata_run`, the same order as SEGGER's own templates
     (`targets/*_placement.xml`). SES 8's C library uses thread-local storage (TLS); `isprint()`,
     for example, reads the locale from it. SES 8's startup code fixes the thread pointer at
     `__tbss_start__ - 8`, so `.tbss` must be the first TLS section.
     - The original order (`.tdata_run`, `.bss`, `.tbss`) fails to link: "TLS sections are not adjacent".
     - `.tbss` placed *after* `.tdata_run` links, but every TLS access is off by `sizeof(.tdata)`.
       The first `NRF_LOG_HEXDUMP` then crashes the chip. That is how FW-BLE-261006 failed: RDY and
       BLE_RSTn pulsed every ~500 ms. Fixed in FW-BLE-261007.
     - Check after any placement change: the `__aeabi_read_tp` literal must equal `__tbss_start__ - 8`
       (`arm-none-eabi-nm` / `objdump` on the ELF).
   - Removed `size="0x4"` from `.text` and `.rodata`. SES 5 treated it as a minimum size;
     newer SES treats it as a maximum ("section is larger than specified size").

**Expect the output to differ slightly from `FW-BLE-250409.hex`.** SES 8 uses a newer compiler and
C library than SES 5.42a. As built with SES 8.26b, FW-BLE-261007 has:
- a byte-identical MBR and SoftDevice;
- the same vector table layout (stack top `0x20006000`, application at `0x19000`);
- an 80,347-byte application, compared with 81,040 bytes in the release.

On 2026-10-07 it was confirmed on hardware to boot and complete the STM32 handshake.
**Test any new build on hardware before releasing it** (see §8).


## 7. Checking a new build on hardware

Quick test:
1. Flash it and confirm the device advertises in nRF Connect (mobile or desktop).
2. In the Windows App, check that all values in the advertisement (battery voltages, temperature, etc) are received
3. Connect to the device and confirm that the authentication handshake finishes correctly.
4. Stream data and confirm that the data arrives
5. Start and stop stimulation and confirm that commands are successfully sent.

## 8. Different SES versions

Only **SES 8.26b on macOS** has been tested with this repo. Other setups should work, but expect
the differences below. Whatever you use, run the checks in §10 ("Checks after changing toolchain")
and test on hardware (§8) before releasing.

Each SES version bundles its own GCC and SEGGER C library, so **each version produces a slightly
different image**. Compiling cleanly does not prove the image works: FW-BLE-261006 compiled
without warnings and still crashed on the board (§6, item 5).

| SES version | What to expect |
|---|---|
| **8.x** (8.26b tested) | Should build as-is. |
| **6.x / 7.x** | These versions also use the newer SEGGER C library, so the §6 changes apply and the project should build. Untested. Check the thread-pointer layout (§10). |
| **5.x** (5.42a = original developer's version) | The old C library has no thread-local storage, and `retarget.c` compiles again. The §6 placement changes should be harmless, but this combination is untested. SES 5.42a matches the shipped image most closely. It's on SEGGER's older-versions download page. On Apple-silicon Macs it may need Rosetta. Select it with `SES_DIR=".../SEGGER Embedded Studio for ARM 5.42a" ./build.sh`. |
| **Future versions** | SEGGER may change its startup code (`source/thumb_crt0.s`) or C library again. If the link fails or the board misbehaves, compare `flash_placement.xml` with the new version's `targets/*_placement.xml`. |

Opening the project in a newer SES GUI can silently rewrite `FW-NIH-BLE.emProject` or add files
(that's where `Setup/` and `System/` came from). Check `git diff` after opening the project in a
new version, and only commit project-file changes you meant to make.

## 9. Troubleshooting

### Build errors

| Symptom | Cause | Fix |
|---|---|---|
| `retarget.c: error: unknown type name '__printf_tag_ptr'` | `retarget.c` was put back in the build. It doesn't compile with SES ≥ 6. | Keep it excluded (§6, item 4). The firmware doesn't use `printf`. |
| `'UICR_APPROTECT_PALL_HwDisabled' undeclared` (in `main.c`) | Building against an SDK with MDK older than 8.43.0, e.g. a fresh download of SDK 17.1.0. | Use the vendored `nRF5_SDK_17.1.0/` (`SDK_DIR` in the project file). |
| `unity.h: No such file or directory` | The `Unity Test` folder was re-enabled. | Exclude `app/test/` again; Unity isn't needed for the firmware. |
| A file in `components/…` or `modules/…` not found | `SDK_DIR` points to the wrong place, or the project was moved. | `SDK_DIR` is relative to `carss_board/s112/ses/`. Keep the folder layout in §2. |
| Linker: `TLS sections are not adjacent` … `map sections to segments failed: bad value` | Thread-local sections in the wrong order in `flash_placement.xml`. | Order must be `.data_run`, `.tbss`, `.tdata_run`, `.bss` (§6, item 5). |
| Linker: `.text section is larger than specified size` (or `.rodata`) | `size="0x4"` is back on those sections. | Remove it (§6, item 5). |
| `emBuild not found` from `build.sh` | SES isn't under `/Applications/SEGGER/`, or you're on Windows. | Set `SES_DIR`, or run the commands by hand (§4, §9). |

### Checks after changing toolchain or `flash_placement.xml`

Run these on `carss_board/s112/ses/Output/Debug/Exe/FW-NIH-BLE.elf`, using SES's own tools in
`<SES>/gcc/arm-none-eabi/bin/`:

1. **Thread pointer.** `objdump -d` the `__aeabi_read_tp` function. Its literal value must equal
   `__tbss_start__ - 8` (see `nm -n`). If it doesn't, the image will crash on the board.
2. **Application end.** The highest application address (end of `.tdata` load image, or see the
   `.map`) must be below **0x2D000**. Pairing/bond storage lives at 0x2D000–0x2FFFF, and the
   linker doesn't know about it.
3. **Hex comparison** with `FW-BLE-250409.hex`: MBR and SoftDevice identical, and stack top
   `0x20006000` at address `0x19000`.

### On the board

| Symptom | Likely cause | What to do |
|---|---|---|
| **RDY (TP404) and BLE_RSTn (TP403) pulse low every ~250–500 ms**, while VDD_BLE and BLE_PWRn are steady | The nRF crashes or hangs after it starts its SPI interface. The STM32 sees RDY low and keeps resetting it. In FW-BLE-261006 this was the thread-local-storage bug (§6, item 5). | Flash with the UICR unlock hex and watch RTT. The last line before the next `BLE Initialize Start` shows where it stops. Debug builds stop dead on any error instead of resetting. |
| RTT shows `BLE Initialize Start` but never `BLE Initialize Finish` | The BLE stack failed to start. The firmware requires the external 32.768 kHz crystal (Y401). | Check the crystal and its load capacitors. Don't change the clock source in `sdk_config.h` unless the hardware changed. |
| No RTT output, or the J-Link can't connect after a reset | APPROTECT locked the chip again. That's normal for production images. | Program `debugging/UICR_APPROTECT_HwDisabled.hex` together with the image (§5), development only. |
| Flashing fails partway, or the chip isn't found | The STM32 has the nRF powered off, or keeps resetting it. **BLE_PWRn low = nRF powered.** Note: `docs/BLE-Flashing-Guide.md` says "high" — that's wrong. | Use a workaround from `docs/BLE-Flashing-Guide.md` (e.g. DVT STM32 firmware). Flash while the STM32 is awake; it holds BLE_RSTn low while asleep. |
| Bonded phones lost or pairing data corrupted after an update | Application grew past 0x2D000 into bond storage, or a full chip erase (expected: `--recover` / Erase & write clears bonds). | Check the application end address (above). Re-pair after any full erase. |
| Device works with `FW-BLE-250409.hex` but not with your build | A toolchain difference. | Do the checks above and the A/B comparison in TEST-PLAN.md. Try SES 5.42a to rule out the toolchain. |

Known firmware issues that affect **all** images (old and new) are listed in TEST-PLAN.md §7.

## 10. Files you can ignore

- `carss_board/s112/ses/FW-NIH-BLE.mak` – Makefile auto-exported from the original developer's Windows SES install. Not used.
- `carss_board/s112/ses/genbdf.bat` – script for Parasoft C/C++test static analysis. Not used.
- `carss_board/s112/ses/Setup/`, `System/` – template files SES creates for new projects. Not referenced by this project.
- `*.jlink`, `*.emSession` – per-user SES/J-Link session state.
