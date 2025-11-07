/**
 ****************************************************************************************
 *
 * @file hw_pcm.c
 *
 * @brief Implementation of the PCM interface Low Level Driver.
 *
 * Copyright (C) 2019-2023 Renesas Electronics Corporation and/or its affiliates.
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
#if dg_configUSE_HW_PCM

#include "hw_pcm.h"
#include "hw_clk.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()   do { } while (0)
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()   do { } while (0)
#endif

static hw_pcm_interrupt_cb_t hw_pcm_interrupt_cb;

/*
 * Implementation of the binary Greatest Common Divisor (GCD) algorithm
 *
 * param[in] numerator is the numerator of the requested fraction
 * param[in] denominator is the denominator of the requested fraction
 *
 * return    the gcd
 *
 */
static uint32_t calculate_fraction_gcd(uint32_t numerator, uint32_t denominator)
{
        int pof2, tmp;

        if (!numerator || !denominator) {
                return (numerator | denominator);
        }

        // pof2 is the greatest power of 2 dividing both numbers .
        // We will use pof2 to multiply the returning number .
        pof2 = 0;

        while (!(numerator & 1) && !(denominator & 1)) {
                // gcd(even1, even1) = pof2 * gcd(even1/pof2, even2/pof2)
                numerator >>= 1;
                denominator >>= 1;
                pof2++;
        }

        do {
                while (!(numerator & 1)) {
                        numerator >>= 1;
                }

                while (!(denominator & 1)) {
                        denominator >>= 1;
                }

                // At this point we know for sure that
                // numerator and denominator are odd
                if (numerator >= denominator) {
                        numerator = (numerator - denominator) >> 1;
                } else {
                        tmp = numerator;
                        numerator = (denominator - numerator) >> 1;
                        denominator = tmp;
                }
        } while (!((numerator == denominator) || (numerator == 0)));

        return (denominator << pof2);
}

static HW_PCM_ERROR_CODE validate_pcm_init_clk_args(hw_pcm_clk_cfg_t *pcm_clk)
{
        /* verify sample rate is supported. 96ksps is not supported by pcm divisors table */
        if (!((pcm_clk->sample_rate == 8) || (pcm_clk->sample_rate == 16) ||
                (pcm_clk->sample_rate == 32) || (pcm_clk->sample_rate == 48))) {
                return HW_PCM_ERROR_INVALID_SAMPLE_RATE;
        }
        /* The value of bit depth must not exceed the size of single pcm register
         * and an integer multiplier of bytes*/
        if (pcm_clk->bit_depth > 32 || pcm_clk->bit_depth == 0 || pcm_clk->bit_depth % 8 != 0) {
                return HW_PCM_ERROR_INVALID_BIT_DEPTH;
        }
        /* verify channel number is supported */
        if (pcm_clk->chs > 2 || pcm_clk->chs == 0) {
                return HW_PCM_ERROR_INVALID_CHANNELS;
        }
        /* verify channel delay number is supported */
        if (pcm_clk->ch_delay > 3) {
                return HW_PCM_ERROR_INVALID_CHANNEL_DELAYS;
        }
        /* verify slot number is supported */
        if (pcm_clk->slot > 2 ) {
                return HW_PCM_ERROR_INVALID_SLOTS;
        }

        return HW_PCM_ERROR_NO_ERROR;
}

