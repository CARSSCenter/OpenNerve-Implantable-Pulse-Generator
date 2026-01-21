## MCU Firmware compiling and flashing guide
Alex Baldwin
1/14/2026

### Overview
The OpenNerve PCB has two microcontrollers: an STM32U5 MCU (referred to as “MCU” throughout documentation) which handles the majority of functions, and an nRF52810 (referred to as “BLE”) which handles Bluetooth Low Energy communication. This guide will describe how to compile and program (“flash”) the firmware for MCU.
To open and edit the firmware source code for MCU you will need to download and install the free STM32Cube IDE (https://www.st.com/en/development-tools/stm32cubeide.html). Once you have done that follow these steps to open the project in the IDE:

1.	File -> import
2.	General -> Existing Projects into Workspace
3.	“Select root directory” -> point at the folder that contains FW-NIH-MCU-H2
4.	Make sure the project FW-NIH-MCU-H2 is checked
5.	Hit finish	

To compile the firmware, go to Project -> Build Project or use the hammer icon at the top of the IDE. Choose “MCU” under Build Configurations, unless you are performing DVT tests or Unit Tests. A folder called “MCU” will be generated, unless it already exists. Inside that folder you will find FW-NIH-MCU-H2.hex.

### MCU Flashing
To flash firmware to MCU, you must first download the free STM32Cube Programmer (https://www.st.com/en/development-tools/stm32cubeprog.html). You must also purchase an STLink-V3Minnie (https://www.mouser.com/ProductDetail/STMicroelectronics/STLINK-V3MINIE?qs=MyNHzdoqoQKcLQe5Jawcgw%3D%3D). 

Step 1. Attach the STLink-V3 by USB  to your computer and confirm that the COM port and ST-Link Debug entries are correct in the Device Manager. 

 <img width="468" height="157" alt="image" src="https://github.com/user-attachments/assets/80a59c6b-e691-47aa-9292-a92d643fc93c" />

<img width="331" height="105" alt="image" src="https://github.com/user-attachments/assets/1a6efead-d04d-4436-9f7c-e34b8b51a19d" />

Step 2. Connect the JLink’s cable to the “MCU” connector on OpenNerve. The cable will need to be correctly centered in its connector, and will be wider than the connector.

<img width="468" height="356" alt="image" src="https://github.com/user-attachments/assets/0032528f-be8d-4684-954c-0a4a5d6315a4" />

<img width="321" height="274" alt="image" src="https://github.com/user-attachments/assets/acfd4e47-c6f5-4396-9b1e-68676226b0cd" />

Step 3. Apply USB power to OpenNerve board, ensuring that a jumper is placed between VRECT and the outside pin of VBAT2R to bypass the magnetic switch.

 <img width="219" height="251" alt="image" src="https://github.com/user-attachments/assets/39f583eb-db94-4ab4-8bd6-420be3df3562" />

Step 4. Open STM32Cube Programmer. First, connect to the board using the JLink. Second, Erase the existing firmware. Third, Open the .hex file. Fourth, Download the hex file’s code to the MCU.

 <img width="468" height="215" alt="image" src="https://github.com/user-attachments/assets/c3e89598-b02f-4db7-b796-d511e08308c6" />

Step 5. Press the Reset button on OpenNerve to reboot the board and start using your code.
