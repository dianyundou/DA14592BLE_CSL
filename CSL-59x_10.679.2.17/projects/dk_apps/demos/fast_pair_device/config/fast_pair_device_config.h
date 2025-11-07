/**
 ****************************************************************************************
 *
 * @file fast_pair_device_config.h
 *
 * @brief Google Fast Pair device application configuration
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

#ifndef FAST_PAIR_DEVICE_CONFIG_H_
#define FAST_PAIR_DEVICE_CONFIG_H_

#include "fast_pair_config.h"

/* Default device name used for GAP service and in scan response */
#if (FP_FMDN == 1)
#define DEVICE_DEFAULT_NAME                     "Renesas FMDN Accessory"
#else
#define DEVICE_DEFAULT_NAME                     "Renesas Fast Pair Device"
#endif

/* TX power in connection */
#if (FP_FMDN == 1)
#define TX_POWER_CONN                           ( FP_FMDN_ADV_TX_POWER )
#else
#define TX_POWER_CONN                           ( FP_ADV_TX_POWER )
#endif

/* Time Fast Pair Provider stays in pairing mode */
#define PAIRING_MODE_TIMEOUT_MS                 ( 60000 )  /* 1 minute */

/* Enable support of multiple advertising events */
#define ADV_CONTROL_MULT_EVENTS_ENABLE          ( 0 )

/*
 * User button press timeout intervals
 */
/* Button long press timeout used for starting pairing mode */
#define LONG_PRESS_TIMEOUT_MS                   ( 4 * 1000 )  /* 4 seconds */
#if (FP_FMDN == 1)
/* Button short press timeout used for stopping ringing and enabling user consent */
#define SHORT_PRESS_TIMEOUT_MS                  ( 250 )
/* Button double press timeout used for indicating motion detection */
#define DOUBLE_PRESS_TIMEOUT_MS                 ( 800 )
#endif /* FP_FMDN */
/* Button very long press timeout used for factory reset */
#define VERY_LONG_PRESS_TIMEOUT_MS              ( 8 * 1000 )  /* 8 seconds */
#if (FP_FMDN == 1)
/* Button press timeout used for advertise stop */
#define ADV_STOP_PRESS_TIMEOUT_MS               ( 2000 )
#endif /* FP_FMDN */

#if (FP_BATTERIES_COUNT != 0)
/* Interval for periodically checking battery level */
#define BATTERY_MONITOR_INTERVAL_MS             ( 5 * 60000 )  /* 5 minutes */
/* Battery voltage level (in mV) for the battery to be considered fully charged */
#define BATTERY_MONITOR_FULL_LEVEL_MV           ( 3000 )
/* Battery voltage level (in mV) for the battery to be considered empty */
#define BATTERY_MONITOR_EMPTY_LEVEL_MV          ( 1700 )

#if (FP_FMDN == 1)
/* Battery level (range 0 - 100) at which the device enters low power mode */
#define LOW_POWER_MODE_BATTERY_LEVEL            ( 15 )  /* % */
/* Interval at which advertising is periodically enabled in low power mode */
#define ADV_CONTROL_LOW_POWER_INTERVAL_MS       ( 10 * 60000 )  /* 10 minutes */
/* Period during which advertising is enabled in low power mode */
#define ADV_CONTROL_LOW_POWER_ADV_TIMEOUT_MS    ( 10 * 1000 )  /* 10 seconds */
#endif /* FP_FMDN */
#endif /* FP_BATTERIES_COUNT */

#if (FP_FMDN == 1)
/* Interval at which beacon time is updated in NVM storage */
#define BEACON_TIME_STORAGE_INTERVAL_MS         ( 12 * 60 * 60000 )  /* 12 hours */
#endif /* FP_FMDN */

/* Use console */
#define USE_CONSOLE                             ( 1 )

#ifndef CONFIG_RETARGET
#undef USE_CONSOLE
#endif

/* Enable logging for Fast Pair application task */
#ifdef CONFIG_RETARGET
#define FP_TASK_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define FP_TASK_PRINTF(fmt, ...)
#endif /* CONFIG_RETARGET */

#endif /* FAST_PAIR_DEVICE_CONFIG_H_ */