HW_PCM_ERROR_CODE hw_pcm_init_clk(hw_pcm_clk_cfg_t *pcm_clk)
{
        uint8_t cpb = 1;
        uint32_t sys_clk_freq = 0;
        uint32_t bit_clock = 0;
        uint32_t divider = 0;

        HW_PCM_ERROR_CODE ret = validate_pcm_init_clk_args(pcm_clk);

        if (ret != HW_PCM_ERROR_NO_ERROR) {
                return ret;
        }

        uint32_t pcm_div_reg = CRG_AUD->PCM_DIV_REG;

        /* PCM clk configuration */
        /* Set DIVN/DIV1 */
        if (pcm_clk->clock == HW_PCM_CLK_DIV1) {
                /* DIV1 clock used */
                REG_SET_FIELD(CRG_AUD, PCM_DIV_REG, PCM_SRC_SEL, pcm_div_reg, true);
                /* Get sys clk frequency in KHz */
                sys_clk_freq = hw_clk_get_sysclk_freq() / 1000;
        } else {
                /* DIVN clock used */
                REG_CLR_FIELD(CRG_AUD, PCM_DIV_REG, PCM_SRC_SEL, pcm_div_reg);
                sys_clk_freq = 32000;
        }

        /* Calculate bit_clock */
        if (pcm_clk->cycle_per_bit == HW_PCM_TWO_CYCLE_PER_BIT) {
                cpb = 2;
        }

        pcm_clk->fsc_div = (pcm_clk->bit_depth * pcm_clk->chs + pcm_clk->ch_delay * 8 * pcm_clk->slot) * cpb;

        if (pcm_clk->fsc_div == 0 || (pcm_clk->fsc_div / cpb ) > 64 ) {
                return HW_PCM_ERROR_INVALID_FSC_DIV;
        }

        bit_clock = pcm_clk->sample_rate * (pcm_clk->fsc_div);

        if (pcm_clk->div == HW_PCM_CLK_GEN_FRACTIONAL) {
                uint16_t fdiv_fractional = 0x0;
                uint32_t denominator = bit_clock;

                /* Find the numerator */
                divider = sys_clk_freq / bit_clock;

                uint32_t numerator = sys_clk_freq - (bit_clock * divider);

                if (numerator != 0) {

                        uint32_t gcd = calculate_fraction_gcd(numerator, denominator);

                        if (gcd != 0) {
                                /* reduce numerator and denominator by dividing with GCD */
                                numerator /= gcd;
                                denominator /= gcd;
                        }

                        /* Check if numerator and denominator are valid */
                        if ((numerator > denominator) || (denominator > 16)) {
                                return HW_PCM_ERROR_NA_DIV;
                        }

                        /* Calculate fdiv_fractional */
                        fdiv_fractional = 0x1;
                        uint16_t fdiv_numerator = 0xFFFF >> (16 - (numerator - 1));
                        fdiv_fractional = (fdiv_fractional << (denominator - 1)) | fdiv_numerator;
                }

                HW_PCM_CRG_REG_SETF(FDIV, PCM_FDIV, fdiv_fractional);
        } else {
                uint16_t actual_bit_depth = pcm_clk->bit_depth;

                while (actual_bit_depth <= 64) {

                        if (sys_clk_freq % bit_clock == 0) {
                                break;
                        }

                        actual_bit_depth++;
                        /* Recalculate bit clock with actual bit depth*/
                        bit_clock = pcm_clk->sample_rate * (actual_bit_depth * pcm_clk->chs + pcm_clk->ch_delay * 8 * pcm_clk->slot) * cpb;

                }

                if (actual_bit_depth > 64) {
                        return HW_PCM_ERROR_NA_DIV;
                }

                divider = sys_clk_freq / bit_clock;
                HW_PCM_CRG_REG_SETF(FDIV, PCM_FDIV, 0);

                pcm_clk->fsc_div = (actual_bit_depth * pcm_clk->chs + pcm_clk->ch_delay * 8 * pcm_clk->slot) * cpb;
        }

        REG_SET_FIELD(CRG_AUD, PCM_DIV_REG, PCM_DIV, pcm_div_reg, divider);
        REG_SET_FIELD(CRG_AUD, PCM_DIV_REG, CLK_PCM_EN, pcm_div_reg, true);

        CRG_AUD->PCM_DIV_REG = pcm_div_reg;

        return HW_PCM_ERROR_NO_ERROR;
}


