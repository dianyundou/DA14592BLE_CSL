/**
 ****************************************************************************************
 *
 * @file fp_defaults.h
 *
 * @brief Google Fast Pair framework default configuration
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

#ifndef FP_DEFAULTS_H_
#define FP_DEFAULTS_H_

/* ************************************************************************************** */
/* ****************************** Fast Pair configuration ******************************* */
/* ************************************************************************************** */

/* -------------------------------- Generic definitions --------------------------------- */

/*
 * Account key length
 */
#define FP_ACC_KEYS_ACCOUNT_KEY_LENGTH          (16)

/*
 * Google Fast Pair framework configuration options
 */
#define FP_CONFIG_OPTION_NORMAL                         (0)
#define FP_CONFIG_OPTION_BLE_DB_CONTROLLED_BY_APP       (1)

/*
 * Accuracy of the clock used as source for OS timers
 */
#define FP_TIMER_CLK_ACCURACY_LOW               (0)
#define FP_TIMER_CLK_ACCURACY_HIGH              (1)

/* ----------------------------- Configuration definitions ------------------------------ */

/**
 * \brief Google Fast Pair framework configuration file
 *
 * This header file specifies the compile-time configuration of Google Fast Pair framework.
 * It shall always be defined.
 */
#ifndef FAST_PAIR_CONFIG_FILE
#error "FAST_PAIR_CONFIG_FILE is not defined!"
#endif

/**
 * \brief Fast Pair Model ID
 *
 * Fast Pair Model ID shall always be defined.
 *
 * Example definition:
 * \code{.c}
 * #define FP_MODEL_ID                             ( 0x123456 )
 * \endcode
 */
#ifndef FP_MODEL_ID
#error "FP_MODEL_ID is not defined!"
#endif

/**
 * \brief Battery level notification extension is enabled
 */
#ifndef FP_BATTERY_NOTIFICATION
#define FP_BATTERY_NOTIFICATION                 (1)
#endif

/**
 * \brief Default personalized name if personalized name extension is enabled
 *
 * Fast Pair personalized name (FP_PERSONALIZED_NAME) shall be defined as string when corresponding
 * Fast Pair extension is enabled, indicating the default name that will be returned to the Seeker
 * if no name has been stored to NVM storage.
 */
#ifdef FP_PERSONALIZED_NAME
#define FP_DEFAULT_PERSONALIZED_NAME            "" FP_PERSONALIZED_NAME
#endif

/**
 * \brief Find My Device Network extension is enabled
 */
#ifndef FP_FMDN
#define FP_FMDN                                 (1)
#endif

/**
 * \brief Device is a locator tag
 */
#ifndef FP_LOCATOR_TAG
#define FP_LOCATOR_TAG                          (1)
#endif

/**
 * \brief Max length (in bytes) of Fast Pair personalized name
 *
 *
 */
#ifndef FP_PERSONALIZED_NAME_MAX_LENGTH
#define FP_PERSONALIZED_NAME_MAX_LENGTH         (64)
#endif

#if (FP_PERSONALIZED_NAME_MAX_LENGTH < 64)
#error "The minimum value for FP_PERSONALIZED_NAME_MAX_LENGTH is 64 bytes."
#endif

/**
 * \brief Anti-Spoofing private key for Model ID
 *
 * Anti-spoofing private key shall always be defined.
 *
 * Example definition:
 * \code{.c}
 * #define FP_ANTI_SPOOFING_PRIVATE_KEY \
 *         { \
 *                 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, \
 *                 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF \
 *         }
 * \endcode
 */
#ifndef FP_ANTI_SPOOFING_PRIVATE_KEY
#error "FP_ANTI_SPOOFING_PRIVATE_KEY is not defined!"
#endif

/**
 * \brief Number of stored account keys
 *
 * Number of stored account keys shall be at least 5.
 */
#ifndef FP_ACCOUNT_KEYS_COUNT
#define FP_ACCOUNT_KEYS_COUNT                   (5)
#endif

