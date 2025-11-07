/**
 ****************************************************************************************
 *
 * @file afmn_config.h
 *
 * @brief Apple FMN configuration header file
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AFMN_CONFIG_H_
#define AFMN_CONFIG_H_

#include "fw_version.h"

/* Product data from MFi product plan */
#define AFMN_PRODUCT_DATA               { 0xF8, 0x6A, 0xE4, 0x3A, 0x2E, 0x03, 0xAC, 0x54 }

/* Manufacturer name */
#define AFMN_MANUFACTURER_NAME          "Renesas"

/* Model name */
#define AFMN_MODEL_NAME                 "DA1459x"

/* Accessory category */
#define AFMN_ACCESSORY_CATEGORY         ( 1 )  /* Finder */

/* Accessory capability */
#define AFMN_ACCESSORY_CAPABILITY       ( AFMN_ACCESSORY_CAPABILITY_PLAY_SOUND | \
                                          AFMN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT | \
                                          AFMN_ACCESSORY_CAPABILITY_SRNM_LOOKUP_BLE )

/* Firmware major version number */
#define AFMN_FW_VERSION_MAJOR           FW_VERSION_MAJOR
/* Firmware minor version number */
#define AFMN_FW_VERSION_MINOR           FW_VERSION_MINOR
/* Firmware revision version number */
#define AFMN_FW_VERSION_REVISION        FW_VERSION_REVISION

/* Battery type */
#define AFMN_BATTERY_TYPE               ( AFMN_BATTERY_TYPE_NONRECHARGEABLE )

/* Configuration option */
#define AFMN_CONFIG_OPTION              ( AFMN_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP )

/* Accuracy of the clock used as source for OS timers */
#if (dg_configLP_CLK_DRIFT > 200)
#define AFMN_TIMER_CLK_ACCURACY         ( AFMN_TIMER_CLK_ACCURACY_LOW )
#else
#define AFMN_TIMER_CLK_ACCURACY         ( AFMN_TIMER_CLK_ACCURACY_HIGH )
#endif

/* ************************************************************************************** */

/*
 * Default definition for Apple FMN framework configuration file
 * (required for flashing NVPARAM storage)
 */
#ifndef AFMN_CONFIG_FILE
#define AFMN_CONFIG_FILE
#endif

/*
 * Include Apple FMN framework default configuration values
 * (relative path required for flashing NVPARAM storage)
 */
#include "../../../../../utilities/apple_fmn/include/afmn_defaults.h"

#endif /* AFMN_CONFIG_H_ */
