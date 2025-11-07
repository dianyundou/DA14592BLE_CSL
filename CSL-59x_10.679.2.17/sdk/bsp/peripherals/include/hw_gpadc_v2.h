/**
 ****************************************************************************************
 *
 * @file hw_gpadc_v2.h
 *
 * @brief Definition of API for the GPADC Low Level Driver
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
 ****************************************************************************************
 */

#ifndef HW_GPADC_V2_H
#define HW_GPADC_V2_H


#if dg_configUSE_HW_GPADC

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/***************************************************************************
 *********    Macro, type and data-structure definitions     ***************
 ***************************************************************************/

/**
 * \addtogroup GPADC_DATA
 *
 * \{
 */

/**
 * \brief Recommended sample time setting for accurate temperature measurements with DIE_TEMP
 *
 */
#define HW_GPADC_DIE_TEMP_SMPL_TIME             0x04

/**
 * \brief Delay for enabling the ADC after enabling the LDO when ADC input is the temperature sensor
 *
 * HW_GPADC_DIE_TEMP_INIT_DELAY * 4 * ADC_CLK period should be > 25usec
 *
 */
#define HW_GPADC_DIE_TEMP_INIT_DELAY            0x68  /* 26 usec with a clock speed of (DivN_clk / 2) */

/**
 * \brief GPADC input voltages
 *
 */
typedef enum {
        HW_GPADC_INPUT_VOLTAGE_UP_TO_0V9 = 0,   /**< input voltages up to 0.9 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_1V8 = 1,   /**< input voltages up to 1.8 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_2V7 = 2,   /**< input voltages up to 2.7 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_3V6 = 3,   /**< input voltages up to 3.6 V are allowed */
} HW_GPADC_MAX_INPUT_VOLTAGE;

/**
 * \brief GPADC Reference Voltage Level
 *
 */
#define HW_GPADC_VREF_MILLIVOLT                 (900)

/**
 * \brief Store delay
 *
 * \note Values 1-3 are reserved
 *
 */
typedef enum {
        HW_GPADC_STORE_DEL_0         = 0x0,     /**< Data is stored after handshake synchronization */
        HW_GPADC_STORE_DEL_2_CYCLES = 0x1,      /**< Data is stored 2 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_3_CYCLES = 0x2,      /**< Data is stored 3 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_4_CYCLES = 0x3,      /**< Data is stored 4 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_5_CYCLES = 0x4,     /**< Data is stored 5 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_6_CYCLES = 0x5,     /**< Data is stored 6 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_7_CYCLES = 0x6,     /**< Data is stored 7 ADC_CLK cycles after internal start trigger */
        HW_GPADC_STORE_DEL_8_CYCLES = 0x7,     /**< Data is stored 8 ADC_CLK cycles after internal start trigger */
} HW_GPADC_STORE_DELAY;

/*
 *  ADC input to GPIO pin mapping
 */
typedef enum {
        HW_GPADC_INPUT_ADC0 = 0,
        HW_GPADC_INPUT_ADC1,
        HW_GPADC_INPUT_ADC2,
        HW_GPADC_INPUT_ADC3,
        HW_GPADC_INPUT_ADC4 = 0x0A,
        HW_GPADC_INPUT_ADC5 = 0x0B,
        HW_GPADC_INPUT_ADC6 = 0x0C,
        HW_GPADC_INPUT_ADC7 = 0x0D,
        HW_GPADC_INPUT_RES1 = 0x0E,
        HW_GPADC_INPUT_RES2 = 0x0F,
} HW_GPADC_GPIO_INPUT;

/**
 * \brief ADC input - Positive side
 *
 */
typedef enum {
        HW_GPADC_INP_P1_0  = HW_GPADC_INPUT_ADC0,       /**< GPIO 1.0 */
        HW_GPADC_INP_P1_1  = HW_GPADC_INPUT_ADC1,       /**< GPIO 1.1 */
        HW_GPADC_INP_P1_2  = HW_GPADC_INPUT_ADC2,       /**< GPIO 1.2 */
        HW_GPADC_INP_P0_10 = HW_GPADC_INPUT_ADC3,       /**< GPIO 0.10 */
        HW_GPADC_INP_DIE_TEMP = 4,                      /**< temperature sensor
                                                             Used jointly with DIE_TEMP=1.
                                                             MUST wait 25usec before measurement */
        HW_GPADC_INP_VDCDC = 5,                         /**< DCDC voltage level */
        HW_GPADC_INP_VBAT = 6,                          /**< Battery voltage level */
        HW_GPADC_INP_VDDD = 7,                          /**< VDD supply of the ADC circuit */
        HW_GPADC_INP_VSSA = 8,                          /**< VSSA */
        HW_GPADC_INP_VDDIO = 9,                         /**< VDDIO */
        HW_GPADC_INP_P1_5  = HW_GPADC_INPUT_ADC4,       /**< GPIO 1.5 */
        HW_GPADC_INP_P1_6  = HW_GPADC_INPUT_ADC5,       /**< GPIO 1.6 */
        HW_GPADC_INP_P1_9  = HW_GPADC_INPUT_ADC6,       /**< GPIO 1.9 */
        HW_GPADC_INP_P1_11 = HW_GPADC_INPUT_ADC7,       /**< GPIO 1.11 */
} HW_GPADC_INPUT_POSITIVE;

