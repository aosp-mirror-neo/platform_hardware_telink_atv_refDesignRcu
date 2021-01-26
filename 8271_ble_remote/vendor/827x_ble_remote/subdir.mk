################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/827x_ble_remote/app.c \
../vendor/827x_ble_remote/app_att.c \
../vendor/827x_ble_remote/app_audio.c \
../vendor/827x_ble_remote/app_custom.c \
../vendor/827x_ble_remote/app_ecdsa.c \
../vendor/827x_ble_remote/app_flash_write.c \
../vendor/827x_ble_remote/app_ir.c \
../vendor/827x_ble_remote/app_phytest.c \
../vendor/827x_ble_remote/app_test.c \
../vendor/827x_ble_remote/app_ui.c \
../vendor/827x_ble_remote/battery_check.c \
../vendor/827x_ble_remote/main.c \
../vendor/827x_ble_remote/rc_ir.c \
../vendor/827x_ble_remote/sha256.c

OBJS += \
./vendor/827x_ble_remote/app.o \
./vendor/827x_ble_remote/app_att.o \
./vendor/827x_ble_remote/app_audio.o \
./vendor/827x_ble_remote/app_custom.o \
./vendor/827x_ble_remote/app_ecdsa.o \
./vendor/827x_ble_remote/app_flash_write.o \
./vendor/827x_ble_remote/app_ir.o \
./vendor/827x_ble_remote/app_phytest.o \
./vendor/827x_ble_remote/app_test.o \
./vendor/827x_ble_remote/app_ui.o \
./vendor/827x_ble_remote/battery_check.o \
./vendor/827x_ble_remote/main.o \
./vendor/827x_ble_remote/rc_ir.o \
./vendor/827x_ble_remote/sha256.o


# Each subdirectory must supply rules for building sources it contributes
vendor/827x_ble_remote/%.o: ../vendor/827x_ble_remote/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"../" -D__PROJECT_8278_BLE_REMOTE__=1 -DCHIP_TYPE=CHIP_TYPE_8278 -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


