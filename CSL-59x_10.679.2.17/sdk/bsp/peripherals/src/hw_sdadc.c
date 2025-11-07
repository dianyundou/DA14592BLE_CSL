/**
 ****************************************************************************************
 *
 * @file hw_sdadc.c
 *
 * @brief Implementation of the SDADC Low Level Driver.
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


#if dg_configUSE_HW_SDADC

#include "hw_sdadc.h"
#if HW_SDADC_DMA_SUPPORT
#include <string.h>
#include "hw_dma.h"

#define SDADC_DMA_TRIGGER                       HW_DMA_TRIG_SD_ADC_FCU
static DMA_setup sdadc_dma_setup;
#endif /* HW_SDADC_DMA_SUPPORT */

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define SDADC_IRQ                               SDADC_IRQn

static uint32_t conversions_to_go;
static uint16_t *sdadc_user_buffer;
static void *sdadc_user_param;
hw_sdadc_read_cb sdadc_user_callback;
static hw_sdadc_interrupt_cb intr_cb = NULL;


/**
 * \brief external reference Calibration Data
 */
typedef struct {
        int16_t gain;   /**< gain */
        int16_t offset; /**< offset */
} hw_sdadc_external_calibration_t;

/*
 * Default vref external values before calibration.
 */
#define EXT_VREF_NO_CALIB_VALUES          {.gain = 0, .offset = 0}

/*
 * Declaring the external calibration values
 */
__RETAINED_RW static hw_sdadc_external_calibration_t external_vref_calibration_value = EXT_VREF_NO_CALIB_VALUES;


/*=============================================================================================*/
/* Help functions for the driver module                                                        */
/*=============================================================================================*/

/**
 * \brief Get trimmed values for gain and offset
 *
 * \param[in]  input_mode indicates differential or single ended
 * \param[out] gain pointer reference to gain correction value
 * \param[out] offs pointer reference to offset correction value
 */
extern void hw_sdadc_get_trimmed_values(HW_SDADC_INPUT_MODE mode, int16_t *gain, int16_t *offs);


static void hw_sdadc_get_trimmed_and_set_to_regs(HW_SDADC_INPUT_MODE mode)
{
        int16_t gain_corr;
        int16_t offs_corr;

        hw_sdadc_get_trimmed_values(mode, &gain_corr, &offs_corr);

        /* set the gain value to REG */
        hw_sdadc_set_gain_correction(gain_corr);

        /* set the corrected offset */
        hw_sdadc_set_offset_correction(offs_corr);
}

void hw_sdadc_store_ext_ref_calibration_values(int16_t gain, int16_t offset)
{
        external_vref_calibration_value.gain = gain;
        external_vref_calibration_value.offset = offset;
}


__STATIC_INLINE void hw_sdadc_set_inp(HW_SDADC_INPUT input)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_INP_SEL, input);
}

__STATIC_INLINE void hw_sdadc_set_inn(HW_SDADC_INPUT input)
{
        ASSERT_WARNING(HW_SDADC_IN_ADC7 >= input);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_INN_SEL, input);
}

void hw_sdadc_set_input_mode(HW_SDADC_INPUT_MODE mode)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_SE, mode);
}


__STATIC_INLINE void hw_sdadc_set_operation_mode(HW_SDADC_MODE mode)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MODE, mode);
}

__STATIC_INLINE HW_SDADC_MODE hw_sdadc_get_operation_mode(void)
{
        return (HW_SDADC_MODE) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_MODE);
}

#if HW_SDADC_DMA_SUPPORT
static DMA_setup sdadc_dma_setup;
/************************************ DMA support functions ************************************/
__STATIC_INLINE void hw_sdadc_dma_reset(void)
{
        /* Invalidate any DMA configuration and the respective channel */
        memset(&sdadc_dma_setup, 0, sizeof(DMA_setup));
        sdadc_dma_setup.channel_number = HW_DMA_CHANNEL_INVALID;
}
#endif

void hw_sdadc_start(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_START, 1);
}

void hw_sdadc_stop_and_wait(void)
{
        hw_sdadc_set_continuous(false);
        while (hw_sdadc_in_progress());
}

/*=============================================================================================*/
/* Basic functionality of the SDADC                                                            */
/*=============================================================================================*/

void hw_sdadc_init(const hw_sdadc_config_t *cfg)
{
        hw_sdadc_stop_and_wait();

        SDADC->SDADC_CTRL_REG = 0;

        NVIC_DisableIRQ(SDADC_IRQ);
        NVIC_ClearPendingIRQ(SDADC_IRQ);

#if HW_SDADC_DMA_SUPPORT
        hw_sdadc_dma_reset();
#endif
        if (cfg) {
                hw_sdadc_enable();
                hw_sdadc_configure(cfg);
        }
}

