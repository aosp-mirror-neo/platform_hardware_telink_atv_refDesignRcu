################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../vendor/common/blt_common.c \
../vendor/common/blt_fw_sign.c \
../vendor/common/blt_led.c \
../vendor/common/blt_soft_timer.c \
../vendor/common/flash_fw_check.c 

OBJS += \
./vendor/common/blt_common.o \
./vendor/common/blt_fw_sign.o \
./vendor/common/blt_led.o \
./vendor/common/blt_soft_timer.o \
./vendor/common/flash_fw_check.o 


# Each subdirectory must supply rules for building sources it contributes
vendor/common/%.o: ../vendor/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	tc32-elf-gcc -ffunction-sections -fdata-sections -I"../" -I"../\drivers\8278" -D__PROJECT_8278_BLE_REMOTE__=1 -DCHIP_TYPE=CHIP_TYPE_827x -Wall -O2 -fpack-struct -fshort-enums -finline-small-functions -std=gnu99 -fshort-wchar -fms-extensions -c -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


