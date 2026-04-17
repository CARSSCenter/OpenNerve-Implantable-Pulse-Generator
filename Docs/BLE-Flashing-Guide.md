### Overview
This document describes how to flash the nRF52810 microcontroller (referred to as "BLE") on the OpenNerve Gen1 and Gen2 boards. Before beginning you must have the .hex file; the most recent valid firmware version is **FW-BLE-250409.hex** and can be found in the folder **FW-BLE**.

### Materials needed
* J-Link EDU Mini (https://shop-us.segger.com/product/j-link-edu-mini-8-08-91/)
* J-Link software (https://www.segger.com/downloads/jlink)
* nRF Connect for Desktop (https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop/Download#infotabs)

### Important Note
Power to BLE is turned on by a GPIO pin from MCU (BLE_PWRn). During MCU's startup sequence in Production mode it activates power to BLE and then listens for an acknowlegement over the SPI lines; if it does not receive an ack before its watchdog timer triggers (2 seconds) it will cycle power to BLE and try again. The net effect is that if MCU is flashed but BLE has not yet been flashed, MCU will not provide power to BLE for long enough to flash it. There are several ways to work around this:
1. Flash MCU with the DVT version of firmware, then flash BLE, then re-flash MCU with Production firmware
2. Connect to MCU using an STLink and start it up in debug mode, adding a breakpoint after it sets BLE_PWRn high to maintain that state while flashing BLE
3. Physically connect the +VDD and +VDD_BLE nets, eliminating the need for MCU to power BLE. This can be easily done using a jumper wire with the female end attached to the DVDD pin J1300 and the male end inserted into the first port of the JLink's ribbon cable, or by soldering a wire to VDD_BLE at TP401.

<img width="2502" height="1012" alt="Jumper Wire DVDD BLE" src="https://github.com/user-attachments/assets/9abf876b-0c06-4ab7-abb5-c8c6dad68820" />

### Setup

**Step 1:** When installing the J-Link software, ensure that the box for "Install legacy USB driver" is checked

<img width="612" height="356" alt="image" src="https://github.com/user-attachments/assets/1083d21b-2700-46a8-b2d3-7045aa0ad688" />

When first connecting the J-Link EDU Mini to a computer, you can confirm that the driver is installed by checking Device Manager under USB Devices

<img width="424" height="216" alt="image" src="https://github.com/user-attachments/assets/2a5060f4-6ae3-4a2f-9e1d-a9c629fb5ff2" />
<img width="450" height="116" alt="image" src="https://github.com/user-attachments/assets/a986c58c-f026-4dfa-b318-88c5b03dace1" />

If the J-Link driver is not in USB devices, Start J-Link Configurator, right click on J-Link EDU Mini, Update FW -> Configure -> Select WinUSB driver -> OK

<img width="613" height="278" alt="image" src="https://github.com/user-attachments/assets/3f82c020-1735-45d0-93cd-a7421ff90ce0" />

**Step 2:** Download and run nRF Connect for Desktop, and install Programmer.

<img width="612" height="298" alt="image" src="https://github.com/user-attachments/assets/a61a6ef1-94a0-46d1-af3e-94a2f2afe724" />

### Flashing BLE Firmware

**Attach cables** Attach the J-Link EDU Mini ribbon cable to the BLE J-Tag headers. Attach a jumper between VUSB and VCHG_RAIL. Apply power via USB-C.

<img width="500" height="500" alt="OpenNerveBLE2" src="https://github.com/user-attachments/assets/af42a880-d4eb-456a-bbca-6e609ee21661" />

**Connect and Flash:** Open Programmer in nRF Connect. Select J-Link. Add the BLE hex file. Click Erase & Write.

<img width="612" height="290" alt="image" src="https://github.com/user-attachments/assets/7326a29b-080e-4f01-86cd-7b0feebe81c3" />

### Debugging

For debugging, load the file "UICR_APPROTECT_HwDisabled.hex" to disable AP Protection. The BLE firmware .hex file used for production does not include this hex file, so AP Protection remains enabled.

Please refer to the figure below for the SEGGER Embedded Studio configuration settings.

<img width="468" height="279" alt="image" src="https://github.com/user-attachments/assets/da9af7e2-eee9-4eb3-af91-2e6972a41305" />


