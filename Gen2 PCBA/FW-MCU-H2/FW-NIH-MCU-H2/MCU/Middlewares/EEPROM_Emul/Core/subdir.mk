################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve\ PCBA\ documentation/Final\ docs\ and\ FW\ from\ Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core/eeprom_emul.c 

OBJS += \
./Middlewares/EEPROM_Emul/Core/eeprom_emul.o 

C_DEPS += \
./Middlewares/EEPROM_Emul/Core/eeprom_emul.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/EEPROM_Emul/Core/eeprom_emul.o: /Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve\ PCBA\ documentation/Final\ docs\ and\ FW\ from\ Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core/eeprom_emul.c Middlewares/EEPROM_Emul/Core/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-EEPROM_Emul-2f-Core

clean-Middlewares-2f-EEPROM_Emul-2f-Core:
	-$(RM) ./Middlewares/EEPROM_Emul/Core/eeprom_emul.cyclo ./Middlewares/EEPROM_Emul/Core/eeprom_emul.d ./Middlewares/EEPROM_Emul/Core/eeprom_emul.o ./Middlewares/EEPROM_Emul/Core/eeprom_emul.su

.PHONY: clean-Middlewares-2f-EEPROM_Emul-2f-Core

