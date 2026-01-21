################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Bsp/Src/bsp_adc.c \
../App/Bsp/Src/bsp_fram.c \
../App/Bsp/Src/bsp_magnet.c \
../App/Bsp/Src/bsp_serialport.c \
../App/Bsp/Src/bsp_watchdog.c 

OBJS += \
./App/Bsp/Src/bsp_adc.o \
./App/Bsp/Src/bsp_fram.o \
./App/Bsp/Src/bsp_magnet.o \
./App/Bsp/Src/bsp_serialport.o \
./App/Bsp/Src/bsp_watchdog.o 

C_DEPS += \
./App/Bsp/Src/bsp_adc.d \
./App/Bsp/Src/bsp_fram.d \
./App/Bsp/Src/bsp_magnet.d \
./App/Bsp/Src/bsp_serialport.d \
./App/Bsp/Src/bsp_watchdog.d 


# Each subdirectory must supply rules for building sources it contributes
App/Bsp/Src/%.o App/Bsp/Src/%.su App/Bsp/Src/%.cyclo: ../App/Bsp/Src/%.c App/Bsp/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Bsp-2f-Src

clean-App-2f-Bsp-2f-Src:
	-$(RM) ./App/Bsp/Src/bsp_adc.cyclo ./App/Bsp/Src/bsp_adc.d ./App/Bsp/Src/bsp_adc.o ./App/Bsp/Src/bsp_adc.su ./App/Bsp/Src/bsp_fram.cyclo ./App/Bsp/Src/bsp_fram.d ./App/Bsp/Src/bsp_fram.o ./App/Bsp/Src/bsp_fram.su ./App/Bsp/Src/bsp_magnet.cyclo ./App/Bsp/Src/bsp_magnet.d ./App/Bsp/Src/bsp_magnet.o ./App/Bsp/Src/bsp_magnet.su ./App/Bsp/Src/bsp_serialport.cyclo ./App/Bsp/Src/bsp_serialport.d ./App/Bsp/Src/bsp_serialport.o ./App/Bsp/Src/bsp_serialport.su ./App/Bsp/Src/bsp_watchdog.cyclo ./App/Bsp/Src/bsp_watchdog.d ./App/Bsp/Src/bsp_watchdog.o ./App/Bsp/Src/bsp_watchdog.su

.PHONY: clean-App-2f-Bsp-2f-Src

