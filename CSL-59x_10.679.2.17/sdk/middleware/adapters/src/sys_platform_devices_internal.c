/**
 ****************************************************************************************
 *
 * @file sys_platform_devices_internal.c
 *
 * @brief Configuration of devices connected to board
 *
 * Copyright (C) 2018-2023 Renesas Electronics Corporation and/or its affiliates.
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

#if (dg_configGPADC_ADAPTER == 1)

#include "sys_platform_devices_internal.h"

/*
 * Define sources connected to GPADC
 */
const ad_gpadc_driver_conf_t temp_sensor_internal = {
        .input_mode             = HW_GPADC_INPUT_MODE_SINGLE_ENDED,
        .positive               = HW_GPADC_INP_DIE_TEMP,
        .oversampling           = HW_GPADC_OVERSAMPLING_16_SAMPLES,
        .sample_time            = HW_GPADC_DIE_TEMP_SMPL_TIME, /* Safe value to use with high oversampling values */
        .input_attenuator       = HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9,
        .chopping               = true,
        .negative               = 0, /* SW_Reset value. Anyway discarded in Single-ended mode */
        .continuous             = false,
        .interval               = 0, /* SW_Reset value. Anyway discarded when continuous mode is off */
};


const ad_gpadc_controller_conf_t TEMP_SENSOR_INTERNAL = {
        HW_GPADC_1,
        NULL,
        &temp_sensor_internal
};



#endif /* dg_configGPADC_ADAPTER */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1))

#include "ad_uart.h"

const ad_uart_io_conf_t sys_platform_console_io_conf = {
        /* Rx UART2 */
        .rx = {
                .port = SER1_RX_PORT,
                .pin = SER1_RX_PIN,
                /* On */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
        },
        /* Tx UART2 */
        .tx = {
                .port = SER1_TX_PORT,
                .pin = SER1_TX_PIN,
                /* On */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
        },
        /* RTSN */
        .rtsn = {
                .port = SER1_RTS_PORT,
                .pin = SER1_RTS_PIN,
                /* On */
                {
                        .mode = SER1_RTS_MODE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RTS_MODE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
        },
        /* CTSN */
        .ctsn = {
                .port = SER1_CTS_PORT,
                .pin = SER1_CTS_PIN,
                /* On */
                {
                        .mode = SER1_CTS_MODE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_CTS_MODE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
        },
};

const ad_uart_driver_conf_t sys_platform_console_uart_driver_conf = {
        {

                .baud_rate = HW_UART_BAUDRATE_115200,
                .data = HW_UART_DATABITS_8,
                .parity = HW_UART_PARITY_NONE,
                .stop = HW_UART_STOPBITS_1,
                .auto_flow_control = 1,
                .use_fifo = 1,
                .use_dma = 1,
                .tx_dma_channel = HW_DMA_CHANNEL_3,
                .rx_dma_channel = HW_DMA_CHANNEL_2,
                .tx_fifo_tr_lvl = 0,
                .rx_fifo_tr_lvl = 0,
        }
};

const ad_uart_controller_conf_t sys_platform_console_controller_conf = {
        .id = SER1_UART,
        .io = &sys_platform_console_io_conf,
        .drv = &sys_platform_console_uart_driver_conf,
};
#endif /* ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1)) */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_DGTL == 1))
#include "ad_uart.h"
#include "dgtl_config.h"

const ad_uart_io_conf_t sys_platform_dgtl_io_conf = {
        /* Rx UART2 */
        .rx = {
                .port = SER1_RX_PORT,
                .pin = SER1_RX_PIN,
                /* On */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_RX_MODE,
                        .function = SER1_RX_FUNC,
                        .high = true,
                },
        },
        /* Tx UART2 */
        .tx = {
                .port = SER1_TX_PORT,
                .pin = SER1_TX_PIN,
                /* On */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = SER1_TX_MODE,
                        .function = SER1_TX_FUNC,
                        .high = true,
                },
        },
        /* RTSN */
        .rtsn = {
                .port = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_PORT : HW_GPIO_PORT_NONE,
                .pin = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_PIN : HW_GPIO_PIN_NONE,
                /* On */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_RTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_RTS_FUNC,
                        .high = true,
                },
        },
        /* CTSN */
        .ctsn = {
                .port = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_PORT : HW_GPIO_PORT_NONE,
                .pin = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_PIN : HW_GPIO_PIN_NONE,
                /* On */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
                /* Off */
                {
                        .mode = DGTL_AUTO_FLOW_CONTROL ? SER1_CTS_MODE : HW_GPIO_MODE_NONE,
                        .function = SER1_CTS_FUNC,
                        .high = true,
                },
        },
};

const ad_uart_driver_conf_t sys_platform_dgtl_uart_driver_conf = {
        {

                .baud_rate = HW_UART_BAUDRATE_115200,
                .data = HW_UART_DATABITS_8,
                .parity = HW_UART_PARITY_NONE,
                .stop = HW_UART_STOPBITS_1,
                .auto_flow_control = DGTL_AUTO_FLOW_CONTROL,
                .use_fifo = 1,
#if dg_configUSE_HW_DMA
                .use_dma = 1,
                .tx_dma_channel = HW_DMA_CHANNEL_3,
                .rx_dma_channel = HW_DMA_CHANNEL_2,
#endif
                .tx_fifo_tr_lvl = 0,
                .rx_fifo_tr_lvl = 0,
        }
};

const ad_uart_controller_conf_t sys_platform_dgtl_controller_conf = {
        .id = SER1_UART,
        .io = &sys_platform_dgtl_io_conf,
        .drv = &sys_platform_dgtl_uart_driver_conf,
};
#endif /* (dg_configUART_ADAPTER == 1) && (dg_configUSE_DGTL == 1) */

