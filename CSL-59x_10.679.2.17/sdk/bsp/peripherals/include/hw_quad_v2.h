/**
 * \addtogroup PLA_DRI_PER_OTHER
 * \{
 * \addtogroup HW_QUAD Q-Dec
 * \{
 * \brief Quadrature Decoder
 */

/**
 *****************************************************************************************
 *
 * @file hw_quad_v2.h
 *
 * @brief Definition of API for the Quad Decoder Low Level Driver.
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

#ifndef HW_QUAD_V2_H_
#define HW_QUAD_V2_H_

#if dg_configUSE_HW_QUAD

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/**\enum HW_QUAD_REG_CLK
 *
 * \brief Quadrature Decoder CTRL Clock Enable Status
 *
 * \note The clock is enabled/disabled through the QDEC_CLK_ENABLE single-bit field.
 *
 * \note If the clock is disabled, no interaction with the QUADEC registers is possible.
 */
typedef enum {
        HW_QUAD_REG_CLK_OFF = 0,
        HW_QUAD_REG_CLK_ON = 1
} HW_QUAD_REG_CLK;

/**\enum HW_QUAD_REG_CTRL_IRQ_STATUS
 *
 * \brief Quadrature Decoder CTRL Register IRQ Status
 */
typedef enum {
        HW_QUAD_REG_CTRL_IRQ_STATUS_OFF_NO_CLR = 0,
        HW_QUAD_REG_CTRL_IRQ_STATUS_ON_CLR = 1
} HW_QUAD_REG_CTRL_IRQ_STATUS;

/**\enum HW_QUAD_REG_CTRL_EVENT_CNT_CLR
 *
 * \brief Quadrature Decoder CTRL Register Event Counter Clear
 */
typedef enum {
        HW_QUAD_REG_CTRL_EVENT_CNT_CLR_OFF = 0,
        HW_QUAD_REG_CTRL_EVENT_CNT_CLR_ON = 1
} HW_QUAD_REG_CTRL_EVENT_CNT_CLR;

/**\enum HW_QUAD_REG_CTRL_IRQ_EN
 *
 * \brief Quadrature Decoder CTRL Register IRQ Status
 */
typedef enum {
        HW_QUAD_REG_CTRL_IRQ_EN_OFF = 0,
        HW_QUAD_REG_CTRL_IRQ_EN_ON = 1
} HW_QUAD_REG_CTRL_IRQ_EN;

/**\enum HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE
 *
 * \brief Quadrature Decoder CTRL2 Register Channel X-Y-Z Event Mode
 */
typedef enum {
        HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE_NORMAL_CNT = 0,
        HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE_EDGE_CNT = 1
} HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE;

/**\enum HW_QUAD_GPIO_INPUT
 *
 * \brief Quadrature Decoder CTRL2 Register Channel X-Y-Z Port Selection
 */
typedef enum {
        HW_QUAD_REG_INPUT_NONE          = 0,
        HW_QUAD_REG_INPUT_A_P000_B_P001 = 1,
        HW_QUAD_REG_INPUT_A_P002_B_P003 = 2,
        HW_QUAD_REG_INPUT_A_P004_B_P005 = 3,
        HW_QUAD_REG_INPUT_A_P008_B_P009 = 4,
        HW_QUAD_REG_INPUT_A_P010_B_P011 = 5,
        HW_QUAD_REG_INPUT_A_P100_B_P102 = 6,
        HW_QUAD_REG_INPUT_A_P103_B_P105 = 7,
        HW_QUAD_REG_INPUT_A_P106_B_P107 = 8,
        HW_QUAD_REG_INPUT_A_P108_B_P109 = 9,
        HW_QUAD_REG_INPUT_A_P110_B_P111 = 10,
        HW_QUAD_REG_INPUT_A_P112_B_P101 = 11,
        HW_QUAD_REG_INPUT_A_P104_B_P115 = 12
} HW_QUAD_GPIO_INPUT;

/**\enum HW_QUAD_REG_CLOCKDIV_PRESCALER
 *
 * \brief Quadrature Decoder CLOCKDIV Register Prescaler
 */
typedef enum {
        HW_QUAD_REG_CLOCKDIV_PRESCALER_OFF = 0,
        HW_QUAD_REG_CLOCKDIV_PRESCALER_ON = 1
} HW_QUAD_REG_CLOCKDIV_PRESCALER;

