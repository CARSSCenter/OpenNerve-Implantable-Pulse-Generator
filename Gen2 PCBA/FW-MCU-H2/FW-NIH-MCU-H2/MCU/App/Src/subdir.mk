################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../App/Src/app.c \
../App/Src/app_mode_battery_test.c \
../App/Src/app_mode_ble_active.c \
../App/Src/app_mode_ble_connection.c \
../App/Src/app_mode_bsl.c \
../App/Src/app_mode_impedance_test.c \
../App/Src/app_mode_oad.c \
../App/Src/app_mode_therapy_session.c \
../App/Src/app_state.c 

OBJS += \
./App/Src/app.o \
./App/Src/app_mode_battery_test.o \
./App/Src/app_mode_ble_active.o \
./App/Src/app_mode_ble_connection.o \
./App/Src/app_mode_bsl.o \
./App/Src/app_mode_impedance_test.o \
./App/Src/app_mode_oad.o \
./App/Src/app_mode_therapy_session.o \
./App/Src/app_state.o 

C_DEPS += \
./App/Src/app.d \
./App/Src/app_mode_battery_test.d \
./App/Src/app_mode_ble_active.d \
./App/Src/app_mode_ble_connection.d \
./App/Src/app_mode_bsl.d \
./App/Src/app_mode_impedance_test.d \
./App/Src/app_mode_oad.d \
./App/Src/app_mode_therapy_session.d \
./App/Src/app_state.d 


# Each subdirectory must supply rules for building sources it contributes
App/Src/%.o App/Src/%.su App/Src/%.cyclo: ../App/Src/%.c App/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-App-2f-Src

clean-App-2f-Src:
	-$(RM) ./App/Src/app.cyclo ./App/Src/app.d ./App/Src/app.o ./App/Src/app.su ./App/Src/app_mode_battery_test.cyclo ./App/Src/app_mode_battery_test.d ./App/Src/app_mode_battery_test.o ./App/Src/app_mode_battery_test.su ./App/Src/app_mode_ble_active.cyclo ./App/Src/app_mode_ble_active.d ./App/Src/app_mode_ble_active.o ./App/Src/app_mode_ble_active.su ./App/Src/app_mode_ble_connection.cyclo ./App/Src/app_mode_ble_connection.d ./App/Src/app_mode_ble_connection.o ./App/Src/app_mode_ble_connection.su ./App/Src/app_mode_bsl.cyclo ./App/Src/app_mode_bsl.d ./App/Src/app_mode_bsl.o ./App/Src/app_mode_bsl.su ./App/Src/app_mode_impedance_test.cyclo ./App/Src/app_mode_impedance_test.d ./App/Src/app_mode_impedance_test.o ./App/Src/app_mode_impedance_test.su ./App/Src/app_mode_oad.cyclo ./App/Src/app_mode_oad.d ./App/Src/app_mode_oad.o ./App/Src/app_mode_oad.su ./App/Src/app_mode_therapy_session.cyclo ./App/Src/app_mode_therapy_session.d ./App/Src/app_mode_therapy_session.o ./App/Src/app_mode_therapy_session.su ./App/Src/app_state.cyclo ./App/Src/app_state.d ./App/Src/app_state.o ./App/Src/app_state.su

.PHONY: clean-App-2f-Src

