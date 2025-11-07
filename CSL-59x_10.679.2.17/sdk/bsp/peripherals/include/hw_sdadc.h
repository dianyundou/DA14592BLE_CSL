/**
 * \addtogroup PLA_DRI_PER_ANALOG
 *
 * \{
 *
 * \addtogroup HW_SDADC SDADC Driver
 *
 * \brief Sigma Delta ADC
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_sdadc.h
 *
 * @brief Definition of API for the SDADC Low Level Driver.
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
#ifndef HW_SDADC_H
#define HW_SDADC_H


#if dg_configUSE_HW_SDADC

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/**
 * \def HW_SDADC_DMA_SUPPORT
 *
 * \brief DMA support for SDADC
 *
 */
#define HW_SDADC_DMA_SUPPORT                    dg_configSDADC_DMA_SUPPORT

/*=============================================================================================*/
/* Macro, type and data-structure definitions                                                  */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_DATA SDADC Data Types
 *
 * \brief Enumeration, structure, type and macro definitions
 *
 * \{
 */
#if HW_SDADC_DMA_SUPPORT
#include "hw_dma.h"

/**
 * \brief Cut-down to necessary DMA configuration
 */
typedef struct {
        HW_DMA_CHANNEL       channel;                   /**< DMA Channel Number to be used */
        HW_DMA_PRIO          prio;                      /**< Channel priority from 0 to 7 */
        bool                 circular;                  /**< Select normal or circular operation for DMA */
        uint16               irq_nr_of_trans;           /**< Number of transfers before IRQ generation */
} hw_sdadc_dma_cfg_t;
#endif

/**
 * \brief ADC callback for read function
 *
 */
typedef void (*hw_sdadc_read_cb)(void *user_data, uint32_t conv_to_go);

/**
 * \brief SDADC input mode
 *
 */
typedef enum {
        HW_SDADC_INPUT_MODE_DIFFERENTIAL = 0,           /**< Differential mode (default) */
        HW_SDADC_INPUT_MODE_SINGLE_ENDED = 1            /**< Single ended mode. Input selection negative side is ignored */
} HW_SDADC_INPUT_MODE;

/**
 * \brief SDADC VREF selection
 *
 */
typedef enum {
        HW_SDADC_VREF_INTERNAL = 0,                     /**<  VREFN=internal VREFP=internal */
        HW_SDADC_VREFN_INTERNAL_VREFP_EXTERNAL = 1,     /**<  VREFN=internal VREFP=external */
        HW_SDADC_VREFN_EXTERNAL_VREFP_INTERNAL = 2,     /**<  VREFN=external VREFP=internal */
        HW_SDADC_VREF_EXTERNAL = 3,                     /**<  VREFN=external VREFP=external */
} HW_SDADC_VREF_SEL;

/**
 * \brief SDADC Reference Voltage level in milliVolt
 *
 */
/**
 * \brief Internal reference Voltage level is fixed to 900 millivolt.
 */
#define HW_SDADC_VREF_VOLTAGE_INTERNAL          ( 900 ) /**< Do not change! */
/**
 * \brief Macro giving the maximum allowed reference voltage level in millivolt
 *
 */
#define HW_SDADC_VREF_VOLTAGE_MAX               HW_SDADC_VREF_VOLTAGE_INTERNAL

#define HW_SDADC_VREF_IN_RANGE(voltage)    ((uint16_t)voltage <= HW_SDADC_VREF_VOLTAGE_MAX)

/**
 * \brief SDADC Input Channel
 *
 */
typedef enum {
        HW_SDADC_IN_ADC0_P1_00 = 0,                     /**< GPIO P1_00 */
        HW_SDADC_IN_ADC1_P1_01 = 1,                     /**< GPIO P1_01 */
        HW_SDADC_IN_ADC2_P1_02 = 2,                     /**< GPIO P1_02 */
        HW_SDADC_IN_ADC3_P0_10 = 3,                     /**< GPIO P0_10 */
        HW_SDADC_IN_ADC4_P1_05 = 4,                     /**< GPIO P1_05 */
        HW_SDADC_IN_ADC5_P1_06 = 5,                     /**< GPIO P1_06 */
        HW_SDADC_IN_ADC6_P1_09 = 6,                     /**< GPIO P1_09 */
        HW_SDADC_IN_ADC7_P1_11 = 7,                     /**< GPIO P1_11 */
        HW_SDADC_IN_VBAT = 8,                           /**< VBAT via 4x attenuator, negative side (INN) connected to ground */
} HW_SDADC_INPUT_CHANNEL;

/**
 * \brief SDADC mode (sensor/audio)
 *
 */
typedef enum {
        HW_SDADC_MODE_SENSOR = 0,                       /**< 0: Sensor mode (default) */
        HW_SDADC_MODE_AUDIO  = 1                        /**< 1: Audio mode */
} HW_SDADC_MODE;

/**
 * \brief SDADC input selection.
 * Generic names mapping to specific GPIO pins for both positive and negative channels
 *
 * \sa HW_SDADC_INPUT_CHANNEL
 *
 */
typedef enum {
        HW_SDADC_IN_ADC0 = HW_SDADC_IN_ADC0_P1_00,
        HW_SDADC_IN_ADC1 = HW_SDADC_IN_ADC1_P1_01,
        HW_SDADC_IN_ADC2 = HW_SDADC_IN_ADC2_P1_02,
        HW_SDADC_IN_ADC3 = HW_SDADC_IN_ADC3_P0_10,
        HW_SDADC_IN_ADC4 = HW_SDADC_IN_ADC4_P1_05,
        HW_SDADC_IN_ADC5 = HW_SDADC_IN_ADC5_P1_06,
        HW_SDADC_IN_ADC6 = HW_SDADC_IN_ADC6_P1_09,
        HW_SDADC_IN_ADC7 = HW_SDADC_IN_ADC7_P1_11,
        HW_SDADC_INP_VBAT = HW_SDADC_IN_VBAT,
} HW_SDADC_INPUT;

/**
 * \brief SDADC oversampling rate
 *
 */
typedef enum {
        HW_SDADC_OSR_256,                               /**< 0: 256 samples (default) */
        HW_SDADC_OSR_512,                               /**< 1: 512 samples */
        HW_SDADC_OSR_1024,                              /**< 2: 1024 samples */
        HW_SDADC_OSR_2048,                              /**< 3: 2048 samples */
} HW_SDADC_OSR;


/**
 * \brief PGA gain selection
 *
 */
typedef enum {
        HW_SDADC_PGA_GAIN_MINUS_12dB = 0,               /**< 0 : -12 dB (default) */
        HW_SDADC_PGA_GAIN_MINUS_6dB = 1,                /**< 1 : -6 dB */
        HW_SDADC_PGA_GAIN_MINUS_0dB = 2,                /**< 2 : 0 dB */
        HW_SDADC_PGA_GAIN_6dB = 3,                      /**< 3 : 6 dB */
        HW_SDADC_PGA_GAIN_12dB = 4,                     /**< 4 : 12 dB */
        HW_SDADC_PGA_GAIN_18dB = 5,                     /**< 5 : 18 dB */
        HW_SDADC_PGA_GAIN_24dB = 6,                     /**< 6 : 24 dB */
        HW_SDADC_PGA_GAIN_30dB = 7,                     /**< 7 : 30 dB */
} HW_SDADC_PGA_GAIN;

/**
 * \brief PGA mode selection
 *
 */
typedef enum {
        HW_SDADC_PGA_MODE_DIFF = 0,                     /**< 0 : Differential mode (default) */
        HW_SDADC_PGA_MODE_SE_N = 1,                     /**< 1 : Use N-branch as single ended mode */
        HW_SDADC_PGA_MODE_SE_P = 2,                     /**< 2 : Use P-branch as single ended mode */
        HW_SDADC_PGA_MODE_RESERVED = 3,                 /**< 3 : Reserved (do not use) */
} HW_SDADC_PGA_MODE;


/**
 * \brief PGA bias configuration
 *
 */
typedef enum {
        HW_SDADC_PGA_BIAS_40 = 0,                       /**< 0 :0.40 x Ibias */
        HW_SDADC_PGA_BIAS_44 = 1,                       /**< 1 :0.44 x Ibias */
        HW_SDADC_PGA_BIAS_50 = 2,                       /**< 2 :0.50 x Ibias */
        HW_SDADC_PGA_BIAS_57 = 3,                       /**< 3 :0.57 x Ibias */
        HW_SDADC_PGA_BIAS_66 = 4,                       /**< 4 :0.66 x Ibias (default) */
        HW_SDADC_PGA_BIAS_80 = 5,                       /**< 5 :0.80 x Ibias */
        HW_SDADC_PGA_BIAS_100 = 6,                      /**< 6 :1.00 x Ibias */
        HW_SDADC_PGA_BIAS_133 = 7,                      /**< 7 :1.33 x Ibias */
} HW_SDADC_PGA_BIAS;

/**
 * \brief PGA enabled branch(es)
 *
 */
typedef enum {
        HW_SDADC_PGA_ENABLE_NONE = 0,                   /**< 00 : both branches of PGA disabled */
        HW_SDADC_PGA_ENABLE_POSITIVE = 1,               /**< 01 : Positive branch of PGA enabled, Negative branch disabled */
        HW_SDADC_PGA_ENABLE_NEGATIVE = 2,               /**< 10 : Positive branch of PGA disabled, Negative branch enabled */
        HW_SDADC_PGA_ENABLE_BOTH = 3,                   /**< 11 : Both branches of PGA enabled */
} HW_SDADC_PGA_EN;

typedef struct {
        HW_SDADC_PGA_GAIN      pga_gain;                /**< PGA gain selection */
        HW_SDADC_PGA_MODE      pga_mode;                /**< PGA mode selection (differential/positive/negative) */
        HW_SDADC_PGA_BIAS      pga_bias;                /**< PGA bias selection */
        HW_SDADC_PGA_EN        pga_en;                  /**< PGA branch enabling */
} hw_sdadc_pga_config_t;


/**
 * \brief SDADC interrupt handler
 *
 */
typedef void (*hw_sdadc_interrupt_cb)(void);

/**
 * \brief SDADC configuration
 *
 */
typedef struct {
        HW_SDADC_INPUT_MODE    input_mode;              /**< Input mode */
        HW_SDADC_INPUT         inn;                     /**< ADC negative input */
        HW_SDADC_INPUT         inp;                     /**< ADC positive input */
        bool                   continuous;              /**< Continuous mode state */
        HW_SDADC_OSR           over_sampling;           /**< Oversampling rate */
        HW_SDADC_VREF_SEL      vref_selection;          /**< VREF source selection (internal/external) Note:external VREF is only available for QFN packaging */
        uint16_t               vref_voltage;            /**< Reference voltage (mV) */
#if HW_SDADC_DMA_SUPPORT
        hw_sdadc_dma_cfg_t     *dma_setup;              /**< DMA configuration - NULL to disable */
#endif
        hw_sdadc_pga_config_t  *pga_setup;
} hw_sdadc_config_t;

/**
 * \}
 */

/*
 * Necessary forward declaration
 */
__STATIC_INLINE bool hw_sdadc_in_progress(void);

/*=============================================================================================*/
/* Configuring the SDADC                                                                       */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_CONFIGURATION Configuration options for the SDADC
 *
 * \brief Access to specific hw_sdadc_config_t structure members and other essential configuration
 *
 * \{
 */

/**
 * \brief Enable SDADC interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_enable_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 1);
}

/**
 * \brief Disable SDADC interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_disable_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 0);
}

/**
 * \brief Get the status of the SDADC maskable interrupt (MINT) to the CPU
 *
 * \return SDADC maskable interrupt (MINT) status
 *
 */
__STATIC_INLINE bool hw_sdadc_is_interrupt_enabled(void)
{
        return (!!(REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_MINT)));
}

/**
 * \brief Set continuous mode
 *
 * With continuous mode enabled SDADC will automatically restart conversion once completed. It's still
 * required to start first conversion using hw_sdadc_start().
 *
 * \param [in] enabled continuous mode state
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE void hw_sdadc_set_continuous(bool enabled)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_CONT, (uint32_t) !!enabled);
}

/**
 * \brief Get continuous mode state
 *
 * \return continuous mode state
 *
 */
__STATIC_INLINE bool hw_sdadc_get_continuous(void)
{
        return (!!(REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_CONT)));
}

