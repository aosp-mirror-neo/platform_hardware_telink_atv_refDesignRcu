################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/8278/adc.c \
../drivers/8278/aes.c \
../drivers/8278/analog.c \
../drivers/8278/audio.c \
../drivers/8278/bsp.c \
../drivers/8278/clock.c \
../drivers/8278/emi.c \
../drivers/8278/flash.c \
../drivers/8278/gpio_8278.c \
../drivers/8278/i2c.c \
../drivers/8278/lpc.c \
../drivers/8278/qdec.c \
../drivers/8278/rf_pa.c \
../drivers/8278/s7816.c \
../drivers/8278/spi.c \
../drivers/8278/timer.c \
../drivers/8278/uart.c \
../drivers/8278/watchdog.c 

OBJS += \
./drivers/8278/adc.o \
./drivers/8278/aes.o \
./drivers/8278/analog.o \
./drivers/8278/audio.o \
./drivers/8278/bsp.o \
./drivers/8278/clock.o \
./drivers/8278/emi.o \
./drivers/8278/flash.o \
./drivers/8278/gpio_8278.o \
./drivers/8278/i2c.o \
./drivers/8278/lpc.o \
./drivers/8278/qdec.o \
./drivers/8278/rf_pa.o \
./drivers/8278/s7816.o \
./drivers/8278/spi.o \
./drivers/8278/timer.o \
./drivers/8278/uart.o \
./drivers/8278/watchdog.o 


# Each subdirectory must supply rules for building sources it contributes
drivers/8278/%.o: ../drivers/8278/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"../" -D__PROJECT_8278_BLE_REMOTE__=1 -DCHIP_TYPE=CHIP_TYPE_8278 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