/**
 * \brief ADC input - Negative side
 *
 */
typedef enum {
        HW_GPADC_INN_P1_0  = HW_GPADC_INPUT_ADC0,       /**< GPIO 1.0 */
        HW_GPADC_INN_P1_1  = HW_GPADC_INPUT_ADC1,       /**< GPIO 1.1 */
        HW_GPADC_INN_P1_2  = HW_GPADC_INPUT_ADC2,       /**< GPIO 1.2 */
        HW_GPADC_INN_P0_10 = HW_GPADC_INPUT_ADC3,       /**< GPIO 0.10 */
        HW_GPADC_INN_P1_5  = HW_GPADC_INPUT_ADC4,       /**< GPIO 1.5 */
        HW_GPADC_INN_P1_6  = HW_GPADC_INPUT_ADC5,       /**< GPIO 1.6 */
        HW_GPADC_INN_P1_9  = HW_GPADC_INPUT_ADC6,       /**< GPIO 1.9 */
        HW_GPADC_INN_P1_11 = HW_GPADC_INPUT_ADC7,       /**< GPIO 1.11 */
} HW_GPADC_INPUT_NEGATIVE;

/**
 * \brief ADC configuration
 *
 */
typedef struct {
        HW_GPADC_INPUT_MODE        input_mode;          /**< input mode */
        HW_GPADC_INPUT_POSITIVE    positive;            /**< positive channel */
        HW_GPADC_INPUT_NEGATIVE    negative;            /**< negative channel */
        uint8_t                    sample_time;         /**< sample time, range: 0-15, time = (sample_time x 8) ADC_CLK cycles */
        bool                       continuous;          /**< continuous mode state */
        uint8_t                    interval;            /**< interval between conversions in continuous mode */
        HW_GPADC_MAX_INPUT_VOLTAGE input_attenuator;    /**< input attenuator regulates the maximum measured input voltage */
        bool                       chopping;            /**< chopping state */
        HW_GPADC_OVERSAMPLING      oversampling;        /**< oversampling rate */
#if HW_GPADC_DMA_SUPPORT
        gpadc_dma_cfg              *dma_setup;          /**< DMA configuration - NULL to disable */
#endif
} hw_gpadc_config_t;

/**
 * \}
 */

/***************************************************************************
 ****************      GP_ADC configuration functions    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_CONFIG_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Set the delay required to enable the ADC_LDO.
 *        0: Not allowed
 *        1: 4x ADC_CLK period.
 *        n: n*4x ADC_CLK period.
 *
 * param [in] LDO enable delay
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_delay(uint32_t delay)
{
        /* Zero delay is not allowed by the h/w specification */
        ASSERT_ERROR(delay != 0);
        REG_SETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_EN_DEL, delay);
}

/**
 * \brief Set STORE_DEL field
 *
 * 0: Data is stored after handshake synchronization
 * 1-3: Reserved
 * 4: Data is stored 5 ADC_CLK cycles after internal start trigger
 * 7: Data is stored 8 ADC_CLK cycles after internal start trigger
 *
 * \param [in] delay store delay setting
 *
 * \note The application should be very careful with this bitfield as it could easily
 * read outdated conversion data if the value is set too optimistic.
 * Setting it too pessimistic is only slowing down the conversion time.
 * The zero default value is strongly recommended to be used.
 */
 __STATIC_INLINE void hw_gpadc_set_store_delay(HW_GPADC_STORE_DELAY delay)
 {
         REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_STORE_DEL, delay);
 }

/**
 * \brief Set positive input channel
 *
 * \param [in] channel positive input channel
 *
 */
__STATIC_INLINE void hw_gpadc_set_positive(HW_GPADC_INPUT_POSITIVE channel)
{
        REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P, channel);
}

/**
 * \brief Get the current positive input channel
 *
 * \return positive input channel
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_POSITIVE hw_gpadc_get_positive(void)
{
        return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_P);
}

/**
 * \brief Set negative input channel
 *
 * \param [in] channel negative input channel
 *
 */
__STATIC_INLINE void hw_gpadc_set_negative(HW_GPADC_INPUT_NEGATIVE channel)
{
        REG_SETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_N, channel);
}

/**
 * \brief Get the current negative input channel
 *
 * \return negative input channel
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_NEGATIVE hw_gpadc_get_negative(void)
{
        return REG_GETF(GPADC, GP_ADC_SEL_REG, GP_ADC_SEL_N);
}

/**
 * \brief Set state of input attenuator
 *
 * Enabling the internal attenuator scales input voltage, increasing the effective input
 * scale from 0-1.2V to 0-3.6V in single ended mode or from -1.2-1.2V to -3.6-3.6V in differential
 * mode.
 *
 * \param [in] vmax attenuator state
 *
 */
