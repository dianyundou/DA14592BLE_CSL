################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

-include makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utils/subdir.mk
-include startup/DA1459x/GCC/subdir.mk
-include startup/DA1459x/subdir.mk
-include startup/subdir.mk
-include sdk/util/src/subdir.mk
-include sdk/sys_man/subdir.mk
-include sdk/peripherals/src/subdir.mk
-include sdk/osal/subdir.mk
-include sdk/memory/src/subdir.mk
-include sdk/ble/stack/da14590/src/subdir.mk
-include sdk/ble/services/src/subdir.mk
-include sdk/ble/manager/src/subdir.mk
-include sdk/ble/clients/src/subdir.mk
-include sdk/ble/api/src/subdir.mk
-include sdk/ble/adapter/src/subdir.mk
-include sdk/asymmetric_suota/subdir.mk
-include sdk/afmn/wolfssl/port/DA1459x/subdir.mk
-include sdk/afmn/src/subdir.mk
-include sdk/adapters/src/subdir.mk
-include sdk/FreeRTOS/portable/MemMang/subdir.mk
-include sdk/FreeRTOS/portable/GCC/DA1459x/subdir.mk
-include sdk/FreeRTOS/portable/Common/subdir.mk
-include sdk/FreeRTOS/subdir.mk
-include afmn_support/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(JMP_DEPS)),)
-include $(JMP_DEPS)
endif
ifneq ($(strip $(FSY_DEPS)),)
-include $(FSY_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(SRC_UPPER_DEPS)),)
-include $(SRC_UPPER_DEPS)
endif
ifneq ($(strip $(P_UPPER_DEPS)),)
-include $(P_UPPER_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(PP_UPPER_DEPS)),)
-include $(PP_UPPER_DEPS)
endif
ifneq ($(strip $(P_DEPS)),)
-include $(P_DEPS)
endif
ifneq ($(strip $(FSY_UPPER_DEPS)),)
-include $(FSY_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(SRC_DEPS)),)
-include $(SRC_DEPS)
endif
ifneq ($(strip $(JMP_UPPER_DEPS)),)
-include $(JMP_UPPER_DEPS)
endif
ifneq ($(strip $(PP_DEPS)),)
-include $(PP_DEPS)
endif
ifneq ($(strip $(SX_DEPS)),)
-include $(SX_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(SX_UPPER_DEPS)),)
-include $(SX_UPPER_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
fmn_accessory.bin \

SECONDARY_SIZE += \
fmn_accessory.siz \


# All Target
all:
	$(MAKE) --no-print-directory pre-build
	$(MAKE) --no-print-directory main-build

# Main-build Target
main-build: fmn_accessory.elf secondary-outputs

# Tool invocations
fmn_accessory.elf: $(OBJS) $(USER_OBJS)
	$(file > fmn_accessory.elf.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -T "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\projects\\dk_apps\\demos\\fmn_accessory/DA1459x-00-Debug_eFLASH/mem.ld" -T "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\projects\\dk_apps\\demos\\fmn_accessory/DA1459x-00-Debug_eFLASH/sections.ld" -Xlinker --gc-sections -L "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\sdk\\bsp\\ldscripts\\ble_projects" -L "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\sdk\\interfaces\\ble\\binaries\\DA1459x-Release/" -L "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\sdk\\middleware\\intrinsic\\lib" -L "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\utilities\\apple_fmn\\findmy_network_adk\\lib" -L "C:\\RenesasWork\\BLETag\\ble_tag_workspace2\\CSL-59x_10.679.2.17\\utilities\\apple_fmn\\wolfssl\\lib" -Wl,-Map,"fmn_accessory.map" --specs=nano.specs --specs=nosys.specs -o "fmn_accessory.elf" $(OBJS) $(USER_OBJS) $(LIBS))
	@echo Building target: $@ && arm-none-eabi-gcc @"fmn_accessory.elf.in"
fmn_accessory.bin: fmn_accessory.elf
	arm-none-eabi-objcopy -O binary "fmn_accessory.elf"  "fmn_accessory.bin"
fmn_accessory.siz: fmn_accessory.elf
	arm-none-eabi-size --format=berkeley "fmn_accessory.elf"
# Other Targets
clean:
	-$(file > clean_file.tmp,  $(JMP_DEPS)  $(FSY_DEPS)  $(C_UPPER_DEPS)  $(SECONDARY_SIZE)  $(SRC_UPPER_DEPS)  $(P_UPPER_DEPS)  $(S_DEPS)  $(PP_UPPER_DEPS)  $(P_DEPS)  $(FSY_UPPER_DEPS)  $(C_DEPS)  $(SRC_DEPS)  $(JMP_UPPER_DEPS)  $(PP_DEPS)  $(SX_DEPS)  $(ASM_UPPER_DEPS)  $(SX_UPPER_DEPS)  $(OBJS)  $(SECONDARY_FLASH)  $(ASM_DEPS)  $(SREC)  $(S_UPPER_DEPS)  $(MAP) )
	-@ xargs -P 2 -s 32000 -t -a clean_file.tmp $(RM)
	-@$(RM) clean_file.tmp
	-$(RM)  fmn_accessory.elf
	-@echo ' '

pre-build:
	-@echo 'Generate linker scripts.'
	-+make generate_ldscripts DEVICE=DA14592_00 PRE_BUILD_EXTRA_DEFS="" APP_CONFIG_H="C:\RenesasWork\BLETag\ble_tag_workspace2\CSL-59x_10.679.2.17\projects\dk_apps\demos\fmn_accessory\config\custom_config_eflash.h" CC="arm-none-eabi-gcc" APP_CONFIG_DIR="C:\RenesasWork\BLETag\ble_tag_workspace2\CSL-59x_10.679.2.17\projects\dk_apps\demos\fmn_accessory\config" BSP_CONFIG_DIR="C:\RenesasWork\BLETag\ble_tag_workspace2\CSL-59x_10.679.2.17\sdk\bsp\config" MIDDLEWARE_CONFIG_DIR="C:\RenesasWork\BLETag\ble_tag_workspace2\CSL-59x_10.679.2.17\sdk\middleware\config" LDSCRIPT_PATH="C:\RenesasWork\BLETag\ble_tag_workspace2\CSL-59x_10.679.2.17\sdk\bsp\ldscripts\ble_projects"
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents main-build pre-build

-include ../makefile.targets