/**
 * \brief Enable/Disable DMA functionality
 *
 * \param [in] enabled When true, DMA functionality is enabled
 *
 */
__STATIC_INLINE void hw_sdadc_set_dma_functionality(bool enabled)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN, !!enabled);
}

/**
 * \brief Get DMA functionality state
 *
 * \return DMA functionality state
 *
 */
__STATIC_INLINE bool hw_sdadc_get_dma_functionality(void)
{
        return (bool) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN);
}

/**
 * \brief Set input mode
 *
 * \param [in] mode input mode
 *
 */
void hw_sdadc_set_input_mode(HW_SDADC_INPUT_MODE mode);

/**
 * \brief Get current input mode
 *
 * return input mode
 *
 */
__STATIC_INLINE HW_SDADC_INPUT_MODE hw_sdadc_get_input_mode(void)
{
        return (HW_SDADC_INPUT_MODE) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_SE);
}

/**
 * \brief Set oversampling
 *
 * With oversampling enabled multiple successive conversions will be executed and results are added
 * together to increase effective number of bits in result.
 *
 * \param [in] osr oversampling rate
 *
 */
__STATIC_FORCEINLINE void hw_sdadc_set_oversampling(HW_SDADC_OSR osr)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_OSR, osr);
}
/**
 * \brief Get current oversampling
 *
 * \return oversampling rate
 *
 */