__STATIC_INLINE void hw_gpadc_set_input_attenuator_state(HW_GPADC_MAX_INPUT_VOLTAGE vmax)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN, vmax);
}

/**
 * \brief Get the current state of input attenuator
 *
 * \return attenuator state
 *
 */
__STATIC_INLINE HW_GPADC_MAX_INPUT_VOLTAGE hw_gpadc_get_input_attenuator_state(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN);
}

/**
 * \brief Set sample time
 *
 * Sample time is \p mult x 8 clock cycles or 1 clock cycle when \p mult is 0. Valid values are
 * 0-15.
 *
 * \param [in] mult multiplier
 *
 */
__STATIC_INLINE void hw_gpadc_set_sample_time(uint8_t mult)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME, mult);
}

/**
 * \brief Get the current sample time.
 *        The sample time is calculated, based on this register field value.
 *
 * \return multiplier (sample time = multiplier x 8 x ADC_CLK)
 *
 * \sa hw_gpadc_set_sample_time
 */
__STATIC_INLINE uint8_t hw_gpadc_get_sample_time(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME);
}

/**
 * \brief Set DIE_TEMP_EN field
 *
 * Enables the die-temperature sensor. Output can be measured on GPADC input 4.
 *
 * \param [in] enabled enable/disable the die-temperature sensor
 *
 * \sa HW_GPADC_INPUT_POSITIVE
 */
__STATIC_INLINE void hw_gpadc_set_die_temp(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, DIE_TEMP_EN, !!enabled);
}

/**
 * \brief Get the current status of the die-temperature sensor. Output can be measured on GPADC input 4.
 *
 * \return current die-temperature sensor status
 *
 * \sa HW_GPADC_INPUT_POSITIVE
 *
 */
__STATIC_INLINE bool hw_gpadc_get_die_temp(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, DIE_TEMP_EN);
}

/**
 * \brief Set the mode of bandgap reference
 *
 * 0: GPADC LDO tracking bandgap reference (default)
 * 1: GPADC LDO hold sampled bandgap reference
 *
 * \param [in] enabled ldo bandgap reference mode
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_hold(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_LDO_HOLD, !!enabled);
}

/**
 * \brief Get the current mode of bandgap reference
 *
 * \return current ldo bandgap reference mode
 *
 * \sa hw_gpadc_set_ldo_hold
 *
 */
__STATIC_INLINE bool hw_gpadc_get_ldo_hold(void)
{
        return (bool) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_LDO_HOLD);
}

/**
 * \}
 */

/***************************************************************************
 ****************    Basic functionality of the GPADC    *******************
 ***************************************************************************/

/**
 * \addtogroup GPADC_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Get the measured voltage in mVolt
 *
 * \return voltage (mVolt)
 *
 * \sa hw_gpadc_get_value
 *
 */
int16_t hw_gpadc_get_voltage(void);

/**
 * \}
 */

/***************************************************************************
 ******************      TEMPERATURE SENSOR functions  *********************
 ***************************************************************************/

/**
 * \addtogroup GPADC_TEMPSENS_FUNCTIONS
 *
 * \{
 */

/**
 * \brief Convert a 16-bit, left-aligned, raw value to temperature.
 *        For accurate conversions using this function the ADC
 *        should operate in the following configuration:
 *
 *        Positive and negative offset registers = Default (0x200 uncalibrated),
 *        SampleTime = 0x02,
 *        Oversampling = 64 Samples,
 *        Chopping = Enabled and
 *        Attenuator = Disabled.
 *
 * \param [in] cfg      GPADC configuration, NULL to use the current ADC settings
 * \param [in] raw_val  digital GPADC value
 *
 * \return temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \sa hw_gpadc_config_t
 * \sa hw_gpadc_set_offset_positive
 * \sa hw_gpadc_set_offset_negative
 */
int16_t hw_gpadc_convert_to_celsius_x100_util(const hw_gpadc_config_t *cfg, uint16_t raw_val);

/**
 * \brief Convert a temperature value to raw GPADC value.
 *
 * \param [in] cfg         GPADC configuration, NULL to use the current ADC settings
 * \param [in] temperature temperature in hundredths of Celsius degrees (ex. 2540 = 25.4 C)
 *
 * \return 16-bit left-aligned ADC value (raw)
 */
uint16_t hw_gpadc_convert_celsius_x100_to_raw_val_util(const hw_gpadc_config_t *cfg, int16_t temperature);

/**
 * \brief Store temperature calibration point at ambient temperature
 *
 * \param [in] raw_val ADC calibration value in 16-bit resolution
 * \param [in] temp    temperature in (Celsius degrees x 100)
 *
 */
void hw_gpadc_store_ambient_calibration_point(uint16_t raw_val, int16_t temp);

/**
 * \}
 */

#endif /* dg_configUSE_HW_GPADC */
#endif /* HW_GPADC_V2_H */