#if (FP_ACCOUNT_KEYS_COUNT < 5)
#error "The minimum value for FP_ACCOUNT_KEYS_COUNT is 5."
#endif

/**
 * \brief Calibrated TX power level for Fast Pair frames as received at 0m
 *
 * Calibrated TX power level is represented as a signed integer, with 1dBm resolution.
 * It shall always be defined.
 */
#ifndef FP_CALIBRATED_TX_POWER_LEVEL
#error "FP_CALIBRATED_TX_POWER_LEVEL is not defined!"
#endif

/**
 * \brief Include TX power level for Fast Pair frames, as received at 0m, in the advertising payload
 *
 * Calibrated TX power level shall be included in Fast Pair advertising payload if it is not
 * provided to Google with model registration.
 */
#ifndef FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL
#define FP_ADVERTISE_CALIBRATED_TX_POWER_LEVEL  (1)
#endif

/**
 * \brief TX power when advertising Fast Pair frames
 *
 * TX power when advertising shall always be defined (gap_tx_power_t).
 */
#ifndef FP_ADV_TX_POWER
#error "FP_ADV_TX_POWER is not defined!"
#endif

/**
 * \brief Battery configuration - number of batteries
 *
 * Number of batteries shall be less than 4.
 */
#ifndef FP_BATTERIES_COUNT
#define FP_BATTERIES_COUNT                      (1)
#endif

#if (FP_BATTERIES_COUNT > 3)
#error "The maximum value for FP_BATTERIES_COUNT is 3."
#endif

/**
 * \brief Maximum number of pairing failures
 */
#ifndef FP_MAX_PAIRING_FAILURES
#define FP_MAX_PAIRING_FAILURES                 (10)
#endif

/**
 * \brief Use hardware crypto block
 */
#ifndef FP_AES_HW
#define FP_AES_HW                               (1)
#endif

/**
 * \brief Enable Device Information Service (DIS)
 *
 * Device Information Service (DIS) is registered by Google Fast Pair framework.
 */
#ifndef FP_DIS_ENABLE
#define FP_DIS_ENABLE                           (1)
#endif

/**
 * \brief Google Fast Pair Framework configuration option
 */
#ifndef FP_CONFIG_OPTION
#define FP_CONFIG_OPTION                        (FP_CONFIG_OPTION_NORMAL)
#endif

/**
 * \brief Zero-initialized Fast Pair data retained memory attribute
 */
#ifndef __FP_RETAINED
#define __FP_RETAINED                           __RETAINED
#endif

/**
 * \brief Accuracy of the clock used as source for OS timers
 *
 * If accuracy of the source clock is low, then an OS timer interval correction mechanism is enabled.
 * Refer to FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS for further configuration of the OS timer interval
 * correction mechanism.
 *
 * - FP_TIMER_CLK_ACCURACY_LOW
 * - FP_TIMER_CLK_ACCURACY_HIGH
 *
 * \sa FP_UTILS_CORR_OS_TIMER_MIN_INTV_MS
 */
#ifndef FP_TIMER_CLK_ACCURACY
#define FP_TIMER_CLK_ACCURACY                   (FP_TIMER_CLK_ACCURACY_LOW)
#endif

#ifdef AFMN_CONFIG_FILE
#ifdef FP_UTILS_CORR_OS_TIMERS
#undef FP_UTILS_CORR_OS_TIMERS
#warning "FP_UTILS_CORR_OS_TIMERS is not used in case of multiple finder network protocols support"
#endif
#else
#ifndef FP_UTILS_CORR_OS_TIMERS
#define FP_UTILS_CORR_OS_TIMERS                 (FP_TIMER_CLK_ACCURACY == FP_TIMER_CLK_ACCURACY_LOW)
#endif
#endif /* AFMN_CONFIG_FILE */

/**
 * \brief Execute bloom filter test on start-up
 */
#ifndef FP_TEST_BLOOM_FILTER
#define FP_TEST_BLOOM_FILTER                    (0)
#endif