__STATIC_INLINE HW_SDADC_OSR hw_sdadc_get_oversampling(void)
{
        return (HW_SDADC_OSR) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_OSR);
}

/**
 * \brief Select voltage reference source
 *
 * \note external voltage reference can be applied only in QFN package
 *
 * \param [in] vref_sel source to set the reference voltage
 */
__STATIC_INLINE void hw_sdadc_set_vref_sel(HW_SDADC_VREF_SEL vref_sel)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_SEL, vref_sel);
}

/**
 * \brief Get voltage reference source
 *
 * \note external voltage reference can be applied only in QFN package
 *
 * \return voltage reference source (internal/external)
 *
 */
__STATIC_INLINE HW_SDADC_VREF_SEL hw_sdadc_get_vref_selection(void)
{
        return (HW_SDADC_VREF_SEL) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_SEL);
}

/**
 * \brief Get the result register value.
 *
 * \return result value
 *
 */
__STATIC_INLINE uint16_t hw_sdadc_get_raw_value(void)
{
        return REG_GETF(SDADC, SDADC_RESULT_REG, SDADC_VAL);
}


/**
 * \brief Connect reference voltage to GPIO pad
 *
 * Connect internal SDADC reference voltage VREF (0.9V) to P1[5]
 *
 */
__STATIC_INLINE void hw_sdadc_connect_vref_to_pad(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_TO_PAD, 1);
}

