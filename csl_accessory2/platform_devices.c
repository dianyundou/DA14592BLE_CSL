/**
 ****************************************************************************************
 *
 * @file platform_devices.c
 *
 * @brief Configuration of devices connected to board
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#include "ad_uart.h"
#include "ad_gpadc.h"
#include "platform_devices.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (dg_configUART_ADAPTER == 1)
/*
 * Console UART configuration
 */

/* Console UART I/O configuration */
static const ad_uart_io_conf_t io = {
        /* Rx CONSOLE_UART_RX */
        {
                .port = CONSOLE_UART_RX_PORT,
                .pin = CONSOLE_UART_RX_PIN,
                /* On */
                {
                        .mode = HW_GPIO_MODE_INPUT,
                        .function = CONSOLE_UART_RX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = HW_GPIO_MODE_INPUT,
                        .function = CONSOLE_UART_RX_FUNC,
                        .high = true,
                }

        },
        /* Tx CONSOLE_UART_TX */
        {
                .port = CONSOLE_UART_TX_PORT,
                .pin = CONSOLE_UART_TX_PIN,
                /* On */
                {
                        .mode = HW_GPIO_MODE_OUTPUT,
                        .function = CONSOLE_UART_TX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = HW_GPIO_MODE_OUTPUT,
                        .function = CONSOLE_UART_TX_FUNC,
                        .high = true,
                },
        },
        /* RTSN */
        {
                .port = HW_GPIO_PORT_MAX, /* Don't care */
        },
        /* CTSN */
        {
                .port = HW_GPIO_PORT_MAX, /* Don't care */
        },

};

/* Console UART driver configuration */
static const ad_uart_driver_conf_t uart_driver = {
        {
                .baud_rate = HW_UART_BAUDRATE_115200,
                .data = HW_UART_DATABITS_8,
                .parity = HW_UART_PARITY_NONE,
                .stop = HW_UART_STOPBITS_1,
                .auto_flow_control = 0,
                .use_fifo = 1,
                .tx_dma_channel = HW_DMA_CHANNEL_3,
                .rx_dma_channel = HW_DMA_CHANNEL_2,
                .use_dma = 1,
                .tx_fifo_tr_lvl = 0,
                .rx_fifo_tr_lvl = 0,
        }
};

/* Console UART controller configuration */
static const ad_uart_controller_conf_t dev_CONSOLE = {
        .id = CONSOLE_UART,
        .io = &io,
        .drv = &uart_driver,
};

/* Console device */
const uart_device CONSOLE = &dev_CONSOLE;

#endif /* dg_configUART_ADAPTER */

#if (dg_configGPADC_ADAPTER == 1)
/*
 * Battery level GPADC configuration
 */

/* Battery level GPADC driver configuration */
static const ad_gpadc_driver_conf_t battery_level_driver = {
        .input_mode             = HW_GPADC_INPUT_MODE_SINGLE_ENDED,
        .positive               = HW_GPADC_INP_VBAT,
        .oversampling           = HW_GPADC_OVERSAMPLING_64_SAMPLES,
        .sample_time            = 4, /* Safe value to use with high oversampling values */
        .input_attenuator       = HW_GPADC_INPUT_VOLTAGE_UP_TO_3V6,
        .chopping               = false,
        .negative               = 0,
        .continuous             = false,
        .interval               = 0,
};

/* Battery level GPADC controller configuration */
static const ad_gpadc_controller_conf_t battery_level_conf = {
        HW_GPADC_1,
        NULL,
        &battery_level_driver
};

/* Battery level monitoring */
const gpadc_device BATTERY_LEVEL = &battery_level_conf;

#endif /* dg_configGPADC_ADAPTER */

#ifdef __cplusplus
}
#endif
