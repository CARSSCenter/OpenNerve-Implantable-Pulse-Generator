################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/crc.c \
../Core/Src/gpdma.c \
../Core/Src/gpio.c \
../Core/Src/hash.c \
../Core/Src/i2c.c \
../Core/Src/icache.c \
../Core/Src/iwdg.c \
../Core/Src/lptim.c \
../Core/Src/main.c \
../Core/Src/memorymap.c \
../Core/Src/pka.c \
../Core/Src/rng.c \
../Core/Src/rtc.c \
../Core/Src/spi.c \
../Core/Src/stm32u5xx_hal_msp.c \
../Core/Src/stm32u5xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32u5xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/crc.o \
./Core/Src/gpdma.o \
./Core/Src/gpio.o \
./Core/Src/hash.o \
./Core/Src/i2c.o \
./Core/Src/icache.o \
./Core/Src/iwdg.o \
./Core/Src/lptim.o \
./Core/Src/main.o \
./Core/Src/memorymap.o \
./Core/Src/pka.o \
./Core/Src/rng.o \
./Core/Src/rtc.o \
./Core/Src/spi.o \
./Core/Src/stm32u5xx_hal_msp.o \
./Core/Src/stm32u5xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32u5xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/crc.d \
./Core/Src/gpdma.d \
./Core/Src/gpio.d \
./Core/Src/hash.d \
./Core/Src/i2c.d \
./Core/Src/icache.d \
./Core/Src/iwdg.d \
./Core/Src/lptim.d \
./Core/Src/main.d \
./Core/Src/memorymap.d \
./Core/Src/pka.d \
./Core/Src/rng.d \
./Core/Src/rtc.d \
./Core/Src/spi.d \
./Core/Src/stm32u5xx_hal_msp.d \
./Core/Src/stm32u5xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32u5xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32U585xx -DFLASH_LINES_128B -c -I../Core/Inc -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Config" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/exDrivers/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Libraries/ECDSA" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Core" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/Middlewares/EEPROM_Emul/Porting" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Bsp/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Functions/Inc" -I"/Users/alexbaldwin/Desktop/OpenNerveTest/OpenNerve PCBA documentation/Final docs and FW from Coforce/FW-MCU-H2-251226/FW-NIH-MCU-H2/App/Inc/DVT" -I../Drivers/STM32U5xx_HAL_Driver/Inc -I../Drivers/STM32U5xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32U5xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/crc.cyclo ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/gpdma.cyclo ./Core/Src/gpdma.d ./Core/Src/gpdma.o ./Core/Src/gpdma.su ./Core/Src/gpio.cyclo ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/hash.cyclo ./Core/Src/hash.d ./Core/Src/hash.o ./Core/Src/hash.su ./Core/Src/i2c.cyclo ./Core/Src/i2c.d ./Core/Src/i2c.o ./Core/Src/i2c.su ./Core/Src/icache.cyclo ./Core/Src/icache.d ./Core/Src/icache.o ./Core/Src/icache.su ./Core/Src/iwdg.cyclo ./Core/Src/iwdg.d ./Core/Src/iwdg.o ./Core/Src/iwdg.su ./Core/Src/lptim.cyclo ./Core/Src/lptim.d ./Core/Src/lptim.o ./Core/Src/lptim.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/memorymap.cyclo ./Core/Src/memorymap.d ./Core/Src/memorymap.o ./Core/Src/memorymap.su ./Core/Src/pka.cyclo ./Core/Src/pka.d ./Core/Src/pka.o ./Core/Src/pka.su ./Core/Src/rng.cyclo ./Core/Src/rng.d ./Core/Src/rng.o ./Core/Src/rng.su ./Core/Src/rtc.cyclo ./Core/Src/rtc.d ./Core/Src/rtc.o ./Core/Src/rtc.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/stm32u5xx_hal_msp.cyclo ./Core/Src/stm32u5xx_hal_msp.d ./Core/Src/stm32u5xx_hal_msp.o ./Core/Src/stm32u5xx_hal_msp.su ./Core/Src/stm32u5xx_it.cyclo ./Core/Src/stm32u5xx_it.d ./Core/Src/stm32u5xx_it.o ./Core/Src/stm32u5xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32u5xx.cyclo ./Core/Src/system_stm32u5xx.d ./Core/Src/system_stm32u5xx.o ./Core/Src/system_stm32u5xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.cyclo ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

