################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/croutine.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/event_groups.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/list.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/queue.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/stream_buffer.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/tasks.c \
C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/timers.c 

C_DEPS += \
./sdk/FreeRTOS/croutine.d \
./sdk/FreeRTOS/event_groups.d \
./sdk/FreeRTOS/list.d \
./sdk/FreeRTOS/queue.d \
./sdk/FreeRTOS/stream_buffer.d \
./sdk/FreeRTOS/tasks.d \
./sdk/FreeRTOS/timers.d 

OBJS += \
./sdk/FreeRTOS/croutine.o \
./sdk/FreeRTOS/event_groups.o \
./sdk/FreeRTOS/list.o \
./sdk/FreeRTOS/queue.o \
./sdk/FreeRTOS/stream_buffer.o \
./sdk/FreeRTOS/tasks.o \
./sdk/FreeRTOS/timers.o 

SREC += \
freertos_retarget.srec 

MAP += \
freertos_retarget.map 


# Each subdirectory must supply rules for building sources it contributes
sdk/FreeRTOS/croutine.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/croutine.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/event_groups.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/event_groups.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/list.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/list.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/queue.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/queue.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/stream_buffer.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/stream_buffer.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/tasks.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/tasks.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
sdk/FreeRTOS/timers.o: C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/timers.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/templates/freertos_retarget/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

