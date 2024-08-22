################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../vendor/827x_ble_remote/app_find_me/app_fms.c

OBJS += \
./vendor/827x_ble_remote/app_find_me/app_fms.o


# Each subdirectory must supply rules for building sources it contributes
vendor/827x_ble_remote/app_find_me/%.o: ../vendor/827x_ble_remote/app_find_me/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I../ -I../drivers/8278 -D__PROJECT_8278_BLE_REMOTE__=1 -DCHIP_TYPE=CHIP_TYPE_827x -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