/**
 * \brief Local Quadrature Decoder Counter and Deltas repository definition
 */
typedef struct {
        int16_t hw_quad_xcnt_raw;
        int16_t hw_quad_ycnt_raw;
        int16_t hw_quad_zcnt_raw;
        uint8_t hw_quad_event_cnt_raw;
        int16_t hw_quad_xcnt_delta;
        int16_t hw_quad_ycnt_delta;
        int16_t hw_quad_zcnt_delta;
} hw_quad_values_t;

/**
 * \brief Quadrature Decoder Interrupt Callback
 *
 * \param [in] user_data pointer to additional user data
 */
typedef void (*hw_quad_handler_cb)(const volatile hw_quad_values_t *quad_values, void *user_data);

/**
 * \brief Initialization Struct for use with "hw_quad_init()"
 *
 * \note clk_prescaler -    defines whether the prescaler will be enabled or not.
 * \note clk_div -          defines the number of the input clock cycles minus one, that
 *                              are required to generate one logic clock cycle.
 * \note chz_event_mode -   defines whether QDEC_ZCNT_REG will count events as normal
 *                              Quadrature pulse pairs or act as a rising-and-falling edge counter for
 *                              both axis ports (NOTE: in the latter mode, if both ports change at the
 *                              same time, the counter increases by 1).
 * \note chy_event_mode -   defines whether QDEC_ZYNT_REG will count events as normal
 *                              Quadrature pulse pairs or act as a rising-and-falling edge counter for
 *                              both axis ports (NOTE: in the latter mode, if both ports change at the
 *                              same time, the counter increases by 1).
 * \note chx_event_mode -   defines whether QDEC_XCNT_REG will count events as normal
 *                              Quadrature pulse pairs or act as a rising-and-falling edge counter for
 *                              both axis ports (NOTE: in the latter mode, if both ports change at the
 *                              same time, the counter increases by 1).
 * \note chz_port_sel -     defines the Quadrature Decoder Z-Axis input port pair.
 * \note chy_port_sel -     defines the Quadrature Decoder Y-Axis input port pair.
 * \note chx_port_sel -     defines the Quadrature Decoder Z-Axis input port pair.
 *
 */
typedef struct {
        HW_QUAD_REG_CLOCKDIV_PRESCALER clk_prescaler;
        uint16_t clk_div;
        HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE chz_event_mode;
        HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE chy_event_mode;
        HW_QUAD_REG_CTRL2_CHANNEL_EVENT_MODE chx_event_mode;
        HW_QUAD_GPIO_INPUT chz_port_sel;
        HW_QUAD_GPIO_INPUT chy_port_sel;
        HW_QUAD_GPIO_INPUT chx_port_sel;
} hw_quad_config_t;

/**
 * \brief Get XCNT Register
 *
 * Returns the contents of the Quadrature Decoder XCNT Register (QDEC_XCNT_REG).
 *
 */
__STATIC_FORCEINLINE int16_t hw_quad_get_xcnt_reg(void)
{
        return QUADEC->QDEC_XCNT_REG;
}

/**
 * \brief Get YCNT Register
 *
 * Returns the contents of the Quadrature Decoder YCNT Register (QDEC_YCNT_REG).
 *
 */
__STATIC_FORCEINLINE int16_t hw_quad_get_ycnt_reg(void)
{
        return QUADEC->QDEC_YCNT_REG;
}

/**
 * \brief Get ZCNT Register
 *
 * Returns the contents of the Quadrature Decoder ZCNT Register (QDEC_ZCNT_REG).
 *
 */
__STATIC_FORCEINLINE int16_t hw_quad_get_zcnt_reg(void)
{
        return QUADEC->QDEC_ZCNT_REG;
}

/**
 * \brief Get Event Count Register
 *
 * Returns the contents of the Quadrature Decoder Event Count Register (QDEC_EVENT_CNT_REG).
 *
 */
__STATIC_FORCEINLINE uint8_t hw_quad_get_event_cnt_reg(void)
{
        return QUADEC->QDEC_EVENT_CNT_REG;
}

/**
 * \brief Quadrature Decoder Clock Enable/Disable.
 *
 * Enables or Disables the Quadrature Decoder clock (CRG_TOP, CLK_AMBA_REG, QDEC_CLK_ENABLE).
 *
 * \note If the clock is NOT enabled, no interaction with the Quadrature Decoder Registers is
 * possible.
 *
 * \param [in] enable defines whether the clock will be enabled or disabled.
 */
