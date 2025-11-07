/**
 ****************************************************************************************
 *
 * @file sys_platform_devices_internal.h
 *
 * @brief Configuration of devices connected to board-Should be hidden from documentation
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SYS_PLATFORM_DEVICES_INTERNAL_H_
#define SYS_PLATFORM_DEVICES_INTERNAL_H_


#ifdef __cplusplus
extern "C" {
#endif

#if (dg_configGPADC_ADAPTER == 1)
#include "ad_gpadc.h"
/*
 * Define sources connected to GPADC
 */

extern const ad_gpadc_controller_conf_t TEMP_SENSOR_INTERNAL;

#endif /* dg_configGPADC_ADAPTER */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1))
#include "ad_uart.h"

extern const ad_uart_io_conf_t sys_platform_console_io_conf;
extern const ad_uart_driver_conf_t sys_platform_console_uart_driver_conf;
extern const ad_uart_controller_conf_t sys_platform_console_controller_conf;

#endif /* ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1) */

#if ((dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1)
#include "ad_uart.h"

extern const ad_uart_io_conf_t sys_platform_dgtl_io_conf;
extern const ad_uart_driver_conf_t sys_platform_dgtl_uart_driver_conf;
extern const ad_uart_controller_conf_t sys_platform_dgtl_controller_conf;

#endif /* (dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1 */

#ifdef __cplusplus
}
#endif

#endif /* SYS_PLATFORM_DEVICES_INTERNAL_H_ */
