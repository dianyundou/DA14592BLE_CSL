################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/deepsleep.S \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/exception_handlers.S \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/startup_da1459x.S \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/vector_table_da1459x.S 

OBJS += \
./startup/DA1459x/GCC/deepsleep.o \
./startup/DA1459x/GCC/exception_handlers.o \
./startup/DA1459x/GCC/startup_da1459x.o \
./startup/DA1459x/GCC/vector_table_da1459x.o 

SREC += \
fmn_accessory.srec 

S_UPPER_DEPS += \
./startup/DA1459x/GCC/deepsleep.d \
./startup/DA1459x/GCC/exception_handlers.d \
./startup/DA1459x/GCC/startup_da1459x.d \
./startup/DA1459x/GCC/vector_table_da1459x.d 

MAP += \
fmn_accessory.map 


# Each subdirectory must supply rules for building sources it contributes
startup/DA1459x/GCC/deepsleep.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/deepsleep.S
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -x assembler-with-cpp -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config/custom_config_eflash.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
startup/DA1459x/GCC/exception_handlers.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/exception_handlers.S
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -x assembler-with-cpp -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config/custom_config_eflash.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
startup/DA1459x/GCC/startup_da1459x.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/startup_da1459x.S
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -x assembler-with-cpp -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config/custom_config_eflash.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
startup/DA1459x/GCC/vector_table_da1459x.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/startup/DA1459x/GCC/vector_table_da1459x.S
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -x assembler-with-cpp -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/fmn_accessory/config/custom_config_eflash.h" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