void hw_sdadc_audio_init(hw_sdadc_interrupt_cb cb, const hw_sdadc_pga_config_t *cfg)
{
        /* Reset configuration */
        SDADC->SDADC_CTRL_REG = 0;

        hw_sdadc_unregister_interrupt();

        hw_sdadc_set_operation_mode(HW_SDADC_MODE_AUDIO);
        hw_sdadc_set_input_mode(HW_SDADC_INPUT_MODE_DIFFERENTIAL);
        hw_sdadc_set_vref_sel(HW_SDADC_VREF_INTERNAL);
        hw_sdadc_set_inp(HW_SDADC_IN_ADC0);
        hw_sdadc_set_inn(HW_SDADC_IN_ADC1);
        hw_sdadc_set_continuous(true);
        hw_sdadc_enable();

        if (cfg) {
                /* PGA configuration */
                hw_sdadc_pga_unmute();
                hw_sdadc_pga_set_gain(cfg->pga_gain);
                ASSERT_ERROR((cfg->pga_mode & 0x03) != HW_SDADC_PGA_MODE_RESERVED);
                hw_sdadc_pga_set_mode(cfg->pga_mode);
                hw_sdadc_pga_set_bias(cfg->pga_bias);
                hw_sdadc_pga_select_enabled_channels(cfg->pga_en);
        }

        if (cb) {
                hw_sdadc_register_interrupt(cb);
        }
}

#if HW_SDADC_DMA_SUPPORT
static void hw_sdadc_dma_configure(hw_sdadc_dma_cfg_t *cfg)
{
        if (!cfg) {
                return;
        }
        ASSERT_WARNING(hw_sdadc_get_continuous());
        /* make sure that the selected dma channel number is even */
        ASSERT_WARNING((cfg->channel & 0x1) == 0);

        hw_sdadc_set_dma_functionality(true);
        /*
         * Volatile user configurable settings
         */
        sdadc_dma_setup.channel_number = cfg->channel;
        sdadc_dma_setup.dma_prio = cfg->prio;
        sdadc_dma_setup.irq_nr_of_trans = cfg->irq_nr_of_trans;
        sdadc_dma_setup.circular = cfg->circular ? HW_DMA_MODE_CIRCULAR : HW_DMA_MODE_NORMAL;
        /*
         * System level fixed settings
         */
        sdadc_dma_setup.bus_width = HW_DMA_BW_HALFWORD;
        sdadc_dma_setup.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
        sdadc_dma_setup.dreq_mode = HW_DMA_DREQ_TRIGGERED;
        sdadc_dma_setup.burst_mode = HW_DMA_BURST_MODE_DISABLED;
        sdadc_dma_setup.a_inc = HW_DMA_AINC_FALSE;
        sdadc_dma_setup.b_inc = HW_DMA_BINC_TRUE;
        sdadc_dma_setup.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE;
        sdadc_dma_setup.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
        sdadc_dma_setup.dma_req_mux = SDADC_DMA_TRIGGER;
        sdadc_dma_setup.src_address = (uint32_t) &SDADC->SDADC_RESULT_REG;
}
#endif /* HW_SDADC_DMA_SUPPORT */

