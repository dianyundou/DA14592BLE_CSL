/**
 ****************************************************************************************
 *
 * @file led_control.h
 *
 * @brief LED control header file
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

#ifndef LED_CONTROL_H_
#define LED_CONTROL_H_

#include <stdint.h>
#include "osal.h"

/**
 * \brief Value indicating constantly blinking LED
 */
#define LED_CONTROL_BLINK_FOREVER       (0xFFFF)

/**
 * \brief LED control mode
 */
typedef enum {
        LED_CONTROL_MODE_SET_OFF,       /**< Set LED off */
        LED_CONTROL_MODE_SET_ON,        /**< Set LED on */
        LED_CONTROL_MODE_START_BLINK,   /**< Start LED blinking */
        LED_CONTROL_MODE_BLINK          /**< Blink/toggle LED */
} LED_CONTROL_MODE;

/**
 * \brief LED control blink rate (as interval in msec)
 */
typedef enum {
        LED_CONTROL_BLINK_RATE_DEFAULT  = 500,
        LED_CONTROL_BLINK_RATE_LOW      = 1200,
        LED_CONTROL_BLINK_RATE_MEDIUM   = 700,
        LED_CONTROL_BLINK_RATE_HIGH     = 200
} LED_CONTROL_BLINK_RATE;

/**
 * \brief LED control mode structure
 */
typedef struct {
        LED_CONTROL_MODE mode;                  /**< LED control mode */
        LED_CONTROL_BLINK_RATE blink_rate_ms;   /**< LED blink rate defined as interval in ms
                                                     (valid with LED_CONTROL_MODE_START_BLINK mode) */
        uint16_t blink_cnt;                     /**< LED blink counter
                                                     (valid with LED_CONTROL_MODE_START_BLINK mode) */
} led_control_mode_t;

/**
 * \brief Initialize LED
 *
 * \param [in] task_hdl control task handle
 */
void led_control_init(OS_TASK task_hdl);

/**
 * \brief Get LED control mode
 *
 * \return LED control mode
 */
LED_CONTROL_MODE led_control_get_mode(void);

/**
 * \brief Set LED control mode
 *
 * Sets LED control mode. In case of LED_CONTROL_MODE_START_BLINK mode, the times LED blinks and
 * blinking rate are also set.
 * LED_CONTROL_BLINK_FOREVER can be used to indicate constantly blinking LED.
 *
 * \param [in] mode             LED control mode
 * \param [in] blink_rate_ms    LED blinking rate (valid only with LED_CONTROL_MODE_START_BLINK mode)
 * \param [in] blink_cnt        times LED blinks(valid only with LED_CONTROL_MODE_START_BLINK mode)
 */
void led_control_set_mode(LED_CONTROL_MODE mode, LED_CONTROL_BLINK_RATE blink_rate_ms,
        uint16_t blink_cnt);

/**
 * \brief Process LED control task notification
 *
 * Handles LED blinking.
 *
 * \param [in] notif control task notification
 */
void led_control_process_notif(uint32_t notif);

#endif /* LED_CONTROL_H_ */