/**
 * \brief Test "key pairing" i.e. Fast Pair pairing characteristic callback
 */
#ifndef FP_TEST_KEY_PAIRING
#define FP_TEST_KEY_PAIRING                     (0)
#endif

/**
 * \brief Verbose logging for Fast Pair
 */
#ifndef FP_LOG_ENABLE
#define FP_LOG_ENABLE                           (0)
#endif

#if (FP_LOG_ENABLE == 1)
#define FP_LOG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define FP_LOG_PRINTF(fmt, ...)
#endif

#if (FP_FMDN == 0)
#undef FP_LOCATOR_TAG
#define FP_LOCATOR_TAG                          (0)
#endif

#if (FP_FMDN == 1)
/* ************************************************************************************** */
/* ******************* Find My Device Network accessory configuration ******************* */
/* ************************************************************************************** */

#if (FP_LOCATOR_TAG == 1)
#if (FP_BATTERY_NOTIFICATION == 1)
#undef FP_BATTERY_NOTIFICATION
#define FP_BATTERY_NOTIFICATION                         (0)
#endif /* FP_BATTERY_NOTIFICATION */
#else
#error "Only locator tag device is currently supported for Google FMDN."
#endif /* FP_LOCATOR_TAG */

/* -------------------------------- Generic definitions --------------------------------- */

/*
 * FMDN accessory capabilities
 */
#define FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND         (1 << 0)  /* Support play sound */
#define FP_FMDN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT   (1 << 1)  /* Support motion detection UT */
#define FP_FMDN_ACCESSORY_CAPABILITY_ID_LOOKUP_BLE      (1 << 3)  /* Support identifier lookup by BLE */

/*
 * FMDN battery types
 */
#define FP_FMDN_BATTERY_TYPE_POWERED                    (0)
#define FP_FMDN_BATTERY_TYPE_NONRECHARGEABLE            (1)
#define FP_FMDN_BATTERY_TYPE_RECHARGEABLE               (2)

/*
 * Ephemeral identity key length for FMDN
 */
#define FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH           (32)

/*
 * FMDN ringing capabilities
 */
#define FP_FMDN_RINGING_VOLUME_NOT_AVAILABLE            (0x00)
#define FP_FMDN_RINGING_VOLUME_AVAILABLE                (0x01)

/*
 * Elliptic curve used for FMDN
 */
#define FP_FMDN_ELLIPTIC_CURVE_SECP160R1                (0x00)
#define FP_FMDN_ELLIPTIC_CURVE_SECP256R1                (0x01)

/* ----------------------------- Configuration definitions ------------------------------ */

/**
 * \brief FMDN product data is the same value as Fast Pair Model ID (8 bytes, 0 padded)
 */
#define FP_FMDN_PRODUCT_DATA                            FP_MODEL_ID

/**
 * \brief FMDN manufacturer name
 *
 * FMDN manufacturer name shall always be defined.
 */
#ifndef FP_FMDN_MANUFACTURER_NAME
#error "FP_FMDN_MANUFACTURER_NAME is not defined"
#endif

/**
 * \brief FMDN model name
 *
 * FMDN model name shall always be defined.
 */
#ifndef FP_FMDN_MODEL_NAME
#error "FP_FMDN_MODEL_NAME is not defined"
#endif

/**
 * \brief FMDN accessory category
 *
 * FMDN accessory category shall always be defined.
 */
#ifndef FP_FMDN_ACCESSORY_CATEGORY
#error "FP_FMDN_ACCESSORY_CATEGORY is not defined"
#endif

/**
 * \brief FMDN accessory capabilities
 *
 * - FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND
 * - FP_FMDN_ACCESSORY_CAPABILITY_UT_MOTION_DETECT
 * - FP_FMDN_ACCESSORY_CAPABILITY_ID_LOOKUP_BLE
 */
