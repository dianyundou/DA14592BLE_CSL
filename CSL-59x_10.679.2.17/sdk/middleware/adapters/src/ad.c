/**
 ****************************************************************************************
 *
 * @file  ad.c
 *
 * @brief Adapters shared functions
 *
 * Copyright (C) 2021-2023 Renesas Electronics Corporation and/or its affiliates.
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

/* Not all the checked adapters are available for every device */
#if (dg_configI2C_ADAPTER == 1) || (dg_configSPI_ADAPTER == 1) || (dg_configGPADC_ADAPTER == 1) \
                           || (dg_configISO7816_ADAPTER == 1) || (dg_configLCDC_ADAPTER == 1 ) \
                           || (dg_configSDADC_ADAPTER == 1) || (dg_configUART_ADAPTER == 1) \
                           || (dg_configI3C_ADAPTER == 1)

#include <stdint.h>
#include "ad.h"
#include "osal.h"

AD_IO_ERROR ad_io_configure(const ad_io_conf_t *io, uint8_t size, AD_IO_CONF_STATE state)
{
        for (int i = 0; i < size; i++) {
                if (!AD_IO_PIN_PORT_VALID(io[i].port, io[i].pin)) {
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_PIN;
                }

                switch (state) {
                case AD_IO_CONF_ON:
                        hw_gpio_configure_pin(io[i].port, io[i].pin, io[i].on.mode,
                                                        io[i].on.function, io[i].on.high);
                        break;
                case AD_IO_CONF_OFF:
                        hw_gpio_configure_pin(io[i].port, io[i].pin, io[i].off.mode,
                                                        io[i].off.function, io[i].off.high);
                        break;
                default:
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_CFG;
                }
        }

        return AD_IO_ERROR_NONE;
}

AD_IO_ERROR ad_io_set_pad_latch(const ad_io_conf_t *io, uint8_t size, AD_IO_PAD_LATCHES_OP operation)
{
        if (!io || (0 == size) || (size > HW_GPIO_PIN_MAX * HW_GPIO_PORT_MAX)) {
                OS_ASSERT(0);
                return AD_IO_ERROR_INVALID_CFG;
        }

        for (int i = 0; i < size; i++) {
                if (!AD_IO_PIN_PORT_VALID(io[i].port, io[i].pin)) {
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_PIN;
                }

                switch (operation) {
                case AD_IO_PAD_LATCHES_OP_ENABLE:
                        hw_gpio_pad_latch_enable(io[i].port, io[i].pin);
                        break;
                case AD_IO_PAD_LATCHES_OP_DISABLE:
                        hw_gpio_pad_latch_disable(io[i].port, io[i].pin);
                        break;
                case AD_IO_PAD_LATCHES_OP_TOGGLE:
                        hw_gpio_pad_latch_enable(io[i].port, io[i].pin);
                        hw_gpio_pad_latch_disable(io[i].port, io[i].pin);
                        break;
                default:
                        OS_ASSERT(0);
                        return AD_IO_ERROR_INVALID_CFG;
                }
        }

        return AD_IO_ERROR_NONE;
}


#endif /* dg_config*_ADAPTER */