__STATIC_FORCEINLINE void hw_quad_qdec_clk_en(HW_QUAD_REG_CLK enable)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QDEC_CLK_ENABLE, enable);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Clear Event Count Register
 *
 * Clears the QDEC_EVENT_CNT_REG. Please note that when an interrupt has been raised the
 * QDEC_EVENT_CNT_REG will continue to count events. No further interrupts will be generated
 * unless the Event Count Register has been cleared.
 *
 */
__STATIC_FORCEINLINE void hw_quad_clr_event_cnt(void)
{
        REG_SETF(QUADEC, QDEC_CTRL_REG, QDEC_EVENT_CNT_CLR, HW_QUAD_REG_CTRL_EVENT_CNT_CLR_ON);
}

/**
 * \brief Clear IRQ
 *
 * Clears pending interrupts.
 *
 */
__STATIC_FORCEINLINE void hw_quad_clr_irq_status(void)
{
        REG_SETF(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_STATUS, HW_QUAD_REG_CTRL_IRQ_STATUS_ON_CLR);
}

/**
 * \brief Return IRQ Status
 *
 * Returns whether an interrupt has occurred or not.
 *
 */
__STATIC_FORCEINLINE HW_QUAD_REG_CTRL_IRQ_EN hw_quad_return_irq_status(void)
{
        return REG_GETF(QUADEC, QDEC_CTRL_REG, QDEC_IRQ_STATUS);
}

/**
 * \brief Enable Interrupt
 *
 * Sets the Control Register (QDEC_CTRL_REG) and registers callback.
 *
 * \note Not to be called before "hw_quad_init" or at least "hw_quad_qdec_clk_en".
 * QDEC will be uninitialized and the block will be without clock. No QDEC-related
 *  registers will be updated whatsoever.
 *
 * \param [in] cb defines the callback intended to be registered.
 * \param [in] thres_num defines the number of events on either counter (X or Y or Z)
 *              that need to be reached before an interrupt is generated. Events are
 *              equal to irq_thres + 1 and are monitored through QDEC_EVENT_CNT_REG.
 * \param [in] user_data_p pointer to additional user data
 *
 * \sa hw_quad_interrupt_disable_and_unregister_interrupt
 *
 */
void hw_quad_interrupt_enable_and_register(hw_quad_handler_cb cb, uint8_t thres_num, void *user_data_p);

/**
 * \brief Disable Interrupt
 *
 * Clears QDEC_CTRL_REG pending IRQ and Event Counter,
 * disables IRQ,s in both QDEC_CTRL_REG and NVIC (unregisters callback)
 *
 * \sa hw_quad_interrupt_enable_and_register
 *
 */
void hw_quad_interrupt_disable_and_unregister(void);

/**
 * \brief Start Quadrature Decoder
 *
 * Sets QDEC_CTRL2_REG Pin Input pairs according to X-Y-Z stored port pairs in
 * hw_x_axis_ports, hw_y_axis_ports and hw_z_axis_ports respectively.
 *
 * \note Function Requires QUAD clock to be ON (e.g. through "hw_quad_init")
 *
 * \sa hw_quad_init
 *
 */
void hw_quad_start(void);

/**
 * \brief Stop Quadrature Decoder
 *
 * Disables QDEC_CTRL2_REG Pin Input pairs, resets counter variables in
 * QDEC_XCNT_REG, QDEC_YCNT_REG. QDEC_ZCNT_REG.
 *
 * \sa hw_quad_init
 *
 */
void hw_quad_stop(void);

/**
 * \brief Disable Quadrature Decoder
 *
 * Disables the quadrature decoder block.
 *
 * \sa hw_quad_init
 *
 */
void hw_quad_disable(void);

/**
 * \brief Initialize Quadrature Decoder
 *
 * Initializes the quadrature decoder block.
 *
 *  \note "hw_quad_start()" required for the Quadrature Decoder to begin
 *
 * \param [in] init_data defines the initialization struct
 *
 * \sa hw_quad_disable
 *
 */
void hw_quad_init(const hw_quad_config_t *init_data);

#endif /* dg_configUSE_HW_QUAD */


#endif /* HW_QUAD_V2_H_ */

/**
 * \}
 * \}
 */