#ifndef FP_FMDN_ACCESSORY_CAPABILITIES
#define FP_FMDN_ACCESSORY_CAPABILITIES                  (FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND | \
                                                         FP_FMDN_ACCESSORY_CAPABILITY_ID_LOOKUP_BLE)
#endif

/**
 * \brief FMDN network ID for Google
 */
#define FP_FMDN_NETWORK_ID                              (0x02)  /* Google LLC */

/**
 * \brief Major version number of FMDN accessory firmware
 *
 * Major version number of FMDN accessory firmware shall always be defined.
 */
#ifndef FP_FMDN_FW_VERSION_MAJOR
#error "FP_FMDN_FW_VERSION_MAJOR is not defined!"
#endif

#if (FP_FMDN_FW_VERSION_MAJOR > 0xFFFF)
#error "Major version of FMDN accessory firmware cannot be greater than (2^16 - 1)."
#endif

/**
 * \brief Minor version number of FMDN accessory firmware
 *
 * Minor version number of FMDN accessory firmware shall always be defined.
 */
#ifndef FP_FMDN_FW_VERSION_MINOR
#error "FP_FMDN_FW_VERSION_MINOR is not defined!"
#endif

#if (FP_FMDN_FW_VERSION_MINOR > 0xFF)
#error "Minor version of FMDN accessory firmware cannot be greater than (2^8 - 1)."
#endif

/**
 * \brief Revision version number of FMDN accessory firmware
 *
 * Revision version number of FMDN accessory firmware shall always be defined.
 */
#ifndef FP_FMDN_FW_VERSION_REVISION
#error "FP_FMDN_FW_VERSION_REVISION is not defined!"
#endif

#if (FP_FMDN_FW_VERSION_REVISION > 0xFF)
#error "Revision version of FMDN accessory firmware cannot be greater than (2^8 - 1)."
#endif

/**
 * \brief FMDN battery type
 *
 * - FP_FMDN_BATTERY_TYPE_POWERED
 * - FP_FMDN_BATTERY_TYPE_NONRECHARGEABLE
 * - FP_FMDN_BATTERY_TYPE_RECHARGEABLE
 */
#ifndef FP_FMDN_BATTERY_TYPE
#define FP_FMDN_BATTERY_TYPE                            (FP_FMDN_BATTERY_TYPE_NONRECHARGEABLE)
#endif

#if (FP_BATTERIES_COUNT == 0) && (FP_FMDN_BATTERY_TYPE != FP_FMDN_BATTERY_TYPE_POWERED)
#error "Incorrect FP_FMDN_BATTERY_TYPE, there are no batteries (see FP_BATTERIES_COUNT)"
#endif
/**
 * \brief Medium battery level threshold for FMDN, range 0-100
 *
 * Battery level less-equal than which the battery capacity is medium.
 */
#ifndef FP_FMDN_BATTERY_LEVEL_MEDIUM
#define FP_FMDN_BATTERY_LEVEL_MEDIUM                    (70)
#endif

/**
 * \brief Low battery level threshold for FMDN, range 0-100
 *
 * Battery level less-equal than which the battery capacity is low.
 */
#ifndef FP_FMDN_BATTERY_LEVEL_LOW
#define FP_FMDN_BATTERY_LEVEL_LOW                       (30)
#endif

#if (FP_FMDN_BATTERY_LEVEL_LOW > FP_FMDN_BATTERY_LEVEL_MEDIUM)
#error "FP_FMDN_BATTERY_LEVEL_LOW cannot be greater than FP_FMDN_BATTERY_LEVEL_MEDIUM."
#endif

/**
 * \brief Critical battery level threshold for FMDN, range 0-100
 *
 * Battery level less-equal than which the battery capacity is critically low.
 */
#ifndef FP_FMDN_BATTERY_LEVEL_CRITICAL
#define FP_FMDN_BATTERY_LEVEL_CRITICAL                  (10)
#endif

#if (FP_FMDN_BATTERY_LEVEL_CRITICAL > FP_FMDN_BATTERY_LEVEL_LOW)
#error "FP_FMDN_BATTERY_LEVEL_CRITICAL cannot be greater than FP_FMDN_BATTERY_LEVEL_LOW."
#endif

