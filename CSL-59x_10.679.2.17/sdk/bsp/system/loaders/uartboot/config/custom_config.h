/**
 ****************************************************************************************
 *
 * @file custom_config.h
 *
 * @brief Custom configuration file for non-FreeRTOS applications executing from RAM.
 *
 * Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef CUSTOM_CONFIG_RAM_H_
#define CUSTOM_CONFIG_RAM_H_

#include "bsp_definitions.h"

#define OS_BAREMETAL

#define __HEAP_SIZE                             0x1400

#define dg_configCODE_LOCATION                  NON_VOLATILE_IS_NONE

#define dg_configUSE_BOD                        (0)

#define dg_configUSE_DCDC                       (1)

#define dg_configFLASH_CONNECTED_TO             (FLASH_CONNECTED_TO_1V8)
#define dg_configUSE_HW_QSPI                    (1)
#define dg_configQSPI_AUTOMODE_ENABLE           (1)
#define dg_configQSPI_FLASH_AUTODETECT          (1)
// Disable the sanity check of the qspi memory detection to prevent the program execution from
// becoming stuck when the QSPI memory is not identified.
#define QSPI_AUTOMODE_SANITY_CHECK              (0)

#define dg_configPOWER_1V8_ACTIVE               (1)
#define dg_configPOWER_1V8_SLEEP                (1)


#define dg_configUSE_SW_CURSOR                  (1)

#define dg_configUART_DMA_SUPPORT               (0)


#define dg_configCRYPTO_ADAPTER                 (0)

#define dg_configUSE_HW_WKUP                    (0)

#define dg_configSUPPRESS_HelloMsg              (0)

#define dg_configVERIFY_QSPI_WRITE              (1)

#define dg_configFLASH_ADAPTER                  (1)
#define dg_configNVMS_ADAPTER                   (1)
#define dg_configNVMS_VES                       (0)

#define CONFIG_PARTITION_TABLE_CREATE           (0)
#define dg_configUSE_SYS_TCS                    (1)

#define dg_configUSE_HW_TIMER                   (1)
#define dg_configENABLE_MTB                     (0)

/* Alternative location of the partition table in flash. Used in the asymmetric SUOTA case */
#define PARTITION_TABLE_ADDR_ALT                (EFLASH_MEM1_VIRTUAL_BASE_ADDR + 0x2F800)

#include "bsp_defaults.h"
/* Include middleware default values */
#include "middleware_defaults.h"

#endif /* CUSTOM_CONFIG_RAM_H_ */
