/**
 ****************************************************************************************
 *
 * @file afmn_defaults.h
 *
 * @brief Apple FMN default configuration
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AFMN_DEFAULTS_H_
#define AFMN_DEFAULTS_H_

/* -------------------------------- Generic definitions --------------------------------- */

/*
 * Apple FMN accessory capabilities
 */
#define AFMN_ACCESSORY_CAPABILITY_PLAY_SOUND            (1 << 0)  /* Support play sound */
#define AFMN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT      (1 << 1)  /* Support motion detector UT */
#define AFMN_ACCESSORY_CAPABILITY_SRNM_LOOKUP_BLE       (1 << 3)  /* Support serial number lookup by BLE */
#define AFMN_ACCESSORY_CAPABILITY_FW_UPDATE_SERVICE     (1 << 4)  /* Support firmware update service */

/*
 * Apple FMN battery types
 */
#define AFMN_BATTERY_TYPE_POWERED                       (0)
#define AFMN_BATTERY_TYPE_NONRECHARGEABLE               (1)
#define AFMN_BATTERY_TYPE_RECHARGEABLE                  (2)

/*
 * Apple FMN configuration options
 */
#define AFMN_CONFIG_OPTION_NORMAL                       (0)
#define AFMN_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP     (1)

/*
 * Accuracy of the clock used as source for OS timers
 */
#define AFMN_TIMER_CLK_ACCURACY_LOW                     (0)
#define AFMN_TIMER_CLK_ACCURACY_HIGH                    (1)

/**
 * \brief Zero-initialized Apple FMN pairing process data retained memory attribute
 */
#define __AFMN_PAIR_RETAINED            __attribute__((section("afmn_pair_retention_mem_zi")))

/**
 * \brief Zero-initialized Apple FMN unpaired state data retained memory attribute
 */
#define __AFMN_UNPAIRED_RETAINED        __attribute__((section("afmn_unpaired_retention_mem_zi")))

/* ----------------------------- Configuration definitions ------------------------------ */

/**
 * \brief Apple FMN framework configuration file
 *
 * This header file specifies the compile-time configuration of Apple FMN framework.
 * It shall always be defined.
 */
#ifndef AFMN_CONFIG_FILE
#error "AFMN_CONFIG_FILE is not defined!"
#endif

/**
 * \brief Apple FMN product data
 *
 * Apple FMN product data shall always be defined in { ... } and have 8 bytes length.
 *
 * Example definition:
 * \code{.c}
 * #define AFMN_PRODUCT_DATA               { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 }
 * \endcode
 */
#ifndef AFMN_PRODUCT_DATA
#error "AFMN_PRODUCT_DATA is not defined!"
#endif

/**
 * \brief Apple FMN manufacturer name
 *
 * Apple FMN manufacturer name shall always be defined.
 */
#ifndef AFMN_MANUFACTURER_NAME
#error "AFMN_MANUFACTURER_NAME is not defined!"
#endif

/**
 * \brief Apple FMN model name
 *
 * Apple FMN model name shall always be defined.
 */
#ifndef AFMN_MODEL_NAME
#error "AFMN_MODEL_NAME is not defined!"
#endif

/**
 * \brief Apple FMN accessory category
 *
 * Apple FMN accessory category shall always be defined.
 */
#ifndef AFMN_ACCESSORY_CATEGORY
#error "AFMN_ACCESSORY_CATEGORY is not defined!"
#endif

/**
 * \brief Apple FMN accessory capability
 *
 * - AFMN_ACCESSORY_CAPABILITY_PLAY_SOUND
 * - AFMN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT
 * - AFMN_ACCESSORY_CAPABILITY_SRNM_LOOKUP_BLE
 * - AFMN_ACCESSORY_CAPABILITY_FW_UPDATE_SERVICE
 */
#ifndef AFMN_ACCESSORY_CAPABILITY
#define AFMN_ACCESSORY_CAPABILITY               (AFMN_ACCESSORY_CAPABILITY_PLAY_SOUND | \
                                                 AFMN_ACCESSORY_CAPABILITY_SRNM_LOOKUP_BLE)
#endif

/**
 * \brief Major version number of Apple FMN accessory firmware
 *
 * Major version number of Apple FMN accessory firmware shall always be defined.
 */
#ifndef AFMN_FW_VERSION_MAJOR
#error "AFMN_FW_VERSION_MAJOR is not defined!"
#endif

/**
 * \brief Minor version number of Apple FMN accessory firmware
 *
 * Minor version number of Apple FMN accessory firmware shall always be defined.
 */