/**
 * \brief Disconnect reference voltage from GPIO pad
 *
 * Connect internal SDADC reference voltage VREF (0.9V) to P1[5]
 *
 */
__STATIC_INLINE void hw_sdadc_disconnect_vref_from_pad(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_TO_PAD, 0);
}

/*
 * \brief Check reference voltage connection to GPIO pad
 *
 * Connect internal SDADC reference voltage VREF (0.9V) to P1[5]
 * 0: Disconnected (default)
 * 1: Connected
 *
 * \return reference voltage connected/disconnected to P1[5]
 *
 */
__STATIC_INLINE bool hw_sdadc_is_vref_connected_to_pad(void)
{
        return (!!(REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_TO_PAD)));
}

/**
 * \}
 */

/*=============================================================================================*/
/* Calibration functions                                                                       */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_CALIBR_FUNCTIONS Calibration functions
 *
 * \brief Gain and offset calibration
 *
 * \{
 */
/**
 * \brief Set gain correction.
 *
 * \param [in] gain gain value
 *
 */
__STATIC_FORCEINLINE void hw_sdadc_set_gain_correction(uint16_t gain)
{
        REG_SETF(SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR, gain);
}

/**
 * \brief Get current gain value
 *
 * \return gain value
 *
 */
__STATIC_INLINE uint16_t hw_sdadc_get_gain_correction(void)
{
        return (uint16_t) REG_GETF(SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR);
}

/**
 * \brief Set offset correction
 *
 * \param [in] offset offset value
 *
 */
__STATIC_INLINE void hw_sdadc_set_offset_correction(uint16_t offset)
{
        REG_SETF(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR, offset);
}

/**
 * \brief Get current offset value
 *
 * \return offset value
 *
 */
__STATIC_INLINE uint16_t hw_sdadc_get_offset_correction(void)
{
        return REG_GETF(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR);
}

/**
 * \}
 */

/*=============================================================================================*/
/* Configuring the PGA in SDADC                                                                */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_PGA Programmable Gain Amplifier
 *
 * \brief Controlling the PGA in SDADC
 *
 * \{
 */

/**
 * \brief Set the PGA gain
 *
 * \param [in] gain gain of the PGA
 */
__STATIC_INLINE void hw_sdadc_pga_set_gain(HW_SDADC_PGA_GAIN gain)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_GAIN, gain);
}

