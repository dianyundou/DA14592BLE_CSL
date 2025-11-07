/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup ADAPTER Adapter
 *
 * \brief Common definitions for I/O adapters
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file  ad.h
 *
 * @brief Adapters shared definitions
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

#ifndef AD_H_
#define AD_H_

#include "hw_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AD_IO_PIN_PORT_VALID(_port, _pin)       ((_pin >= HW_GPIO_PIN_0) &&\
                                                 (_port >= HW_GPIO_PORT_0) &&\
                                                 (_pin < HW_GPIO_PIN_MAX) &&\
                                                 (_port < HW_GPIO_PORT_MAX))

/*
 * Data types definitions section
 */

/**
 * \brief Adapters IO configuration state
 */
typedef enum {
        AD_IO_CONF_OFF = 0,             /**< Off configuration */
        AD_IO_CONF_ON  = 1,             /**< On configuration */
} AD_IO_CONF_STATE;

/**
 * \brief Pad latch operation
 */
typedef enum {
        AD_IO_PAD_LATCHES_OP_DISABLE    = 0,    /**< Pad latch disabled.
                                                     The pad retains its current state (e.g.
                                                     during sleep). */
        AD_IO_PAD_LATCHES_OP_ENABLE     = 1,    /**< Pad latch enabled.
                                                     The pad can change state, either by
                                                     external drive (input) or by us (output). */
        AD_IO_PAD_LATCHES_OP_TOGGLE     = 2,    /**< Pad latch enabled and then disabled */
} AD_IO_PAD_LATCHES_OP;

/**
 * \brief ad_io_configure return value
 */
typedef enum {
        AD_IO_ERROR_NONE         =  0,          /**< Configuration set */
        AD_IO_ERROR_INVALID_PIN  = -1,          /**< Invalid pin passed */
        AD_IO_ERROR_INVALID_CFG  = -2,          /**< Invalid IO configuration passed */
} AD_IO_ERROR;

/**
 * \brief Adapters pin configuration
 */
typedef struct {
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC function;
        bool         high;              /**< IO level when pin is configured as gpio output */
} ad_pin_conf_t;

/**
 * \brief Adapters IO configuration
 */
typedef struct {
        HW_GPIO_PORT port;
        HW_GPIO_PIN  pin;
        ad_pin_conf_t on;
        ad_pin_conf_t off;
} ad_io_conf_t;

/**
 * \brief Apply a list of IO configurations
 *
 * This function applies the on or off configuration of a list of IO pins. It configures :
 * * IO pin function
 * * Pad voltage level
 *
 * \param [in] io list of configurations of gpio pins
 * \param [in] size number of pins to configure
 * \param [in] state configuration to be applied (AD_IO_CONF_OFF/AD_IO_CONF_ON)
 *
 * \return AD_IO_ERROR
 *
 */
AD_IO_ERROR ad_io_configure(const ad_io_conf_t *io, uint8_t size, AD_IO_CONF_STATE state);

/**
 * \brief Apply a list of io pad latch operations
 *
 * \param [in] io list of configurations of io pins
 * \param [in] size number of pins to configure
 * \param [in] operation latch state
 *
 * \return AD_IO_ERROR
 *
 */
AD_IO_ERROR ad_io_set_pad_latch(const ad_io_conf_t *io, uint8_t size, AD_IO_PAD_LATCHES_OP operation);


#ifdef __cplusplus
}
#endif

#endif /* AD_H_ */

/**
 * \}
 * \}
 */
