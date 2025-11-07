/**
 ****************************************************************************************
 *
 * @file fast_pair_config.h
 *
 * @brief Google Fast Pair framework configuration header file
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef FAST_PAIR_CONFIG_H_
#define FAST_PAIR_CONFIG_H_

/* ************************************************************************************** */
/* ****************************** Fast Pair configuration ******************************* */
/* ************************************************************************************** */

/* Fast Pair Model ID */
#define FP_MODEL_ID                             ( 0x123456 )

/* Anti-Spoofing key for Model ID */
#define FP_ANTI_SPOOFING_PRIVATE_KEY \
        { \
                0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, \
                0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF \
        }

/* Enable Find My Device Network extension */
#define FP_FMDN                                 ( 1 )

/* Device is a locator tag */
#define FP_LOCATOR_TAG                          ( 1 )

/* Default personalized name */
#if (FP_LOCATOR_TAG != 1)
#if (FP_FMDN == 1)
#define FP_PERSONALIZED_NAME                    "Renesas FMDN Accessory"
#else
#define FP_PERSONALIZED_NAME                    "Renesas Fast Pair Device"
#endif
#endif /* !FP_LOCATOR_TAG */

/*
 * Calibrated TX power for Fast Pair frames as received at 0m.
 * Represented as a signed integer, with 1dBm resolution.
 */
#if (dg_configDEFAULT_RADIO_OP_MODE == LOW_POWER_ALL_PHYS)
#define FP_CALIBRATED_TX_POWER_LEVEL            ( -30 )
#elif (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS)
#define FP_CALIBRATED_TX_POWER_LEVEL            ( -24 )
#endif

/* TX power when advertising Fast Pair frames */
#if (dg_configDEFAULT_RADIO_OP_MODE == LOW_POWER_ALL_PHYS)
#define FP_ADV_TX_POWER                         ( GAP_TX_POWER_MINUS_23_dBm )
#elif (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS)
#define FP_ADV_TX_POWER                         ( GAP_TX_POWER_MINUS_22_dBm )
#endif

/* Battery configuration - number of batteries */
#define FP_BATTERIES_COUNT                      ( 1 )

/* Accuracy of the clock used as source for OS timers */
#if (dg_configLP_CLK_DRIFT > 200)
#define FP_TIMER_CLK_ACCURACY                   ( FP_TIMER_CLK_ACCURACY_LOW )
#else
#define FP_TIMER_CLK_ACCURACY                   ( FP_TIMER_CLK_ACCURACY_HIGH )
#endif

/* Enable verbose logging for Fast Pair */
#define FP_LOG_ENABLE                           ( 0 )

#ifndef CONFIG_RETARGET
#undef FP_LOG_ENABLE
#define FP_LOG_ENABLE                           ( 0 )
#endif

#if (FP_FMDN == 1)
/* ************************************************************************************** */
/* ******************* Find My Device Network accessory configuration ******************* */
/* ************************************************************************************** */

/* Manufacturer name */
#define FP_FMDN_MANUFACTURER_NAME               "Renesas"

/* Model name */
#define FP_FMDN_MODEL_NAME                      "DA1459x"

/* Accessory category */
#define FP_FMDN_ACCESSORY_CATEGORY              ( 1 )  /* Location Tracker */

/* Accessory capabilities */
#define FP_FMDN_ACCESSORY_CAPABILITIES          ( FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND | \
                                                  FP_FMDN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT | \
                                                  FP_FMDN_ACCESSORY_CAPABILITY_ID_LOOKUP_BLE )

/* Firmware major version number */
#define FP_FMDN_FW_VERSION_MAJOR                1
/* Firmware minor version number */
#define FP_FMDN_FW_VERSION_MINOR                0
/* Firmware revision version number */
#define FP_FMDN_FW_VERSION_REVISION             0

/* Battery type */
#define FP_FMDN_BATTERY_TYPE                    ( FP_FMDN_BATTERY_TYPE_NONRECHARGEABLE )

/*
 * Calibrated TX power for FMDN frames as received at 0m.
 * Represented as a signed integer, with 1dBm resolution.
 */
#if (dg_configDEFAULT_RADIO_OP_MODE == LOW_POWER_ALL_PHYS)
#define FP_FMDN_CALIBRATED_TX_POWER_LEVEL       ( 2 )
#elif (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS)
#define FP_FMDN_CALIBRATED_TX_POWER_LEVEL       ( 4 )
#endif

/* TX power used by Provider after successful FMDN provisioning */
#if (dg_configDEFAULT_RADIO_OP_MODE == LOW_POWER_ALL_PHYS)
#define FP_FMDN_ADV_TX_POWER                    ( GAP_TX_POWER_4_5_dBm )
#elif (dg_configDEFAULT_RADIO_OP_MODE == HIGH_PERFORMANCE_ALL_PHYS)
#define FP_FMDN_ADV_TX_POWER                    ( GAP_TX_POWER_6_dBm )
#endif

/* The number of Provider components capable of ringing */
#define FP_FMDN_RING_COMPONENTS_NUM             ( 1 )
/* Ringing volume selection is available for Provider ringing components */
#define FP_FMDN_RINGING_CAPABILITIES            ( FP_FMDN_RINGING_VOLUME_AVAILABLE )

/* Enable verbose logging for FMDN */
#define FP_FMDN_LOG_ENABLE                      ( 0 )

#ifndef CONFIG_RETARGET
#undef FP_FMDN_LOG_ENABLE
#define FP_FMDN_LOG_ENABLE                      ( 0 )
#endif

#define FP_FMDN_FW_VERSION(mj, mn, rv)          _FP_FMDN_FW_VERSION(mj, mn, rv)
#define _FP_FMDN_FW_VERSION(mj, mn, rv)         #mj "." #mn "." #rv

#endif /* FP_FMDN */

/* ************************************************************************************** */

#include "../sw_version.h"

/* Software revision for Device Information Service */
#define defaultBLE_DIS_SW_REVISION              SW_VERSION

/* Firmware revision for Device Information Service */
#if (FP_FMDN == 1)
#define defaultBLE_DIS_FW_REVISION              FP_FMDN_FW_VERSION(FP_FMDN_FW_VERSION_MAJOR, \
                                                                   FP_FMDN_FW_VERSION_MINOR, \
                                                                   FP_FMDN_FW_VERSION_REVISION)
#else
#define defaultBLE_DIS_FW_REVISION              "1.0.0"
#endif /* FP_FMDN */

/*
 * Default definition for Google Fast Pair framework configuration file
 * (required for flashing NVPARAM storage)
 */
#ifndef FAST_PAIR_CONFIG_FILE
#define FAST_PAIR_CONFIG_FILE
#endif

/*
 * Include Google Fast Pair framework default configuration values
 * (relative path required for flashing NVPARAM storage)
 */
#include "../../../../../utilities/fast_pair/include/fp_defaults.h"

#endif /* FAST_PAIR_CONFIG_H_ */