static void hw_pcm_init_generic_pcm(hw_pcm_config_generic_pcm_t *config)
{
        uint32_t pcm1_ctrl_reg = PCM1->PCM1_CTRL_REG;

        /* Set channel delay in multiples of 8 bits */
        ASSERT_WARNING(config->channel_delay <= 31);
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CH_DEL, pcm1_ctrl_reg, config->channel_delay);

        /* Set the number of clock cycles per data bit */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT, pcm1_ctrl_reg, HW_PCM_ONE_CYCLE_PER_BIT);

        /* Set polarity of PCM FSC */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCINV, pcm1_ctrl_reg, config->fsc_polarity);

        /* Set polarity of PCM CLK */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLKINV, pcm1_ctrl_reg, config->clock_polarity);

        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCDEL, pcm1_ctrl_reg, config->fsc_delay);

        /* FSC length */
        ASSERT_WARNING(config->fsc_length <= 8);
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCLEN, pcm1_ctrl_reg, config->fsc_length);

        /* Set PCM edge */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_EDGE, pcm1_ctrl_reg, HW_PCM_FSC_EDGE_RISING);

        /* Set PCM FSC divider */
        ASSERT_WARNING(config->fsc_div >= 8 && config->fsc_div <= 0x1000);

        if (pcm1_ctrl_reg & REG_MSK(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT)) {
                ASSERT_ERROR((config->fsc_div % 2 == 0));
        }
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_DIV, pcm1_ctrl_reg, config->fsc_div - 1);

        PCM1->PCM1_CTRL_REG = pcm1_ctrl_reg;

        /* Generic PCM configuration done successfully */
}

static void hw_pcm_init_i2s(hw_pcm_config_i2s_mode_t *config)
{
        uint32_t pcm1_ctrl_reg = PCM1->PCM1_CTRL_REG;

        /* Set channel delay in multiples of 8 bits */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CH_DEL, pcm1_ctrl_reg, 0);

        /* Set PCM edge */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_EDGE, pcm1_ctrl_reg, HW_PCM_FSC_EDGE_RISING_AND_FALLING);

        /* Set the number of clock cycles per data bit */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT, pcm1_ctrl_reg, HW_PCM_ONE_CYCLE_PER_BIT);

        /* Set polarity of PCM FSC */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCINV, pcm1_ctrl_reg, config->fsc_polarity);

        /* Set polarity of PCM CLK */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLKINV, pcm1_ctrl_reg, HW_PCM_CLK_POLARITY_INVERTED);

        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCDEL, pcm1_ctrl_reg, HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT);

        /* FSC length */
        ASSERT_WARNING(config->fsc_length <= 8);
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCLEN, pcm1_ctrl_reg, config->fsc_length);

        /* Set PCM FSC divider */
        ASSERT_WARNING(config->fsc_div >= 8 && config->fsc_div <= 0x1000);

        if (pcm1_ctrl_reg & REG_MSK(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT)) {
                ASSERT_ERROR((config->fsc_div % 2 == 0));
        }
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_DIV, pcm1_ctrl_reg, config->fsc_div - 1);

        PCM1->PCM1_CTRL_REG = pcm1_ctrl_reg;
}

static void hw_pcm_set_init_tdm(hw_pcm_config_tdm_mode_t *config)
{
        uint32_t pcm1_ctrl_reg = PCM1->PCM1_CTRL_REG;

        /* Set channel delay in multiples of 8 bits */
        ASSERT_WARNING(config->channel_delay <= 31);
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CH_DEL, pcm1_ctrl_reg, config->channel_delay);

        /* Set PCM edge */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_EDGE, pcm1_ctrl_reg, HW_PCM_FSC_EDGE_RISING_AND_FALLING);

        /* Set the number of clock cycles per data bit */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT, pcm1_ctrl_reg, HW_PCM_ONE_CYCLE_PER_BIT);

        /* Set polarity of PCM FSC */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCINV, pcm1_ctrl_reg, config->fsc_polarity);

        /* Set polarity of PCM CLK */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLKINV, pcm1_ctrl_reg, HW_PCM_CLK_POLARITY_INVERTED);

        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCDEL, pcm1_ctrl_reg, HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT);

        /* FSC length */
        ASSERT_WARNING(config->fsc_length <= 8);
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCLEN, pcm1_ctrl_reg, config->fsc_length);

        /* Set PCM FSC divider */
        ASSERT_WARNING(config->fsc_div >= 8 && config->fsc_div <= 0x1000);

        if (pcm1_ctrl_reg & REG_MSK(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT)) {
                ASSERT_ERROR((config->fsc_div % 2 == 0));
        }
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_DIV, pcm1_ctrl_reg, config->fsc_div - 1);

        PCM1->PCM1_CTRL_REG = pcm1_ctrl_reg;
}

