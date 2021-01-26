################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables
C_SRCS += \
../vendor/827x_ble_remote/pke_common/app_pke.c

OBJS += \
./vendor/827x_ble_remote/pke_common/app_pke.o


# Each subdirectory must supply rules for building sources it contributes
vendor/827x_ble_remote/pke_common/%.o: ../vendor/827x_ble_remote/pke_common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"../" -D__PROJECT_8278_BLE_REMOTE__=1 -DCHIP_TYPE=CHIP_TYPE_8278 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