/**
 * \brief Get the PGA gain status
 *
 * \return PGA current gain
 */
__STATIC_INLINE HW_SDADC_PGA_GAIN hw_sdadc_pga_get_gain(void)
{
        return (HW_SDADC_PGA_GAIN) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_GAIN);
}

/**
 * \brief Set the PGA branch mode. Use PGA in single ended/differential mode
 *
 * \param [in] mode mode of the PGA positive and negative branches
 *
 */
__STATIC_INLINE void hw_sdadc_pga_set_mode(HW_SDADC_PGA_MODE mode)
{
        ASSERT_WARNING((mode & 0x03) != HW_SDADC_PGA_MODE_RESERVED);
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SINGLE, mode);
}

/**
 * \brief Get the PGA branch mode status
 *
 * \return PGA current mode of the positive and negative branches
 */
__STATIC_INLINE HW_SDADC_PGA_MODE hw_sdadc_pga_get_mode(void)
{
        return (HW_SDADC_PGA_MODE) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SINGLE);
}

/**
 * \brief Mute the PGA
 *
 */
__STATIC_INLINE void hw_sdadc_pga_mute(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE, 1);
}

/**
 * \brief Un-mute the PGA
 *
 */
__STATIC_INLINE void hw_sdadc_pga_unmute(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE, 0);
}

/**
 * \brief Get the PGA mute status
 *
 * \return PGA is in mute or unmute state
 */
__STATIC_INLINE bool hw_sdadc_pga_is_mute(void)
{
        return REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_MUTE);
}

/**
 * \brief Set the PGA bias
 *
 * \param [in] bias PGA bias control value
 */
__STATIC_INLINE void hw_sdadc_pga_set_bias(HW_SDADC_PGA_BIAS bias)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_BIAS, bias);
}

/**
 * \brief Get the PGA bias status
 *
 * \return PGA current bias
 */
__STATIC_INLINE HW_SDADC_PGA_BIAS hw_sdadc_pga_get_bias(void)
{
        return (HW_SDADC_PGA_BIAS) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_BIAS);
}


/**
 * \brief Short the PGA input channels
 *
 */
__STATIC_INLINE void hw_sdadc_pga_short_inputs(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN, 1);
}

/**
 * \brief Disconnect the short-circuited PGA input channels
 *
 */
__STATIC_INLINE void hw_sdadc_pga_unshort_inputs(void)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN, 0);
}

/**
 * \brief Get the PGA short-input status
 *
 * \return true if input channels are shorted, false otherwise
 */
__STATIC_INLINE bool hw_sdadc_pga_inputs_are_shorted(void)
{
        return REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_SHORTIN);
}

/**
 * \brief Select the enabled PGA input channel(s)
 *
 * \param [in] channels selection of channel to enable
 */
__STATIC_INLINE void hw_sdadc_pga_select_enabled_channels(HW_SDADC_PGA_EN channels)
{
        REG_SETF(SDADC, SDADC_PGA_CTRL_REG, PGA_EN, channels);
}

/**
 * \brief Get the PGA enabled input-channel status
 *
 * \return PGA current gain
 */
__STATIC_INLINE HW_SDADC_PGA_GAIN hw_sdadc_pga_enabled_channels_status(void)
{
        return (HW_SDADC_PGA_GAIN) REG_GETF(SDADC, SDADC_PGA_CTRL_REG, PGA_EN);
}

/**
 * \brief Set the audio filter register
 *
 * Constant CIC offset
 *
 * \param [in] val
 *
 */