/**
 * \brief Calibrated TX power level for FMDN frames as received at 0m
 *
 * Calibrated TX power level is represented as a signed integer, with 1dBm resolution.
 * It shall always be defined.
 */
#ifndef FP_FMDN_CALIBRATED_TX_POWER_LEVEL
#error "FP_FMDN_CALIBRATED_TX_POWER_LEVEL is not defined!"
#endif

/**
 * \brief Advertising TX power when device is FMDN provisioned
 *
 * Advertising TX power when device is FMDN provisioned shall always be defined (gap_tx_power_t).
 */
#ifndef FP_FMDN_ADV_TX_POWER
#error "FP_FMDN_ADV_TX_POWER is not defined!"
#endif

/**
 * \brief Number of available ring components for FMDN
 *
 * Number of available ring components for FMDN shall be maximum 3.
 */
#ifndef FP_FMDN_RING_COMPONENTS_NUM
#define FP_FMDN_RING_COMPONENTS_NUM                     (1)
#endif

#if (FP_FMDN_RING_COMPONENTS_NUM > 3)
#error "The maximum value for FP_FMDN_RING_COMPONENTS_NUM is 3."
#endif

#if (FP_FMDN_RING_COMPONENTS_NUM == 0) && \
    (FP_FMDN_ACCESSORY_CAPABILITIES & FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND)
#error "Play sound capability is set, but there is no sound maker."
#endif

/**
 * \brief FMDN ringing capabilities
 *
 * - FP_FMDN_RINGING_VOLUME_NOT_AVAILABLE
 * - FP_FMDN_RINGING_VOLUME_AVAILABLE
 */
#ifndef FP_FMDN_RINGING_CAPABILITIES
#define FP_FMDN_RINGING_CAPABILITIES                    (FP_FMDN_RINGING_VOLUME_NOT_AVAILABLE)
#endif

/**
 * \brief User consent timeout for reading the accessory identifier and EIK for FMDN in milliseconds
 */
#ifndef FP_FMDN_USER_CONSENT_TIMEOUT_MS
#define FP_FMDN_USER_CONSENT_TIMEOUT_MS                 (5 * 60000)
#endif

/**
 * \brief FMDN provisioning initiation duration (after power loss) in milliseconds
 */
#ifndef FP_FMDN_PROVISIONING_INIT_TIMEOUT_MS
#define FP_FMDN_PROVISIONING_INIT_TIMEOUT_MS            (30 * 1000)
#endif

/**
 * \brief FMDN play sound duration when initiated by non-onwer device in milliseconds
 */
#ifndef FP_FMDN_NON_OWNER_PLAY_SOUND_TIMEOUT_MS
#define FP_FMDN_NON_OWNER_PLAY_SOUND_TIMEOUT_MS         (12 * 1000)
#endif

#if (FP_FMDN_NON_OWNER_PLAY_SOUND_TIMEOUT_MS < (5 * 1000))
#error "Sound maker for non-owner must play sound for a minimum duration of 5 seconds."
#endif

/**
 * \brief FMDN play sound duration when motion is detected in separated state in milliseconds
 */
#ifndef FP_FMDN_MOTION_DETECT_PLAY_SOUND_TIMEOUT_MS
#define FP_FMDN_MOTION_DETECT_PLAY_SOUND_TIMEOUT_MS     (1 * 1000)
#endif

/**
 * \brief Verbose logging for FMDN
 */
#ifndef FP_FMDN_LOG_ENABLE
#define FP_FMDN_LOG_ENABLE                              (0)
#endif

#if (FP_FMDN_LOG_ENABLE == 1)
#define FP_FMDN_LOG_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define FP_FMDN_LOG_PRINTF(fmt, ...)
#endif /* FP_FMDN_LOG_ENABLE */

#endif /* FP_FMDN */

#endif /* FP_DEFAULTS_H_ */
