################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include vendor/common/subdir.mk
-include vendor/827x_ble_remote/subdir.mk
-include drivers/8278/subdir.mk
-include common/subdir.mk
-include boot/8278/subdir.mk
-include application/usbstd/subdir.mk
-include application/print/subdir.mk
-include application/keyboard/subdir.mk
-include application/app/subdir.mk
-include objects.mk

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LST += \
8278_ble_remote.lst \

FLASH_IMAGE += \

SIZEDUMMY += \
sizedummy \


# All Target
all: vulture_sdk_rcu_reference.elf secondary-outputs

# Tool invocations
vulture_sdk_rcu_reference.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	tc32-elf-ld --gc-sections -L"../proj_lib" -T ../boot.link -o"vulture_sdk_rcu_reference.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

8278_ble_remote.lst: vulture_sdk_rcu_reference.elf
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump -x -D -l -S vulture_sdk_rcu_reference.elf  >"8278_ble_remote.lst"
	@echo 'Finished building: $@'
	@echo ' '

: vulture_sdk_rcu_reference.elf
	@echo 'Create Flash image (binary format)'
	tc32-elf-objcopy -O binary vulture_sdk_rcu_reference.elf
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: vulture_sdk_rcu_reference.elf
	@echo 'Invoking: Print Size'
	tc32-elf-size -t vulture_sdk_rcu_reference.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(LST)$(SIZEDUMMY) vulture_sdk_rcu_reference.elf
	-@echo ' '

post-build:
	-"../tl_check_fw.sh"   8278_ble_remote   vulture_sdk_rcu_reference
	-@echo ' '

secondary-outputs: $(LST) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
