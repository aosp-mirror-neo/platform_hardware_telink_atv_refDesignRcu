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
-include boot/8271/subdir.mk
-include application/usbstd/subdir.mk
-include application/print/subdir.mk
-include application/keyboard/subdir.mk
-include application/audio/subdir.mk
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
all: GoogleRCU_8271.elf secondary-outputs

# Tool invocations
GoogleRCU_8271.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	tc32-elf-ld --gc-sections -L"../proj_lib" -L"../application\audio" -T ../boot.link -o"GoogleRCU_8271.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

8278_ble_remote.lst: GoogleRCU_8271.elf
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump -x -D -l -S GoogleRCU_8271.elf  >"8278_ble_remote.lst"
	@echo 'Finished building: $@'
	@echo ' '

: GoogleRCU_8271.elf
	@echo 'Create Flash image (binary format)'
	tc32-elf-objcopy -O binary GoogleRCU_8271.elf
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: GoogleRCU_8271.elf
	@echo 'Invoking: Print Size'
	tc32-elf-size -t GoogleRCU_8271.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(FLASH_IMAGE)$(ELFS)$(OBJS)$(LST)$(SIZEDUMMY) GoogleRCU_8271.elf
	-@echo ' '

post-build:
	-"../tl_check_fw.sh"   8278_ble_remote   GoogleRCU_8271
	-@echo ' '

secondary-outputs: $(LST) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
