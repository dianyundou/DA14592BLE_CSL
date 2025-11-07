/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_CONFIG_DEFINITIONS Configuration Definitions
 *
 * \brief Doxygen documentation is not yet available for this module.
 *        Please check the source code file(s)
 *
 *\{
 */

/**
 ****************************************************************************************
 *
 * @file bsp_definitions.h
 *
 * @brief Board Support Package. System Configuration file definitions.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
 * All rights reserved. Confidential Information.
 *
 * This software ("Software") is supplied by Renesas Electronics Corporation and/or its
 * affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
 * revocable, non-sub-licensable right and license to use the Software, solely if used in
 * or together with Renesas products. You may make copies of this Software, provided this
 * copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
 * reserves the right to change or discontinue the Software at any time without notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
 * MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
 * AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
 * OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
 * SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
 * THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
 * SOFTWARE.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEFINITIONS_H_
#define BSP_DEFINITIONS_H_

/* ------------------------------------ DEVICE DEFINITIONS -------------------------------------- */

#include "bsp_device_definitions.h"

/* ------------------------------------ Generic definitions ------------------------------------- */

#define PRO_DK_DA1469x_BRD              1
#define PRO_DK_DA1468x_BRD              2
#define FPGA_DA1469x_BRD                3
#define FPGA_DA1468x_BRD                4
#define FPGA_DA1459x_BRD                5

#define LP_CLK_32000                    0
#define LP_CLK_32768                    1
#define LP_CLK_RCX                      2
#define LP_CLK_ANY                      3

#define LP_CLK_IS_ANALOG                0
#define LP_CLK_IS_DIGITAL               1

#define MODE_IS_MIRRORED                0
#define MODE_IS_CACHED                  1
#define MODE_IS_RAM                     MODE_IS_MIRRORED

#define NON_VOLATILE_IS_OTP             0       // Code is in OTP
#define NON_VOLATILE_IS_QSPI_FLASH      1       // Code is in QSPI Flash
#define NON_VOLATILE_IS_EMBEDDED_FLASH  2       // Code is in Embedded Flash
#define NON_VOLATILE_IS_NONE            3       // Debug mode! Code is in RAM!
#define NON_VOLATILE_IS_FLASH           NON_VOLATILE_IS_QSPI_FLASH      // For backward compatibility

#define EXT_CRYSTAL_IS_16M              0
#define EXT_CRYSTAL_IS_32M              1

#define DEVELOPMENT_MODE                0       // Code is built for debugging
#define PRODUCTION_MODE                 1       // Code is built for production

#define FLASH_IS_NOT_CONNECTED          0
#define FLASH_CONNECTED_TO_1V8          1

#define BATTERY_TYPE_2xNIMH             0
#define BATTERY_TYPE_3xNIMH             1
#define BATTERY_TYPE_LICOO2             2       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_LIMN2O4            3       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_NMC                4       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_LIFEPO4            5       // 2.5V discharge voltage, 3.65V charge voltage
#define BATTERY_TYPE_LINICOAIO2         6       // 3.0V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_CUSTOM             7
#define BATTERY_TYPE_NO_RECHARGE        8
#define BATTERY_TYPE_NO_BATTERY         9

#define BATTERY_TYPE_2xNIMH_ADC_VOLTAGE         (2785)
#define BATTERY_TYPE_3xNIMH_ADC_VOLTAGE         (4013)
#define BATTERY_TYPE_LICOO2_ADC_VOLTAGE         (3440)
#define BATTERY_TYPE_LIMN2O4_ADC_VOLTAGE        (3440)
#define BATTERY_TYPE_NMC_ADC_VOLTAGE            (3440)
#define BATTERY_TYPE_LIFEPO4_ADC_VOLTAGE        (2989)
#define BATTERY_TYPE_LINICOAIO2_ADC_VOLTAGE     (3440)


/*
 * The supported RF Front-End Modules
 */
#define FEM_NOFEM                       0
#define FEM_SKY66112_11                 1

/*
 * The BLE event notification user hook types
 */
#define BLE_EVENT_NOTIF_USER_ISR        0       /// User-defined hooks directly from ISR context
#define BLE_EVENT_NOTIF_USER_TASK       1       /// Notification of the user task, using task notifications.

/*
 * Definitions for the different USB suspend modes.
 */
#define USB_SUSPEND_MODE_NONE          0       /// No action, just stop the PLL clock
#define USB_SUSPEND_MODE_PAUSE         1       /// Pause the system and wake only from VBUS and USB interrupts
#define USB_SUSPEND_MODE_IDLE          2       /// System state changes to idle

/*
 * Definition for available MPU regions
 */
#define MPU_REGION_NONE                 -1
#define MPU_REGION_0                    0
#define MPU_REGION_1                    1
#define MPU_REGION_2                    2
#define MPU_REGION_3                    3
#define MPU_REGION_4                    4
#define MPU_REGION_5                    5
#define MPU_REGION_6                    6
#define MPU_REGION_7                    7

/*
 * Enable FCU bus error to cause a bus fault during boot.
 * Disabling it, is not recommended.
 */
#ifndef HW_FCU_ENABLE_BUS_ERROR_AT_BOOT
#define HW_FCU_ENABLE_BUS_ERROR_AT_BOOT (1)
#endif

/*
 * Presets for memory cell retention
 */
#define RETMEM_RETAIN_ALL              (0x0000)
#define RETMEM_RETAIN_NONE             (0x0015)

#endif /* BSP_DEFINITIONS_H_ */

/**
\}
\}
*/
