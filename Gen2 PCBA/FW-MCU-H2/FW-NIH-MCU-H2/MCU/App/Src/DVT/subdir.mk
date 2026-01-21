################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/DVT/app_mode_dvt.c \
../App/Src/DVT/app_mode_dvt_test.c 

OBJS += \
./App/Src/DVT/app_mode_dvt.o \
./App/Src/DVT/app_mode_dvt_test.o 

C_DEPS += \
./App/Src/DVT/app_mode_dvt.d \
./App/Src/DVT/app_mode_dvt_test.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/DVT/%.o App/Src/DVT/%.su App/Src/DVT/%.cyclo: ../App/Src/DVT/%.c App/Src/DVT/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src-2f-DVT

clean-App-2f-Src-2f-DVT:
	-$(RM) ./App/Src/DVT/app_mode_dvt.cyclo ./App/Src/DVT/app_mode_dvt.d ./App/Src/DVT/app_mode_dvt.o ./App/Src/DVT/app_mode_dvt.su ./App/Src/DVT/app_mode_dvt_test.cyclo ./App/Src/DVT/app_mode_dvt_test.d ./App/Src/DVT/app_mode_dvt_test.o ./App/Src/DVT/app_mode_dvt_test.su

.PHONY: clean-App-2f-Src-2f-DVT