__STATIC_INLINE void hw_sdadc_set_cic_offset(uint32_t val)
{
        ASSERT_WARNING(hw_sdadc_in_progress() == false);
        REG_SETF(SDADC, SDADC_AUDIO_FILT_REG, SDADC_CIC_OFFSET, val);
}

/**
 * \brief Get the audio filter register status
 *
 * Constant CIC offset
 *
 * \return cic offset
 *
 */
__STATIC_INLINE uint32_t hw_sdadc_get_cic_offset(void)
{
        return REG_GETF(SDADC, SDADC_AUDIO_FILT_REG, SDADC_CIC_OFFSET);
}

/**
 * \}
 */

/*=============================================================================================*/
/* Basic functionality of the SDADC                                                            */
/*=============================================================================================*/

/**
 * \addtogroup SDADC_BASIC Basic SDADC Functionality
 *
 * \brief Initialization, configuration, measurement and voltage conversion functions
 *
 * \{
 */

/**
 * \brief Initialize SDADC
 *
 * Stops ongoing sdadc operation, if any,
 * sets the SDADC control register to default values, then enables the SDADC and calls
 * the configuration function. It also disables and clears pending SDADC interrupts.
 *
 * \p cfg can be NULL - no SDADC enabling and no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 * \sa hw_sdadc_configure
 *
 */
void hw_sdadc_init(const hw_sdadc_config_t *cfg);

/**
 * \brief Initialize the SDADC to configure the audio path
 *
 * Sets the SDADC control register to support the audio path and
 * registers the function to be called upon SDADC_IRQn interrupt firing.
 * Still, hw_sdadc_start() must be called by the application to start conversion.
 *
 * \param [in] cb interrupt callback function
 * \param [in] cfg pga configuration
 *
 * \note  If cfg is NULL, this function does not configure the PGA
 *
 * \sa hw_sdadc_register_interrupt
 *
 */
void hw_sdadc_audio_init(hw_sdadc_interrupt_cb cb, const hw_sdadc_pga_config_t *cfg);


/**
 * \brief Configure SDADC
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_sdadc_configure(const hw_sdadc_config_t *cfg);

/**
 * \brief Register interrupt handler
 *
 * Interrupt is enabled after calling this function. Application is responsible for clearing
 * interrupt using hw_sdadc_clear_interrupt(). If no callback is specified interrupt is cleared by
 * driver.
 *
 * \param [in] cb callback fired on interrupt
 *
 * \sa hw_sdadc_clear_interrupt
 *
 */
void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is disabled after calling this function.
 *
 */
void hw_sdadc_unregister_interrupt(void);

/**
 * \brief Clear interrupt
 *
 * Application should call this in interrupt handler to clear interrupt.
 *
 * \sa hw_sdadc_register_interrupt
 *
 */
__STATIC_INLINE void hw_sdadc_clear_interrupt(void)
{
        REG_SETF(SDADC, SDADC_CLEAR_INT_REG, SDADC_CLR_INT, 1);
}

/**
 * \brief Stop and Wait
 *
 * Disable the continuous mode, and wait for the ADC engine to finish
 *
 * \note When DMA is in use, calling this function also disables
 *       the respective DMA channel and unregisters its callback.
 *
 */
void hw_sdadc_stop_and_wait(void);

/**
 * \brief Enable SDADC
 *
 * This function enables the SDADC. LDO, bias currents and modulator are enabled.
 * To start a conversion, the application should call hw_sdadc_start().
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE void hw_sdadc_enable(void)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 1);
        while (0 == REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_LDO_OK));       // Wait for LDO OK
}

/**
 * \brief Disable SDADC
 *
 * Application should wait for conversion to be completed before disabling SDADC. In case of
 * continuous mode, application should disable continuous mode and then wait for conversion to be
 * completed in order to have SDADC in defined state.
 *
 * \sa hw_sdadc_stop_and_wait
 *
 */
__STATIC_INLINE void hw_sdadc_disable(void)
{
        hw_sdadc_stop_and_wait();

        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_EN, 0);
}

/**
 * \brief Get the enable status of the SDADC
 *
 * \return SDADC enable status
 *
 */
__STATIC_INLINE bool hw_sdadc_is_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_EN);
}

