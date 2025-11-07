################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/adv_contol.c \
../utils/app_params.c \
../utils/battery_monitor.c \
../utils/fn_control.c \
../utils/led_control.c \
../utils/motion_detector.c \
../utils/sound_maker.c 

C_DEPS += \
./utils/adv_contol.d \
./utils/app_params.d \
./utils/battery_monitor.d \
./utils/fn_control.d \
./utils/led_control.d \
./utils/motion_detector.d \
./utils/sound_maker.d 

OBJS += \
./utils/adv_contol.o \
./utils/app_params.o \
./utils/battery_monitor.o \
./utils/fn_control.o \
./utils/led_control.o \
./utils/motion_detector.o \
./utils/sound_maker.o 

SREC += \
csl_accessory.srec 

MAP += \
csl_accessory.map 


# Each subdirectory must supply rules for building sources it contributes
utils/%.o: ../utils/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Werror -Wall -g3 -Ddg_configDEVICE=DA14592_00 -DRELEASE_BUILD -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/utils" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/afmn_support" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/fp_support" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/apple_fmn/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/apple_fmn/wolfssl/port/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/mbedtls_port" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/fast_pair/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/fast_pair/ble/services/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/libbo_crypto/third_party_crypto/uECC" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/adapters/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/adapter/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/api/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/manager/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/services/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/stack/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/interfaces/ble/stack/da14590/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/config" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/free_rtos/portable/GCC/DA1459x" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/memory/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/osal" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/peripherals/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/system/sys_man/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/bsp/util/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/sdk/middleware/intrinsic/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/libbo_crypto/third_party_crypto/mbedtls/include" -I"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/utilities/libbo_crypto/third_party_crypto/mbedtls/include/mbedtls" -include"C:/RenesasWork/BLETag/ble_tag_workspace2/CSL-59x_10.679.2.17/projects/dk_apps/demos/csl_accessory/config/custom_config_eflash.h" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