static void hw_pcm_set_init_iom(hw_pcm_config_iom_mode_t *config)
{
        uint32_t pcm1_ctrl_reg = PCM1->PCM1_CTRL_REG;

        /* Set channel delay in multiples of 8 bits */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CH_DEL, pcm1_ctrl_reg, 0);

        /* Set PCM edge */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_EDGE, pcm1_ctrl_reg, HW_PCM_FSC_EDGE_RISING);

        /* Set the number of clock cycles per data bit */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT, pcm1_ctrl_reg, HW_PCM_TWO_CYCLE_PER_BIT);

        /* Set polarity of PCM FSC */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCINV, pcm1_ctrl_reg, config->fsc_polarity);

        /* Set polarity of PCM CLK */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_CLKINV, pcm1_ctrl_reg, HW_PCM_CLK_POLARITY_NORMAL);

        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCDEL, pcm1_ctrl_reg, HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT);

        /* FSC length */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSCLEN, pcm1_ctrl_reg, 0);

        /* For 2 clock cycles per bit fsc_div must be even */
        ASSERT_WARNING((config->fsc_div % 2) == 0);

        /* Set PCM Frame synchronization divider */
        ASSERT_WARNING(config->fsc_div >= 8 && config->fsc_div <= 0x1000);

        if (pcm1_ctrl_reg & REG_MSK(PCM1, PCM1_CTRL_REG, PCM_CLK_BIT)) {
                ASSERT_ERROR((config->fsc_div % 2 == 0));
        }
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_FSC_DIV, pcm1_ctrl_reg, config->fsc_div - 1);

        PCM1->PCM1_CTRL_REG = pcm1_ctrl_reg;
}

void hw_pcm_init(hw_pcm_config_t *config)
{
        /* Disable PCM */
        hw_pcm_disable();

        /* Write zero value to output registers to force the unused channels to zero */
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_1, 0);
        hw_pcm_output_write(HW_PCM_OUTPUT_REG_2, 0);

        uint32_t pcm1_ctrl_reg = PCM1->PCM1_CTRL_REG;

        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_PPOD, pcm1_ctrl_reg, config->gpio_output_mode);

        /* Set PCM in Master Mode */
        REG_SET_FIELD(PCM1, PCM1_CTRL_REG, PCM_MASTER, pcm1_ctrl_reg, config->pcm_mode);

        PCM1->PCM1_CTRL_REG = pcm1_ctrl_reg;

        /* When fsc_edge == HW_PCM_FSC_EDGE_RISING and two channels are used then
         * the bits_depth must be 32bits because there is no way to define when the
         * bits of the first channel end and where the bits of the second channel begin.
         */
        switch (config->config_mode) {
        case HW_PCM_CONFIG_GENERIC_PCM_MODE:
                hw_pcm_init_generic_pcm(&config->pcm_param);
                break;
        case HW_PCM_CONFIG_I2S_MODE:
                hw_pcm_init_i2s(&config->i2s_param);
                break;
        case HW_PCM_CONFIG_TDM_MODE:
                hw_pcm_set_init_tdm(&config->tdm_param);
                break;
        case HW_PCM_CONFIG_IOM_MODE:
                hw_pcm_set_init_iom(&config->iom_param);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

void hw_pcm_register_interrupt(hw_pcm_interrupt_cb_t cb)
{
        if (cb == NULL) {
                hw_pcm_unregister_interrupt();
                return;
        }

        hw_pcm_interrupt_cb = cb;

        NVIC_ClearPendingIRQ(PCM_IRQn);
        NVIC_EnableIRQ(PCM_IRQn);
}

void hw_pcm_unregister_interrupt(void)
{
        hw_pcm_interrupt_cb = NULL;
        NVIC_DisableIRQ(PCM_IRQn);
}

/**
 * \brief PCM1 Interrupt Handler
 *
 */
void PCM_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();
        if (hw_pcm_interrupt_cb) {
                hw_pcm_interrupt_cb();
        }
        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_PCM */
