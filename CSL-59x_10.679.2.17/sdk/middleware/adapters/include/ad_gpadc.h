/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup GPADC_ADAPTER GPADC Adapter
 *
 * \brief General Purpose Analog-Digital Converter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.h
 *
 * @brief GPADC adapter API
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

#if (dg_configGPADC_ADAPTER == 1)

#ifndef AD_GPADC_H_
#define AD_GPADC_H_

#include "ad.h"
#include "hw_gpadc.h"
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_GPADC_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether GPADC asynchronous transaction API will be used
 *
 * If the API is not to be used, setting this macro to 0 will save retention RAM,
 * namely the callback pointer and its argument.
 */
#ifndef CONFIG_GPADC_USE_ASYNC_TRANSACTIONS
#define CONFIG_GPADC_USE_ASYNC_TRANSACTIONS     (1)
#endif

/**
 * \def CONFIG_GPADC_USE_SYNC_TRANSACTIONS
 *
 * \brief Controls whether GPADC synchronous transaction API will be used
 *
 * \warning If an application based on FreeRTOS employs the RCX as a low power clock or uses BLE,
 *          the sys_adc API is enabled forcefully, because is essential for critical tasks, such as
 *          RCX and RF calibration. In that case, the synched GPADC transactions feature is enabled
 *          as well and cannot be disabled by the application, because is mandatory to perform the
 *          aforementioned critical actions.
 */
#if dg_configUSE_SYS_ADC
#define CONFIG_GPADC_USE_SYNC_TRANSACTIONS      (1)
#else
#ifndef CONFIG_GPADC_USE_SYNC_TRANSACTIONS
#define CONFIG_GPADC_USE_SYNC_TRANSACTIONS      (1)
#endif
#endif

/**
 * \brief GPADC I/O configuration
 *
 * GPADC I/O configuration
 */

typedef struct ad_gpadc_io_conf {
        ad_io_conf_t input0;                    /**< I/O pin for POSITIVE Input */
        ad_io_conf_t input1;                    /**< I/O pin for NEGATIVE Input */
} ad_gpadc_io_conf_t;

/**
 * \brief GPADC driver configuration
 *
 * Configuration of GPADC low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef hw_gpadc_config_t ad_gpadc_driver_conf_t;

/**
 * \brief GPADC controller instance
 */
#define HW_GPADC_1                      ((void *)GPADC_BASE)

/**
 * \brief GPADC controller configuration
 *
 * Configuration of GPADC controller
 *
 */
typedef struct ad_gpadc_controller_conf {
        const HW_GPADC_ID               id;        /**< Controller instance*/
        const ad_gpadc_io_conf_t       *io;        /**< IO configuration*/
        const ad_gpadc_driver_conf_t   *drv;       /**< Driver configuration*/
} ad_gpadc_controller_conf_t;

/**
 * \brief GPADC Handle returned by ad_gpadc_open()
 */
typedef void *ad_gpadc_handle_t;

/**
 * \brief enum with return values of API calls
 */
typedef enum {
        AD_GPADC_ERROR_NONE                    =  0,
        AD_GPADC_ERROR_HANDLE_INVALID          = -1,
        AD_GPADC_ERROR_CHANGE_NOT_ALLOWED      = -2,
        AD_GPADC_ERROR_ADAPTER_NOT_OPEN        = -3,
        AD_GPADC_ERROR_CONFIG_INVALID          = -4,
        AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS  = -5,
        AD_GPADC_ERROR_TIMEOUT                 = -6,
        AD_GPADC_ERROR_OTHER                   = -7,
        AD_GPADC_ERROR_IO_CFG_INVALID          = -8,
} AD_GPADC_ERROR;

/**
 * \brief GPADC adapter callback function
 *
 * \param [in] user_data   pointer to user data
 * \param [in] value       number of remaining conversions
 */
typedef void (*ad_gpadc_user_cb)(void *user_data, int value);

/**
 * \brief Initialize GPADC adapter and some required variables
 *
 * \warning     Do not call this function directly. It is called
 *              automatically during power manager initialization.
 *
 */
void ad_gpadc_init(void);

#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 1)
/**
 * \brief Read synchronously nof_conv conversions from the selected source
 *
 * This function starts synchronous measurement read. Blocking read  - caller task will block until the resource becomes available
 *
 * \param [in]  handle           handle to GPADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_gpadc_read_nof_conv(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf);
#endif /* CONFIG_GPADC_USE_SYNC_TRANSACTIONS */

#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)
/**
 * \brief Read asynchronously nof_conv conversions from the selected source
 *
 * This function starts asynchronous measurement read. Non blocking read operation
 *
 * \param [in]  handle           handle to GPADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 * \param [in]  read_async_cb    user callback fired after read operation completes
 * \param [in]  user_data        pointer to user data passed to callback
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_gpadc_read_nof_conv_async(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf, ad_gpadc_user_cb read_async_cb, void *user_data);
#endif

/**
 * \brief Return maximum value that can be read for ADC source
 *
 * A GPADC raw value can have 10 to 16 valid bits (left aligned) depending on
 * oversampling specified in source description. This function will return value
 * 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF depending on oversampling,
 * offering a right-aligned representation for the maximum value
 * that the GPADC can return (i.e. when the measured voltage equals to Vref).
 *
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return value 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF
 *
 */
uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv);

/**
 * \brief Open GPADC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf controller configuration
 *
 * \return >0: pointer to adapter instance - should be used in subsequent API calls, <0: error code
 *
 * \note The function will block until it acquires all controller resources
 */
ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf);

/**
 * \brief Reconfigure GPADC controller
 *
 * This function will apply a new GPADC driver configuration.
 *
 * \param [in] p     pointer returned from ad_gpadc_open()
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return 0: success, <0: error code
 */
int ad_gpadc_reconfig(const ad_gpadc_handle_t p, const ad_gpadc_driver_conf_t *drv);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_gpadc_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_gpadc_io_config (const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Close GPADC controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_gpadc_open())
 * - Releases the controller resources
 *
 * \param [in] p        pointer returned from ad_gpadc_open()
 * \param [in] force    force close even if an async read is pending
 *
* \return 0: success, <0: error code
 */
int ad_gpadc_close(ad_gpadc_handle_t p, bool force);

/**
 * \brief Convert raw value read from GPADC to temperature value in hundredths of degree Celsius.
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   raw value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return value of temperature in hundredths of degree Celsius
 *
 */
__STATIC_FORCEINLINE int ad_gpadc_conv_to_temp_x100(const ad_gpadc_driver_conf_t *drv, uint16_t raw_value)
{
        return hw_gpadc_convert_to_celsius_x100_util(drv, raw_value);
}

/**
 * \brief Convert raw value read from GPADC to battery voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return battery voltage in mV
 *
 */
uint16_t ad_gpadc_conv_raw_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value);

/**
 * \brief Convert raw value read from GPADC to voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] drv         GPADC driver configuration structure, NULL to use the current ADC settings
 * \param [in] raw_value   value returned from ad_gpadc_read_nof_conv() or ad_gpadc_read_nof_conv_async()
 *
 * \return voltage in mV
 *
 */
int ad_gpadc_conv_to_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value);

#ifdef __cplusplus
}
#endif

#endif /* AD_GPADC_H_ */

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 */
