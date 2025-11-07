/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SDADC_ADAPTER SDADC Adapter
 *
 * \brief Sigma Delta Analog-Digital Converter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_sdadc.h
 *
 * @brief SDADC adapter API
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

#ifndef AD_SDADC_H_
#define AD_SDADC_H_

#if dg_configSDADC_ADAPTER

#include "ad.h"
#include "hw_gpio.h"
#include "hw_sdadc.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief SDADC Id
 */
#define HW_SDADC        ((void *)SDADC_BASE)
typedef void *HW_SDADC_ID;

/**
* \brief SDADC I/O configuration
*/
typedef struct ad_sdadc_io_conf {
        ad_io_conf_t input0;
        ad_io_conf_t input1;
} ad_sdadc_io_conf_t;

/**
 * \brief SDADC driver configuration
 *
 * Configuration of the SDADC low level driver
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef hw_sdadc_config_t ad_sdadc_driver_conf_t;

/**
 * \brief SDADC controller configuration
 *
 * Configuration of SDADC controller
 *
 * \note There may be more than one controller configurations needed (e.g DMA)
 *
 */
typedef struct ad_sdadc_controller_conf {
        const HW_SDADC_ID                id;    /**< Controller instance*/
        const ad_sdadc_io_conf_t        *io;    /**< IO configuration*/
        const ad_sdadc_driver_conf_t    *drv;   /**< Driver configuration*/
} ad_sdadc_controller_conf_t;

 /**
 * \brief SDADC Handle returned by ad_sdadc_open()
 */
typedef void *ad_sdadc_handle_t;

/**
 * \brief Asynchronous callback function
 *
 * \param [in] user_data   pointer to user data
 * \param [in] conversions number of remaining conversions
 */
typedef void (*ad_sdadc_user_cb)(void *user_data, uint32_t conversions);

/**
 * \brief Error Codes
 *
 */
typedef enum {
        AD_SDADC_ERROR_NONE                   =  0,
        AD_SDADC_ERROR_HANDLE_INVALID         = -1,
        AD_SDADC_ERROR_DRIVER_CONF_INVALID    = -2,
        AD_SDADC_ERROR_DRIVER_INPUT_INVALID   = -3,
        AD_SDADC_ERROR_DRIVER_MODE_INVALID    = -4,
        AD_SDADC_ERROR_DRIVER_UNINITIALIZED   = -5,
        AD_SDADC_ERROR_IO_CONF_INVALID        = -6,
        AD_SDADC_ERROR_CB_INVALID             = -7,
        AD_SDADC_ERROR_READ_IN_PROGRESS       = -8,
        AD_SDADC_ERROR_CANNOT_ACQUIRE         = -9,
        AD_SDADC_ERROR_ID_INVALID             = -10,
        AD_SDADC_ERROR_IO_CFG_INVALID         = -11,
        AD_SDADC_ERROR_OTHER                  = -12,
} AD_SDADC_ERROR;

/**
 * \brief Initialize SDADC adapter and some required variables
 *
 * \note: It should ONLY be called by the system.
 *
 */
void ad_sdadc_init(void);

/**
 * \brief Open SDADC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 * \param [in] conf controller configuration
 *
 * \return >0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_sdadc_handle_t ad_sdadc_open(const ad_sdadc_controller_conf_t *conf);

/**
 * \brief Reconfigure SDADC controller
 *
 * This function will apply a new SDADC driver configuration.
 *
 * \param [in] handle handle returned from ad_sdadc_open()
 * \param [in] drv    new driver configuration
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_reconfig(const ad_sdadc_handle_t handle, const ad_sdadc_driver_conf_t *drv);

/**
 * \brief Close SDADC controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_xxx_open())
 * - If DMA configuration has changed returns error
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_sdadc_open()
 * \param [in] forced force adapter closing
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_close(const ad_sdadc_handle_t handle, bool forced);

/**
 * \brief Initialize controller pins to on / off io configuration
 *
 * This function should be called for setting pins to the correct level before external
 * devices are powered up (e.g on system init). It does not need to be called before every
 * ad_sdadc_open() call.
 *
 * \param [in] id         controller instance
 * \param [in] io         controller io configuration
 * \param [in] state      on/off io configuration
 *
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_io_config(const HW_SDADC_ID id, const ad_sdadc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Read asynchronously nof_conv conversions from the selected source
 *
 * This function starts asynchronous measurement read. Non blocking read operation
 *
 * \param [in]  handle           handle to SDADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 * \param [in]  read_async_cb    user callback fired after read operation completes
 * \param [in]  user_data        pointer to user data passed to callback
 *
 * \sa ad_sdadc_open()
 * \sa ad_sdadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_sdadc_read_async(const ad_sdadc_handle_t handle, uint32_t nof_conv, uint16_t *outbuf, ad_sdadc_user_cb read_async_cb, void *user_data);

/**
 * \brief Read synchronously nof_conv conversions from the selected source
 *
 * This function starts synchronous measurement read. The caller task will block until the resource becomes available.
 * Upon the resource acquisition, ΣΔ start conversion(s) using the interrupt mode.
 *
 * \param [in]  handle           handle to SDADC source
 * \param [in]  nof_conv         number of conversions to be delivered. Must be non-zero
 * \param [out] outbuf           pointer to conversion results buffer. Output buffer contains raw values
 *
 * \sa ad_sdadc_open()
 * \sa ad_sdadc_close()
 *
 * \return 0 on success, negative value on error
 *
 */
int ad_sdadc_read(const ad_sdadc_handle_t handle, uint32_t nof_conv, uint16_t *outbuf);

/**
 * \brief Convert raw value read from SDADC to voltage in mV.
 *        The same configuration which was used to obtain the adc_value
 *        is needed for the conversion.
 *
 * \param [in] conf        controller configuration
 * \param [in] raw_value   value returned from ad_sdadc_read()/ad_sdadc_read_async()
 *
 * \return voltage in mV
 *
 */
__STATIC_FORCEINLINE int32_t ad_sdadc_conv_to_mvolt(const ad_sdadc_controller_conf_t *conf, uint32_t raw_value)
{
        return hw_sdadc_convert_to_millivolt(conf->drv, raw_value);
}

/**
 * \brief Store external reference voltage calibration values
 *
 * \param [in] gain    gain correction value
 * \param [in] offset  offset correction value
 *
 * \note In case of external VREF, this function must be called before ad_sdadc_open().
 *
 */
__STATIC_FORCEINLINE void ad_sdadc_store_ext_ref_calibration_values(int16_t gain, int16_t offset)
{
        hw_sdadc_store_ext_ref_calibration_values(gain, offset);
}

#ifdef __cplusplus
}
#endif

#endif /* dg_configSDADC_ADAPTER */

#endif /* AD_SDADC_H_ */

/**
 * \}
 * \}
 */
