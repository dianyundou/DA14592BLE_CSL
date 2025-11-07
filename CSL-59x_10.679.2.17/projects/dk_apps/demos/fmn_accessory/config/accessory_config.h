/**
 ****************************************************************************************
 *
 * @file accessory_config.h
 *
 * @brief Apple Find My Network accessory application configuration
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

#ifndef ACCESSORY_CONFIG_H_
#define ACCESSORY_CONFIG_H_

#include "afmn_config.h"

/* Default accessory name used for GAP service and in scan response */
#define ACCESSORY_DEFAULT_NAME                  "Renesas FMN Accessory"

/* Enable support of multiple advertising events */
#define ADV_CONTROL_MULT_EVENTS_ENABLE          ( 0 )

/*
 * User button press timeout intervals
 */
/* Button short press timeout used for enabling serial number lookup */
#define SHORT_PRESS_TIMEOUT_MS                  ( 250 )
/* Button long press timeout used for starting pair mode */
#define LONG_PRESS_TIMEOUT_MS                   ( 4 * 1000 )  /* 4 seconds */
/* Button very long press timeout used for factory reset */
#define VERY_LONG_PRESS_TIMEOUT_MS              ( 8 * 1000 )  /* 8 seconds */
/* Button double press timeout used for triggering motion detected event*/
#define DOUBLE_PRESS_TIMEOUT_MS                 ( 800 )

/* Interval for keeping enabled serial number lookup */
#define SERIAL_NUMBER_LOOKUP_TIMEOUT_MS         ( 5 * 60 * 1000 )  /* 5 minutes */

/* Interval for periodically checking battery level */
#define BATTERY_MONITOR_INTERVAL_MS             ( 5 * 60 * 1000 )  /* 5 minutes */
/* Battery voltage level (in mV) for the battery to be considered fully charged */
#define BATTERY_MONITOR_FULL_LEVEL_MV           ( 3000 )
/* Battery voltage level (in mV) for the battery to be considered empty */
#define BATTERY_MONITOR_EMPTY_LEVEL_MV          ( 1700 )

/* TX power in connection */
#define TX_POWER_CONN                           ( GAP_TX_POWER_4_5_dBm )

/* LED blinking rate */
#define LED_CONTROL_BLINK_RATE_MS               ( 200 )

/* Use console */
#define USE_CONSOLE                             ( 1 )

#ifndef CONFIG_RETARGET
#undef USE_CONSOLE
#endif

/* Enable logging for Apple FindMy Network accessory task */
#ifdef CONFIG_RETARGET
#define FMNA_TASK_PRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define FMNA_TASK_PRINTF(fmt, ...)
#endif /* CONFIG_RETARGET */

#endif /* ACCESSORY_CONFIG_H_ */