/**
 * \brief Start conversion
 *
 * Application should not call this function while conversion is still in progress.
 *
 * \sa hw_sdadc_in_progress
 *
 */
void hw_sdadc_start(void);
/**
 * \brief Check if conversion is in progress
 *
 * \return conversion state
 *
 */
__STATIC_INLINE bool hw_sdadc_in_progress(void)
{
        return (!!(REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_START)));
}


/**
 * \brief Start the ADC conversion engine, providing one measurement
 *
 * \sa hw_sdadc_start
 * \sa hw_sdadc_in_progress
 *
 * \note The function polls the ADC engine waiting for the measurement to be ready.
 */
__STATIC_INLINE void hw_sdadc_adc_measure(void)
{
        ASSERT_WARNING(hw_sdadc_get_continuous() == false);
        hw_sdadc_start();
        while (hw_sdadc_in_progress());
}

/**
 * \brief Perform an ADC measurement and return the ADC value converted to mV
 *
 * \param [in] cfg sdadc configuration
 *
 * \return The ADC value converted to mV
 *
 * \sa hw_sdadc_start
 * \sa hw_sdadc_get_raw_value
 *
 */
int32_t hw_sdadc_get_voltage(const hw_sdadc_config_t *cfg);

/**
 * \brief Convert the ADC value to mV
 *
 * \param [in] cfg sdadc configuration
 * \param [in] raw adc raw value
 *
 * \return The ADC raw value converted to mV
 *
 */
int32_t hw_sdadc_convert_to_millivolt(const hw_sdadc_config_t *cfg, uint16_t raw);

/**
 * \brief Generic read function
 *        Follows \sa hw_sdadc_init() or \sa hw_sdadc_configure().
 *        According to the \sa hw_sdadc_config_t passed in the above mentioned functions, the generic read function
 *        starts the ADC engine, delivers the requested conversions and stops the ADC engine when finished.
 *        If a callback is set by the user, the function operates in interrupt mode, otherwise in blocking mode.
 *        The results are always in raw format, which means they need post-processing to be converted to something valuable.
 *        To ensure all conversions are in place, the caller may poll for the falling of the start bit via \sa hw_sdadc_in_progress.
 *        \sa hw_sdadc_get_value
 *        \sa hw_sdadc_convert_to_millivolt
 *        \sa hw_sdadc_apply_correction
 *
 * \param [in] nof_conv  number of conversions to be delivered. Must be non-zero
 * \param [out] out_buf  buffer to place the conversion results, NULL is allowed but user has to set a user callback in order to be able to fetch the converted results from the SD_ADC_RESULT_REG
 * \param [in] cb        user callback to execute when conversions are over, NULL for polling mode which blocks until conversions are over
 * \param [in] user_data parameter for callback
 *
 * \return true if conversions have started, false otherwise
 *
 * \note Interrupt mode can operate without an output buffer but never without a user callback.
 *
 * \note DMA mode can operate without a callback but never without an output buffer.
 *
 * \note If in \sa hw_sdadc_config_t the \sa dma_setup section is valid, the converted results are transfered through DMA to the requested buffer.
 *       In this case the ADC interrupt in M33 is bypassed, unless there is deliberate extra handling by the user outside this function.
 *       At any given point, calling \sa hw_sdadc_abort_read will abandon the converting process, executing the user callback passed as argument.
 *
 */
bool hw_sdadc_read(uint32_t nof_conv, uint16_t *out_buf, hw_sdadc_read_cb cb, void *user_data);

/**
 * \brief Stop conversions
 *
 * Application can call this function to abort an ongoing read operation.
 * It is applicable only when the ADC operates either in interrupt or DMA mode.
 *
 * \sa hw_sdadc_read
 *
 */
void hw_sdadc_abort_read(void);

/**
 * \brief Store external reference voltage calibration values
 *
 * \param [in] gain    gain correction value
 * \param [in] offset  offset correction value
 *
 * \note In case of external VREF, this function must be called before hw_sdadc_configure().
 *
 */
void hw_sdadc_store_ext_ref_calibration_values(int16_t gain, int16_t offset);

/**
 * \}
 */

#endif /* dg_configUSE_HW_SDADC */


#endif /* HW_SDADC_H_ */
/**
 * \}
 * \}
 */
