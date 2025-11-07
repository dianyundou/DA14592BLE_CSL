/**
 ****************************************************************************************
 *
 * @file platform_devices.h
 *
 * @brief Configuration of devices connected to board
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

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_

#include "hw_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * User button configuration section (used for entering pairing mode, performing factory reset,
 * controlling (Apple FMN) ringing or user consent mode)
 */

/* User button on motherboard */
#define USER_BUTTON_PORT                ( KEY1_PORT )
#define USER_BUTTON_PIN                 ( KEY1_PIN )
#define USER_BUTTON_MODE                ( KEY1_MODE )

#if (dg_configUSE_HW_QSPI != 1)
/* User button for both motherboard and daughterboard */
#define USER_BUTTON2_PORT               ( HW_GPIO_PORT_0 )
#define USER_BUTTON2_PIN                ( HW_GPIO_PIN_3 )
#define USER_BUTTON2_MODE               ( HW_GPIO_MODE_INPUT_PULLUP )
#endif

#if (dg_configUART_ADAPTER == 1)
/* UART device handle */
typedef const void *uart_device;

/*
 * Console UART configuration
 */
#define CONSOLE_UART_RX_PORT            SER1_RX_PORT
#define CONSOLE_UART_RX_PIN             SER1_RX_PIN
#define CONSOLE_UART_TX_PORT            SER1_TX_PORT
#define CONSOLE_UART_TX_PIN             SER1_TX_PIN

#define CONSOLE_UART                    SER1_UART
#define CONSOLE_UART_RX_FUNC            SER1_RX_FUNC
#define CONSOLE_UART_TX_FUNC            SER1_TX_FUNC

/*
 * Console device
 */
extern const uart_device CONSOLE;
#endif /* dg_configUART_ADAPTER */

#if (dg_configGPADC_ADAPTER == 1)
/* GPADC device handle */
typedef const void *gpadc_device;

/*
 * Battery level monitoring
 */
extern const gpadc_device BATTERY_LEVEL;
#endif /* dg_configGPADC_ADAPTER */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_DEVICES_H_ */
