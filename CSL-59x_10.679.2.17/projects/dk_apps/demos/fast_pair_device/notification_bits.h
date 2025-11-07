/**
 ****************************************************************************************
 *
 * @file notification_bits.h
 *
 * @brief Google Fast Pair application task notification bits
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

#ifndef NOTIFICATION_BITS_H_
#define NOTIFICATION_BITS_H_

#include "fast_pair_config.h"

/*
 * Notification bits reservation
 *
 * Bit #0 is always assigned to BLE event queue notification.
 */
#define PAIRING_MODE_TMO_NOTIF          (1 << 1)
#define LED_CONTROL_NOTIF               (1 << 2)
#if (FP_BATTERIES_COUNT != 0)
#define BATTERY_MONITOR_NOTIF           (1 << 3)
#endif
#define ADV_CONTROL_NOTIF               (1 << 4)
#define BUTTON_PRESS_NOTIF              (1 << 5)
#define UPDATE_CONN_PARAM_NOTIF         (1 << 6)
#define CONSOLE_NOTIF                   (1 << 7)
#define LONG_PRESS_TMO_NOTIF            (1 << 8)
#if (FP_FMDN == 1)
#define SHORT_PRESS_TMO_NOTIF           (1 << 9)
#define DOUBLE_PRESS_TMO_NOTIF          (1 << 10)
#endif
#define FACTORY_RESET_TMO_NOTIF         (1 << 11)
#define GOOGLE_FAST_PAIR_NOTIF          (1 << 12)
#define ADV_STOP_PRESS_TMO_NOTIF        (1 << 13)
#if (FP_FMDN == 1)
#define BEACON_TIME_TMO_NOTIF           (1 << 14)
#endif
#endif /* NOTIFICATION_BITS_H_ */
