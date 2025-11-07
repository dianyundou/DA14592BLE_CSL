/**
 ****************************************************************************************
 *
 * @file qspi_memory_config_table_internal.h
 *
 * @brief Header file which contains the QSPI memory configuration table
 *
 * When the memory autodetection functionality is enabled, the SDK reads the JEDEC ID of the
 * connected devices, and compares them with the JEDEC IDs of the QSPI flash/PSRAM drivers
 * contained in qspi_memory_config_table[]. If matched, the QSPIC/QSPIC2 are initialized with
 * the settings of the corresponding drivers(s).
 *
 * The SDK provides the option of implementing a custom qspi_memory_config_table[] in a new header
 * file. In this case, this file can be used as template. In order to build an application using the
 * custom qspi_memory_config_table[], the dg_configQSPI_MEMORY_CONFIG_TABLE_HEADER must be defined
 * with the name of aforementioned header file.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_
#define QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_

#include "qspi_mx25u3235_v2.h"
#include "qspi_w25q32jwiq_v2.h"
#include "qspi_w25q32jwim_v2.h"

const qspi_flash_config_t* qspi_memory_config_table[] = {
        &qspi_mx25u3235_cfg,
        &qspi_w25q32jwiq_cfg,
        &qspi_w25q32jwim_cfg,
};

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && (dg_configQSPI_FLASH_AUTODETECT == 1)
#if !defined (CUSTOM_PRODUCT_HEADERS_FLASH_CONFIG)
/*
 * The bootrom is designed to be memory agnostic, and it determines the appropriate configuration for
 * the QSPI Controller and the connected QSPI flash memory by retrieving the corresponding settings
 * from the product headers. When the auto-detect feature is disabled, the instantiation of the product
 * headers is done within the flash memory drivers specific to each memory. On the other hand, when
 * the auto-detect feature is enabled, the SDK instantiates the product headers here to provide a
 * low performance single SPI configuration for the QSPI Controller and memory. This default configuration
 * is designed to offer broad compatibility with the majority of QSPI flash memories.
 *
 * To customize these configurations and deviate from the default settings, you can define the
 * CUSTOM_PRODUCT_HEADERS_FLASH_CONFIG macro and instantiate your own ph_primary and ph_backup.
 * This allows you to tailor the QSPI configuration according to your specific requirements,
 * providing flexibility and customization options.
 */
#pragma message "The product headers provide a low performance single SPI configuration of the QSPIC." \
                " To customize this configuration provide your own implementation."

__attribute__((used, __section__("__product_header_primary__")))
static const PRODUCT_HEADER_STRUCT(1) ph_primary = {
        .burstcmdA = 0x00000003,
        .burstcmdB = 0x00000000,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0001,
        .config_seq = { 0x00 },
        .crc = 0x14EE
};

__attribute__((used, __section__("__product_header_backup__")))
static const PRODUCT_HEADER_STRUCT(1) ph_backup = {
        .burstcmdA = 0x00000003,
        .burstcmdB = 0x00000000,
        .flash_config_section = 0x11AA,
        .flash_config_length = 0x0001,
        .config_seq = { 0x00 },
        .crc = 0x14EE
};
#endif /* !defined (DEFINE_PRODUCT_HEADERS_FLASH_CONFIG) */
#endif /* (dg_configUSE_SEGGER_FLASH_LOADER == 1) &&  (dg_configQSPI_FLASH_AUTODETECT == 1) */

#endif /* QSPI_FLASH_CONFIG_TABLE_INTERNAL_H_ */
