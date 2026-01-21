################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Functions/Src/app_func_authentication.c \
../App/Functions/Src/app_func_ble.c \
../App/Functions/Src/app_func_command.c \
../App/Functions/Src/app_func_logs.c \
../App/Functions/Src/app_func_measurement.c \
../App/Functions/Src/app_func_parameter.c \
../App/Functions/Src/app_func_state_machine.c \
../App/Functions/Src/app_func_stimulation.c 

OBJS += \
./App/Functions/Src/app_func_authentication.o \
./App/Functions/Src/app_func_ble.o \
./App/Functions/Src/app_func_command.o \
./App/Functions/Src/app_func_logs.o \
./App/Functions/Src/app_func_measurement.o \
./App/Functions/Src/app_func_parameter.o \
./App/Functions/Src/app_func_state_machine.o \
./App/Functions/Src/app_func_stimulation.o 

C_DEPS += \
./App/Functions/Src/app_func_authentication.d \
./App/Functions/Src/app_func_ble.d \
./App/Functions/Src/app_func_command.d \
./App/Functions/Src/app_func_logs.d \
./App/Functions/Src/app_func_measurement.d \
./App/Functions/Src/app_func_parameter.d \
./App/Functions/Src/app_func_state_machine.d \
./App/Functions/Src/app_func_stimulation.d 


# Each subdirectory must supply rules for building sources it contributes
App/Functions/Src/%.o App/Functions/Src/%.su App/Functions/Src/%.cyclo: ../App/Functions/Src/%.c App/Functions/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Functions-2f-Src

clean-App-2f-Functions-2f-Src:
	-$(RM) ./App/Functions/Src/app_func_authentication.cyclo ./App/Functions/Src/app_func_authentication.d ./App/Functions/Src/app_func_authentication.o ./App/Functions/Src/app_func_authentication.su ./App/Functions/Src/app_func_ble.cyclo ./App/Functions/Src/app_func_ble.d ./App/Functions/Src/app_func_ble.o ./App/Functions/Src/app_func_ble.su ./App/Functions/Src/app_func_command.cyclo ./App/Functions/Src/app_func_command.d ./App/Functions/Src/app_func_command.o ./App/Functions/Src/app_func_command.su ./App/Functions/Src/app_func_logs.cyclo ./App/Functions/Src/app_func_logs.d ./App/Functions/Src/app_func_logs.o ./App/Functions/Src/app_func_logs.su ./App/Functions/Src/app_func_measurement.cyclo ./App/Functions/Src/app_func_measurement.d ./App/Functions/Src/app_func_measurement.o ./App/Functions/Src/app_func_measurement.su ./App/Functions/Src/app_func_parameter.cyclo ./App/Functions/Src/app_func_parameter.d ./App/Functions/Src/app_func_parameter.o ./App/Functions/Src/app_func_parameter.su ./App/Functions/Src/app_func_state_machine.cyclo ./App/Functions/Src/app_func_state_machine.d ./App/Functions/Src/app_func_state_machine.o ./App/Functions/Src/app_func_state_machine.su ./App/Functions/Src/app_func_stimulation.cyclo ./App/Functions/Src/app_func_stimulation.d ./App/Functions/Src/app_func_stimulation.o ./App/Functions/Src/app_func_stimulation.su

.PHONY: clean-App-2f-Functions-2f-Src

