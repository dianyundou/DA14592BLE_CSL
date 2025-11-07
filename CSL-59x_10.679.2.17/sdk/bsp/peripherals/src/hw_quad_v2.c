/**
 *****************************************************************************************
 *
 * @file hw_quad_v2.c
 *
 * @brief Implementation of the Quad Decoder Low Level Driver.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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
 *****************************************************************************************
 */

#if dg_configUSE_HW_QUAD

#include <stdio.h>
#include <limits.h>
#include "hw_quad.h"

static __RETAINED hw_quad_handler_cb hw_quad_cb;

static __RETAINED volatile hw_quad_values_t hw_quad_values;
static __RETAINED void *user_data;

static __RETAINED HW_QUAD_GPIO_INPUT hw_x_axis_ports;
static __RETAINED HW_QUAD_GPIO_INPUT hw_y_axis_ports;
static __RETAINED HW_QUAD_GPIO_INPUT hw_z_axis_ports;

/**
 * \Support functions intended for internal LLD use.
 * **********************************************************
 */
static void hw_quad_get_new_x_raw_and_delta(void)
{
        int16_t new_x_raw =  hw_quad_get_xcnt_reg();
        hw_quad_values.hw_quad_xcnt_delta = new_x_raw - hw_quad_values.hw_quad_xcnt_raw;
        hw_quad_values.hw_quad_xcnt_raw = new_x_raw;
}

static void hw_quad_get_new_y_raw_and_delta(void)
{
        int16_t new_y_raw =  hw_quad_get_ycnt_reg();
        hw_quad_values.hw_quad_ycnt_delta = new_y_raw - hw_quad_values.hw_quad_ycnt_raw;
        hw_quad_values.hw_quad_ycnt_raw = new_y_raw;
}

static void hw_quad_get_new_z_raw_and_delta(void)
{
        int16_t new_z_raw =  hw_quad_get_zcnt_reg();
        hw_quad_values.hw_quad_zcnt_delta = new_z_raw - hw_quad_values.hw_quad_zcnt_raw;
        hw_quad_values.hw_quad_zcnt_raw = new_z_raw;
}

static void hw_quad_get_new_event_cnt_raw(void)
{
        hw_quad_values.hw_quad_event_cnt_raw = hw_quad_get_event_cnt_reg();
}

static void hw_quad_reset_quad_values(void)
{
        hw_quad_values.hw_quad_event_cnt_raw = 0x0;
        hw_quad_values.hw_quad_xcnt_raw = 0x0;
        hw_quad_values.hw_quad_xcnt_delta = 0x0;
        hw_quad_values.hw_quad_ycnt_raw = 0x0;
        hw_quad_values.hw_quad_ycnt_delta = 0x0;
        hw_quad_values.hw_quad_zcnt_raw = 0x0;
        hw_quad_values.hw_quad_zcnt_delta = 0x0;
}

/**
  * **********************************************************
 */

void hw_quad_interrupt_enable_and_register(hw_quad_handler_cb cb, uint8_t thres_num, void *user_data_p)
{
        ASSERT_WARNING(cb);
        uint16_t tmp_ctrl_reg = QUADEC->QDEC_CTRL_REG;
        REG_SET_FIELD(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_THRES, tmp_ctrl_reg, thres_num);
        REG_SET_FIELD(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_STATUS, tmp_ctrl_reg, HW_QUAD_REG_CTRL_IRQ_STATUS_ON_CLR);
        REG_SET_FIELD(QUADEC, QDEC_CTRL_REG, QDEC_EVENT_CNT_CLR, tmp_ctrl_reg, HW_QUAD_REG_CTRL_EVENT_CNT_CLR_ON);
        REG_SET_FIELD(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_ENABLE, tmp_ctrl_reg, HW_QUAD_REG_CTRL_IRQ_EN_ON);
        QUADEC->QDEC_CTRL_REG = tmp_ctrl_reg;
        user_data = user_data_p;
        hw_quad_cb = cb;
        NVIC_ClearPendingIRQ(QUADDEC_IRQn);
        NVIC_EnableIRQ(QUADDEC_IRQn);
}

void hw_quad_interrupt_disable_and_unregister(void)
{
        REG_SETF(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_ENABLE, HW_QUAD_REG_CTRL_IRQ_EN_OFF);
        NVIC_DisableIRQ(QUADDEC_IRQn);
        NVIC_ClearPendingIRQ(QUADDEC_IRQn);
        hw_quad_cb = NULL;
        user_data = NULL;
}

void hw_quad_start(void)
{
        uint16_t temp_reg = QUADEC->QDEC_CTRL2_REG;
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHX_PORT_SEL, temp_reg, hw_x_axis_ports);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHY_PORT_SEL, temp_reg, hw_y_axis_ports);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHZ_PORT_SEL, temp_reg, hw_z_axis_ports);
        QUADEC->QDEC_CTRL2_REG = temp_reg;
}

void hw_quad_stop(void)
{
        uint16_t temp_reg = QUADEC->QDEC_CTRL2_REG;
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHX_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHY_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHZ_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        QUADEC->QDEC_CTRL2_REG = temp_reg;
}

void hw_quad_disable(void)
{
        hw_quad_stop();
        hw_quad_reset_quad_values();
        hw_quad_interrupt_disable_and_unregister();
        hw_quad_qdec_clk_en(HW_QUAD_REG_CLK_OFF);
}

void hw_quad_init(const hw_quad_config_t *init_data)
{
        hw_quad_qdec_clk_en(HW_QUAD_REG_CLK_ON);
        uint32_t temp_reg = 0x0;
        REG_SET_FIELD(QUADEC, QDEC_CLOCKDIV_REG, QDEC_CLOCKDIV, temp_reg, init_data->clk_div);
        REG_SET_FIELD(QUADEC, QDEC_CLOCKDIV_REG, QDEC_PRESCALER_EN, temp_reg, init_data->clk_prescaler);
        QUADEC->QDEC_CLOCKDIV_REG = temp_reg;
        temp_reg = 0x0;
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHX_EVENT_MODE, temp_reg, init_data->chx_event_mode);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHY_EVENT_MODE, temp_reg, init_data->chy_event_mode);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHZ_EVENT_MODE, temp_reg, init_data->chz_event_mode);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHX_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHY_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        REG_SET_FIELD(QUADEC, QDEC_CTRL2_REG, QDEC_CHZ_PORT_SEL, temp_reg, HW_QUAD_REG_INPUT_NONE);
        QUADEC->QDEC_CTRL2_REG = temp_reg;
        hw_x_axis_ports = init_data->chx_port_sel;
        hw_y_axis_ports = init_data->chy_port_sel;
        hw_z_axis_ports = init_data->chz_port_sel;
}

void QUADDEC_Handler (void)
{
        hw_quad_clr_irq_status();
        hw_quad_get_new_x_raw_and_delta();
        hw_quad_get_new_y_raw_and_delta();
        hw_quad_get_new_z_raw_and_delta();
        hw_quad_get_new_event_cnt_raw();
        if (hw_quad_cb) {

                hw_quad_cb(&hw_quad_values, user_data);
        }
        hw_quad_clr_event_cnt();
}

#endif /* dg_configUSE_HW_QUAD */

