################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/chacha20.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_crc16.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_list.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_queue.c 

C_DEPS += \
./sdk/util/src/chacha20.d \
./sdk/util/src/sdk_crc16.d \
./sdk/util/src/sdk_list.d \
./sdk/util/src/sdk_queue.d 

OBJS += \
./sdk/util/src/chacha20.o \
./sdk/util/src/sdk_crc16.o \
./sdk/util/src/sdk_list.o \
./sdk/util/src/sdk_queue.o 

SREC += \
freertos_retarget.srec 

MAP += \
freertos_retarget.map 


# Each subdirectory must supply rules for building sources it contributes
sdk/util/src/chacha20.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/chacha20.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/util/src/sdk_crc16.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_crc16.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/util/src/sdk_list.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_list.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/util/src/sdk_queue.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/src/sdk_queue.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

