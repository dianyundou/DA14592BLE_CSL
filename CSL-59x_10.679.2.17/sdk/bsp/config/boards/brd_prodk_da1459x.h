/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup PLA_BSP_CFG_BOARDS
 * \{
 */

 /**
 ****************************************************************************************
 *
 * @file brd_prodk_da1459x.h
 *
 * @brief Board Support Package. DA1459x Board I/O configuration.
 *
 * Copyright (C) 2020 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef BRD_PRODK_DA1459X_H
#define BRD_PRODK_DA1459X_H

/* Serial port configuration section */
#define SER1_UART       (HW_UART2)

#define SER1_TX_PORT    (HW_GPIO_PORT_0)
#define SER1_TX_PIN     (HW_GPIO_PIN_13)
#define SER1_TX_MODE    (HW_GPIO_MODE_OUTPUT)
#define SER1_TX_FUNC    (HW_GPIO_FUNC_UART2_TX)

#define SER1_RX_PORT    (HW_GPIO_PORT_0)
#define SER1_RX_PIN     (HW_GPIO_PIN_15)
#define SER1_RX_MODE    (HW_GPIO_MODE_INPUT)
#define SER1_RX_FUNC    (HW_GPIO_FUNC_UART2_RX)

#define SER1_RTS_PORT   (HW_GPIO_PORT_1)
#define SER1_RTS_PIN    (HW_GPIO_PIN_0)
#define SER1_RTS_MODE   (HW_GPIO_MODE_OUTPUT)
#define SER1_RTS_FUNC   (HW_GPIO_FUNC_UART2_RTSN)

#define SER1_CTS_PORT   (HW_GPIO_PORT_0)
#define SER1_CTS_PIN    (HW_GPIO_PIN_11)
#define SER1_CTS_MODE   (HW_GPIO_MODE_INPUT)
#define SER1_CTS_FUNC   (HW_GPIO_FUNC_UART2_CTSN)

/* LED configuration section */
#define LED1_PORT       (HW_GPIO_PORT_1)
#define LED1_PIN        (HW_GPIO_PIN_1)
#define LED1_MODE       (HW_GPIO_MODE_OUTPUT)
#define LED1_FUNC       (HW_GPIO_FUNC_GPIO)

/* KEY configuration section */
#define KEY1_PORT       (HW_GPIO_PORT_0)
#define KEY1_PIN        (HW_GPIO_PIN_10)
#define KEY1_MODE       (HW_GPIO_MODE_INPUT_PULLUP)
#define KEY1_FUNC       (HW_GPIO_FUNC_GPIO)

#endif /* BRD_PRODK_DA1459X_H */
/**
\}
\}
*/
