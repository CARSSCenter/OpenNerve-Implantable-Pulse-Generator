### Overview
This document describes how to flash the nRF52810 microcontroller (referred to as "BLE") on the OpenNerve Gen1 and Gen2 boards. Before beginning you must have the .hex file; the most recent valid firmware version is **FW-BLE-250409.hex** and can be found in the folder **FW-BLE**.

### Materials needed
* J-Link EDU Mini (https://shop-us.segger.com/product/j-link-edu-mini-8-08-91/)
* J-Link software (https://www.segger.com/downloads/jlink)
* nRF Connect for Desktop (https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop/Download#infotabs 
<img width="889" height="42" alt="image" src="https://github.com/user-attachments/assets/b9650d81-06b4-4473-852c-65cc292f9e5b" />


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

**Attach cables** Attach the J-Link EDU Mini ribbon cable to the BLE J-Tag headers. Attach a jumper between VBAT2R and the outside pin of VRECT. Apply power via USB-C.

<img width="418" height="508" alt="image" src="https://github.com/user-attachments/assets/0df41112-4e0d-49ee-9816-9853b3ca9e71" />


**Connect and Flash:** Open Programmer in nRF Connect. Select J-Link. Add the BLE hex file. Click Erase & Write.

<img width="612" height="290" alt="image" src="https://github.com/user-attachments/assets/7326a29b-080e-4f01-86cd-7b0feebe81c3" />


