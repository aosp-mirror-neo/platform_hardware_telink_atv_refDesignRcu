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
-include vendor/827x_ble_remote/pke_common/subdir.mk
-include vendor/827x_ble_remote/app_fmr/subdir.mk
-include vendor/827x_ble_remote/app_find_me/subdir.mk
-include drivers/8278/subdir.mk
-include drivers/8278/flash/subdir.mk
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

FLASH_IMAGE += \

SIZEDUMMY += \
sizedummy \


# All Target
all: GoogleRCU_8271.elf secondary-outputs

# Tool invocations
GoogleRCU_8271.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: TC32 C Linker'
	tc32-elf-ld --gc-sections -L../proj_lib -T ../boot.link -o"GoogleRCU_8271.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

: GoogleRCU_8271.elf
	@echo 'Invoking: TC32 Create Extended Listing'
	tc32-elf-objdump GoogleRCU_8271.elf  >
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
	-"../tl_check_fw.sh"   8271_ble_remote   GoogleRCU_8271
	-@echo ' '

secondary-outputs: $(LST) $(FLASH_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