#ifndef AFMN_FW_VERSION_MINOR
#error "AFMN_FW_VERSION_MINOR is not defined!"
#endif

/**
 * \brief Revision version number of Apple FMN accessory firmware
 *
 * Revision version number of Apple FMN accessory firmware shall always be defined.
 */
#ifndef AFMN_FW_VERSION_REVISION
#error "AFMN_FW_VERSION_REVISION is not defined!"
#endif

/**
 * \brief Apple FMN battery type
 *
 * - AFMN_BATTERY_TYPE_POWERED
 * - AFMN_BATTERY_TYPE_NONRECHARGEABLE
 * - AFMN_BATTERY_TYPE_RECHARGEABLE
 */
#ifndef AFMN_BATTERY_TYPE
#define AFMN_BATTERY_TYPE                       (AFMN_BATTERY_TYPE_NONRECHARGEABLE)
#endif

/**
 * \brief Apple FMN medium battery level, range 0-100
 *
 * Battery level less-equal than which the battery capacity is medium.
 */
#ifndef AFMN_BATTERY_LEVEL_MEDIUM
#define AFMN_BATTERY_LEVEL_MEDIUM               (70)
#endif

/**
 * \brief Apple FMN low battery level, range 0-100
 *
 * Battery level less-equal than which the battery capacity is low.
 */
#ifndef AFMN_BATTERY_LEVEL_LOW
#define AFMN_BATTERY_LEVEL_LOW                  (30)
#endif

#if (AFMN_BATTERY_LEVEL_LOW > AFMN_BATTERY_LEVEL_MEDIUM)
#error "AFMN_BATTERY_LEVEL_LOW cannot be greater than AFMN_BATTERY_LEVEL_MEDIUM."
#endif

/**
 * \brief Apple FMN critically low battery level, range 0-100
 *
 * Battery level less-equal than which the battery capacity is critically low.
 */
#ifndef AFMN_BATTERY_LEVEL_CRITICAL
#define AFMN_BATTERY_LEVEL_CRITICAL             (10)
#endif

#if (AFMN_BATTERY_LEVEL_CRITICAL > AFMN_BATTERY_LEVEL_LOW)
#error "AFMN_BATTERY_LEVEL_CRITICAL cannot be greater than AFMN_BATTERY_LEVEL_LOW."
#endif

/**
 * \brief Apple FMN advertise TX power
 *
 * advertise TX power should be at least 4dBm
 */
#ifndef AFMN_ADV_TX_POWER
#define AFMN_ADV_TX_POWER                       (GAP_TX_POWER_4_5_dBm)
#endif

#if (AFMN_ADV_TX_POWER < GAP_TX_POWER_4_5_dBm)
#error "AFMN_ADV_TX_POWER cannot be lower than 4dBm."
#endif

/**
 * \brief Apple FMN TX power service level
 *
 * TX power service level should be at least 4dBm
 */
#ifndef AFMN_TX_POWER_SERVICE_LEVEL
#define AFMN_TX_POWER_SERVICE_LEVEL             (4)
#endif

#if (AFMN_TX_POWER_SERVICE_LEVEL < 4)
#error "AFMN_TX_POWER_SERVICE_LEVEL cannot be lower than 4dBm."
#endif

/**
 * \brief Apple FMN Play sound duration
 */
#ifndef AFMN_SOUND_DURATION
#define AFMN_SOUND_DURATION                     (10)
#endif

/**
 * \brief Apple FMN configuration option
 */
#ifndef AFMN_CONFIG_OPTION
#define AFMN_CONFIG_OPTION                      (AFMN_CONFIG_OPTION_NORMAL)
#endif

/**
 * \brief Accuracy of the clock used as source for OS timers
 *
 * If accuracy of the source clock is low, then an OS timer interval correction mechanism is enabled.
 * Refer to AFMN_OS_CORR_TIMER_MIN_INTV_MS for further configuration of the OS timer interval
 * correction mechanism.
 *
 * - AFMN_TIMER_CLK_ACCURACY_LOW
 * - AFMN_TIMER_CLK_ACCURACY_HIGH
 *
 * \sa AFMN_OS_CORR_TIMER_MIN_INTV_MS
 */
#ifndef AFMN_TIMER_CLK_ACCURACY
#define AFMN_TIMER_CLK_ACCURACY                 (AFMN_TIMER_CLK_ACCURACY_LOW)
#endif

#ifndef AFMN_OS_CORR_TIMERS
#define AFMN_OS_CORR_TIMERS                     (AFMN_TIMER_CLK_ACCURACY == AFMN_TIMER_CLK_ACCURACY_LOW)
#endif

#endif /* AFMN_DEFAULTS_H_ */