void hw_sdadc_configure(const hw_sdadc_config_t *cfg)
{
        if (!cfg) {
                return;
        }
        ASSERT_WARNING(!hw_sdadc_in_progress());
        /* For audio mode use hw_sdadc_audio_init() */
        ASSERT_WARNING(hw_sdadc_get_operation_mode() == HW_SDADC_MODE_SENSOR);
        /* The first two input channels are dedicated to audio operation mode */
        ASSERT_WARNING(cfg->inp > HW_SDADC_IN_ADC1);
        if (cfg->input_mode == HW_SDADC_INPUT_MODE_DIFFERENTIAL) {
                ASSERT_WARNING(cfg->inn > HW_SDADC_IN_ADC1);
        }

        if (HW_SDADC_VREF_INTERNAL == cfg->vref_selection) {
                if (HW_SDADC_VREF_VOLTAGE_INTERNAL != cfg->vref_voltage) {
                        /*
                         * Internal reference voltage is set by system to 0.9V
                         * A different value will yield erroneous conversion
                         */
                        ASSERT_WARNING(0);
                        return;
                }
                /* input mode affects the internal gain/offset values */
                hw_sdadc_get_trimmed_and_set_to_regs(cfg->input_mode);
        } else {
                /* set the calibrated gain/offset values for external vref to REGs */
                hw_sdadc_set_gain_correction(external_vref_calibration_value.gain);
                hw_sdadc_set_offset_correction(external_vref_calibration_value.offset);
        }

        if (!(HW_SDADC_VREF_IN_RANGE(cfg->vref_voltage))) {

                /*
                 * Reference voltage must not exceed HW limits
                 */
                ASSERT_WARNING(0);
                return;
        }

        /*
         * Selecting the external reference voltage setting has some pre-requisites:
         * The external reference voltage should be applied to pins P1.5 (ADC4) and/or P1.6 (ADC5).
         * Therefore, these pins cannot serve as input at the same time.
         * Those pins are available only in QFN package.
         */
        switch (cfg->vref_selection) {
        case HW_SDADC_VREFN_INTERNAL_VREFP_EXTERNAL:
                ASSERT_WARNING(cfg->inp != HW_SDADC_IN_ADC4);
                break;
        case HW_SDADC_VREFN_EXTERNAL_VREFP_INTERNAL:
                ASSERT_WARNING(cfg->inn != HW_SDADC_IN_ADC5);
                break;
        case HW_SDADC_VREF_EXTERNAL:
                ASSERT_WARNING(cfg->inp != HW_SDADC_IN_ADC4);
                ASSERT_WARNING(cfg->inn != HW_SDADC_IN_ADC5);
                break;
        default:
                break;
        }
        hw_sdadc_stop_and_wait();
        hw_sdadc_set_vref_sel(cfg->vref_selection);
        hw_sdadc_set_input_mode(cfg->input_mode);
        hw_sdadc_set_inp(cfg->inp);
        hw_sdadc_set_inn(cfg->inn);
        hw_sdadc_set_continuous(cfg->continuous);
        hw_sdadc_set_oversampling(cfg->over_sampling);
#if HW_SDADC_DMA_SUPPORT
        hw_sdadc_dma_configure(cfg->dma_setup);
#endif
}
int32_t hw_sdadc_convert_to_millivolt(const hw_sdadc_config_t *cfg, uint16_t raw)
{
        int32_t converted;
        uint16_t max;
        uint8_t attenuator = 0x01;

        if (HW_SDADC_INP_VBAT == cfg->inp) {
                attenuator = 0x04;
        }

        switch (cfg->input_mode) {
        case HW_SDADC_INPUT_MODE_SINGLE_ENDED:
                converted = raw;
                max = UINT16_MAX;
                break;
        case HW_SDADC_INPUT_MODE_DIFFERENTIAL:
                converted = (int16_t) raw;
                max = INT16_MAX;
                break;
        default:
                ASSERT_WARNING(0);
                return 0; // Should never reach this line
        }
        converted = converted * attenuator * cfg->vref_voltage;
        return converted / max;
}

int32_t hw_sdadc_get_voltage(const hw_sdadc_config_t *cfg)
{
        uint16_t adc_raw;

        hw_sdadc_start(); // Start AD conversion

        while (hw_sdadc_in_progress());
        hw_sdadc_clear_interrupt();
        adc_raw = hw_sdadc_get_raw_value();
        return hw_sdadc_convert_to_millivolt(cfg, adc_raw);
}
void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb)
{
        intr_cb = cb;

        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 1);

        NVIC_ClearPendingIRQ(SDADC_IRQ);
        NVIC_EnableIRQ(SDADC_IRQ);
}

void hw_sdadc_unregister_interrupt(void)
{
        NVIC_DisableIRQ(SDADC_IRQ);
        NVIC_ClearPendingIRQ(SDADC_IRQ);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 0);

        intr_cb = NULL;
}

void SDADC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        hw_sdadc_clear_interrupt();
        if (intr_cb) {
                intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

/**
 * \brief Set the ADC engine to stop, unregister any interrupt handler and exit immediately
 *
 */
__STATIC_INLINE void hw_sdadc_stop_no_wait(void)
{
        hw_sdadc_set_continuous(false);
        hw_sdadc_unregister_interrupt();
}

static void read_irq_callback_wrapper(void)
{
        conversions_to_go--;

        /* Last interrupt handling */
        if (conversions_to_go == 0) {
                intr_cb = hw_sdadc_stop_no_wait;
        }

        if (sdadc_user_buffer) {
                *sdadc_user_buffer = hw_sdadc_get_raw_value();
                sdadc_user_buffer++;
                if (conversions_to_go == 0) {
                        sdadc_user_callback(sdadc_user_param, conversions_to_go);
                }
        } else {
                /* No buffer forces callback on every interrupt */
                sdadc_user_callback(sdadc_user_param, conversions_to_go);
        }
}

static bool read_irq_mode()
{
        if (conversions_to_go > 1) {
                ASSERT_WARNING(hw_sdadc_get_continuous());
        }

        hw_sdadc_register_interrupt(read_irq_callback_wrapper);

        hw_sdadc_start();
        return true;
}

#if HW_SDADC_DMA_SUPPORT
static void read_dma_callback_wrapper(void *user_data, dma_size_t len)
{
        /* Variable len holds the total DMA transferred items so far */
        conversions_to_go -= len;

        if ((len >= sdadc_dma_setup.length) && (sdadc_dma_setup.circular == HW_DMA_MODE_NORMAL)) {
                hw_sdadc_stop_no_wait();
        }

        if (sdadc_user_callback) {
                /* Notifies the user about remaining conversions */
                sdadc_user_callback(sdadc_user_param, conversions_to_go);
        }

        if (sdadc_dma_setup.irq_nr_of_trans) {
                uint16_t next_step = MIN(sdadc_dma_setup.length - 1, len + sdadc_dma_setup.irq_nr_of_trans - 1);
                hw_dma_channel_update_int_ix(sdadc_dma_setup.channel_number, next_step);
        }
}

static bool read_dma_mode()
{
        if (conversions_to_go > 1) {
                /* In interrupt and DMA modes the ADC engine needs to operate in continuous mode */
                ASSERT_WARNING(hw_sdadc_get_continuous());
        }
        if ((dma_size_t)sdadc_dma_setup.irq_nr_of_trans > conversions_to_go) {
                /* Invalid DMA configuration */
                return false;
        }
        if (sdadc_dma_setup.irq_nr_of_trans > 0 && sdadc_dma_setup.circular == HW_DMA_MODE_CIRCULAR) {
                /* This option is not supported */
                return false;
        }

        /*
         * Setup DMA - Enable channel
         */
        sdadc_dma_setup.length = (dma_size_t) conversions_to_go;
        sdadc_dma_setup.dest_address = (uint32_t) sdadc_user_buffer;
        sdadc_dma_setup.callback = read_dma_callback_wrapper;
        hw_dma_channel_initialization(&sdadc_dma_setup);

        hw_dma_channel_enable(sdadc_dma_setup.channel_number, HW_DMA_STATE_ENABLED);

        hw_sdadc_start();
        return true;
}
#endif

static bool read_polling_mode()
{
        if (conversions_to_go == 1) {
                hw_sdadc_adc_measure();
                /* outbuf can be omitted - the result register holds the result */
                if (sdadc_user_buffer) {
                        *sdadc_user_buffer = hw_sdadc_get_raw_value();
                }
                return true;
        }

        if (!sdadc_user_buffer) {
                /* A buffer is mandatory to store multiple results */
                return false;
        }

        for (uint32_t i = 0; i < conversions_to_go; i++) {
                hw_sdadc_adc_measure();
                sdadc_user_buffer[i] = hw_sdadc_get_raw_value();
        }
        return true;
}
bool hw_sdadc_read(uint32_t nof_conv, uint16_t *out_buf, hw_sdadc_read_cb cb, void *user_data)
{
        if (nof_conv == 0) {
                return false;
        }

        if (hw_sdadc_in_progress()) {
                return false;
        }

        /*
         * Update local data
         */
        sdadc_user_buffer = out_buf;
        sdadc_user_callback = cb;
        sdadc_user_param = user_data;
        conversions_to_go = nof_conv;

#if HW_SDADC_DMA_SUPPORT
        if (sdadc_dma_setup.channel_number < HW_DMA_CHANNEL_INVALID) {
                /* A buffer is mandatory to set the DMA destination address */
                ASSERT_WARNING(sdadc_user_buffer != NULL);

                return read_dma_mode();
        }
#endif

        if (sdadc_user_callback) {
                return read_irq_mode();
        } else {
                return read_polling_mode();
        }
}

void hw_sdadc_abort_read(void)
{
        hw_sdadc_stop_and_wait();

#if HW_SDADC_DMA_SUPPORT
        if (sdadc_dma_setup.channel_number < HW_DMA_CHANNEL_INVALID) {
                if (hw_dma_is_channel_active(sdadc_dma_setup.channel_number)) {
                        /*
                         * DMA callback in stop
                         */
                        hw_dma_channel_stop(sdadc_dma_setup.channel_number);
                }
                if (sdadc_user_callback) {
                        conversions_to_go -= hw_dma_transfered_bytes(sdadc_dma_setup.channel_number);
                        sdadc_user_callback(sdadc_user_param, conversions_to_go);
                }
                return;
        }
#endif
        if (sdadc_user_callback) {
                hw_sdadc_unregister_interrupt();
                sdadc_user_callback(sdadc_user_param, conversions_to_go);
        }
}

#endif /* dg_configUSE_HW_SDADC */

