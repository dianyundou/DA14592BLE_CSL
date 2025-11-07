/**
 ****************************************************************************************
 *
 * @file ad_gpadc.c
 *
 * @brief GPADC adapter implementation
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

#include <stdarg.h>
#include <stdint.h>
#include "ad.h"
#include "ad_gpadc.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "hw_gpadc.h"
#include "sdk_defs.h"
#include "hw_gpio.h"
#include "hw_sys.h"
#include "sys_bsr.h"


#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 0) && (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 0)
#error "At least one macro CONFIG_GPADC_USE_SYNC_TRANSACTIONS or CONFIG_GPADC_USE_ASYNC_TRANSACTIONS must be set"
#endif

/**
 * \def CONFIG_AD_GPADC_LOCKING
 *
 * \brief Controls whether GPADC adapter resource locking is enabled
 *
 * By default, the GPADC adapter internally handles concurrent accesses to GPADC by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_GPADC_LOCKING
# define CONFIG_AD_GPADC_LOCKING        ( 1 )
#endif /* CONFIG_AD_GPADC_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_GPADC_LOCKING == 1)
# define GPADC_MUTEX_CREATE(mutex)                      do { \
                                                                OS_ASSERT((mutex) == NULL); \
                                                                OS_MUTEX_CREATE(mutex); \
                                                                OS_ASSERT(mutex); \
                                                        } while (0)

# define GPADC_MUTEX_GET(mutex)                         do { \
                                                                OS_ASSERT(mutex); \
                                                                OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                        } while (0)

# define GPADC_MUTEX_GET_TIMEOUT(mutex, timeout)        ({ \
                                                                OS_BASE_TYPE ret; \
                                                                OS_ASSERT(mutex); \
                                                                ret = OS_MUTEX_GET((mutex), (timeout)); \
                                                                ret; \
                                                        })

# define GPADC_MUTEX_PUT(mutex)         OS_MUTEX_PUT(mutex)

# define GPADC_RES_ACQUIRE(timeout)     ad_gpadc_acquire(timeout)
# define GPADC_RES_RELEASE()            ad_gpadc_release()
#else /* CONFIG_AD_GPADC_LOCKING == 0 */
# define GPADC_MUTEX_CREATE(mutex)      do {} while (0)
# define GPADC_MUTEX_GET(mutex)         do {} while (0)
# define GPADC_MUTEX_PUT(mutex)         do {} while (0)
# define GPADC_RES_ACQUIRE(timeout)     ({ true; })
# define GPADC_RES_RELEASE()            do {} while (0)

#endif /* CONFIG_AD_GPADC_LOCKING */

#define AD_GPADC_ASSERT_HANDLE_VALID(__handle)                          \
        OS_ASSERT(__handle == dynamic_data.handle && __handle != NULL); \
        if (__handle != dynamic_data.handle || __handle == NULL) {      \
                return AD_GPADC_ERROR_HANDLE_INVALID;                   \
        }


typedef enum {
        AD_GPADC_PAD_LATCHES_OP_ENABLE,
        AD_GPADC_PAD_LATCHES_OP_DISABLE,
} AD_GPADC_PAD_LATCHES_OP;

/**
 * \brief GPADC adapter (internal) data
 *
 * Data structure of GPADC controller
 *
 */
typedef struct ad_gpadc_data {
        /**< GPADC controller current configuration */
        ad_gpadc_controller_conf_t      *conf;                  /**< Current GPADC configuration */
#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)
        ad_gpadc_user_cb                read_cb;                /**< User function to call after asynchronous read finishes */
        void                            *user_data;             /**< User data for callback */
#endif
        ad_gpadc_handle_t               handle;                 /**< The handle for the active controller */
        /**< Internal data */
#if (CONFIG_AD_GPADC_LOCKING == 1)
        OS_MUTEX                        busy;                   /**< Semaphore for thread safety */
#endif
        OS_EVENT                        sync_event;             /**< Semaphore for thread safety */
        bool                            read_in_progress;       /**< Number of source_acquire calls */
        bool                            latch_input0;           /**< flag to indicate if input 0 needs latching */
        bool                            latch_input1;           /**< flag to indicate if input 1 needs latching */
} ad_gpadc_data;

__RETAINED static ad_gpadc_data dynamic_data;

#include <sys_power_mgr.h>

static void ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP pad_latches_op, const ad_gpadc_io_conf_t * io, bool use_mutex)
{
        if (io == NULL) {
                return;
        }

        if (use_mutex) {
                GPADC_MUTEX_GET(dynamic_data.busy);
        }

        if (pad_latches_op == AD_GPADC_PAD_LATCHES_OP_ENABLE) {
                if (dynamic_data.latch_input0) {
                        hw_gpio_pad_latch_enable(io->input0.port, io->input0.pin);
                }

                if (dynamic_data.latch_input1) {
                        hw_gpio_pad_latch_enable(io->input1.port, io->input1.pin);
                }
        } else {
                if (dynamic_data.latch_input0) {
                        hw_gpio_pad_latch_disable(io->input0.port, io->input0.pin);
                }

                if (dynamic_data.latch_input1) {
                        hw_gpio_pad_latch_disable(io->input1.port, io->input1.pin);
                }
        }

        if (use_mutex) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
        }
}

static void ad_gpadc_gpio_io_config (const ad_io_conf_t * gpio, AD_IO_CONF_STATE state)
{
        if (gpio == NULL) {
                return;
        }

        if (state == AD_IO_CONF_ON) {
                if (gpio->pin != HW_GPIO_PIN_NONE && gpio->port != HW_GPIO_PORT_NONE) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->on.mode, gpio->on.function);
                }

                if (gpio->on.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        } else {
                if (gpio->pin != HW_GPIO_PIN_NONE && gpio->port != HW_GPIO_PORT_NONE) {
                        hw_gpio_set_pin_function(gpio->port, gpio->pin, gpio->off.mode, gpio->off.function);
                }

                if (gpio->off.high) {
                        hw_gpio_set_active(gpio->port, gpio->pin);
                } else {
                        hw_gpio_set_inactive(gpio->port, gpio->pin);
                }
        }
}

int ad_gpadc_io_config (const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        if (io == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        hw_sys_pd_com_enable();
        ad_gpadc_gpio_io_config(&io->input0, state);
        ad_gpadc_gpio_io_config(&io->input1, state);
        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_ENABLE, io, true);
        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_DISABLE, io, true);
        hw_sys_pd_com_disable();
        return AD_GPADC_ERROR_NONE;
}

void ad_gpadc_init(void)
{
        dynamic_data.conf = NULL;
        GPADC_MUTEX_CREATE(dynamic_data.busy);
        OS_EVENT_CREATE(dynamic_data.sync_event);
}

#if (CONFIG_AD_GPADC_LOCKING == 1)
static bool ad_gpadc_acquire(uint32_t timeout)
{
        if (resource_acquire(RES_MASK(RES_ID_GPADC), timeout)) {
                return true;
        }

        return false;
}

static void ad_gpadc_release(void)
{
        resource_release(RES_MASK(RES_ID_GPADC));
}
#endif /* CONFIG_AD_GPADC_LOCKING */

static bool validate_drv_config(const ad_gpadc_controller_conf_t *conf)
{
        if (conf->drv->input_mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                switch (conf->drv->positive) {
                case HW_GPADC_INPUT_ADC0:
                case HW_GPADC_INPUT_ADC1:
                case HW_GPADC_INPUT_ADC2:
                case HW_GPADC_INPUT_ADC3:
                        if (!conf->io) {
                                /* Mandatory GPIO configuration for the above inputs */
                                return false;
                        }
                        dynamic_data.latch_input0 = true;
                        dynamic_data.latch_input1 = false;
                        ad_gpadc_gpio_io_config(&conf->io->input0, AD_IO_CONF_ON);
                        ad_gpadc_gpio_io_config(&conf->io->input1, AD_IO_CONF_OFF);
                        break;
                        case HW_GPADC_INP_DIE_TEMP:
                        case HW_GPADC_INP_VBAT:
                        case HW_GPADC_INP_VDCDC:
                        case HW_GPADC_INP_VSSA:
                        case HW_GPADC_INP_VDDIO:
                        dynamic_data.latch_input0 = false;
                        dynamic_data.latch_input1 = false;
                        break;
                default:
                        return false;
                }
        } else {
                /* HW_GPADC_INPUT_MODE_DIFFERENTIAL */
                if (!conf->io) {
                        /* Only GPIO configurations are valid in this mode */
                        return false;
                }
                switch (conf->drv->negative) {
                case HW_GPADC_INPUT_ADC0:
                case HW_GPADC_INPUT_ADC1:
                case HW_GPADC_INPUT_ADC2:
                case HW_GPADC_INPUT_ADC3:
                case HW_GPADC_INPUT_ADC4:
                case HW_GPADC_INPUT_ADC5:
                case HW_GPADC_INPUT_ADC6:
                case HW_GPADC_INPUT_ADC7:
                        break;
                default:
                        return false;
                }
                dynamic_data.latch_input0 = true;
                dynamic_data.latch_input1 = true;
                ad_gpadc_gpio_io_config(&conf->io->input0, AD_IO_CONF_ON);
                ad_gpadc_gpio_io_config(&conf->io->input1, AD_IO_CONF_ON);
        }
        return true;
}

static int ad_gpadc_check_and_apply_config(const ad_gpadc_controller_conf_t *conf, AD_IO_CONF_STATE onoff)
{
        if (!conf || !conf->drv) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        /* Validate input channel combinations and mark GPIO's to-be-latched */
        if (!validate_drv_config(conf)) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        /* Apply I/O configuration and latching */
        if (conf->io) {
                return ad_gpadc_io_config(conf->id, conf->io, onoff);
        }

        return AD_GPADC_ERROR_NONE;
}

int ad_gpadc_reconfig(const ad_gpadc_handle_t handle, const ad_gpadc_driver_conf_t *drv)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        if (drv == NULL) {
                return AD_GPADC_ERROR_CONFIG_INVALID;
        }

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (dynamic_data.conf == NULL) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                // Use ad_gpadc_open instead
                return AD_GPADC_ERROR_ADAPTER_NOT_OPEN;
        }

        if ((dynamic_data.conf->drv->positive != drv->positive) ||
                (dynamic_data.conf->drv->negative != drv->negative)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                // Not allowed to change input with reconfig
                return AD_GPADC_ERROR_CHANGE_NOT_ALLOWED;
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.conf->drv = (ad_gpadc_driver_conf_t *) drv;

        hw_gpadc_configure(dynamic_data.conf->drv);

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf)
{
        if (conf == NULL) {
                return NULL;
        }

        /* The driver configuration is mandatory */
        if (conf->drv == NULL) {
                return NULL;
        }

        pm_sleep_mode_request(pm_mode_idle);

        GPADC_RES_ACQUIRE(RES_WAIT_FOREVER);

        if (dynamic_data.conf != NULL) {
                //use ad_gpadc_reconfig instead
                GPADC_RES_RELEASE();
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        /* Power-up the ADC block */
        hw_sys_pd_periph_enable();
        hw_sys_pd_com_enable();

        if (ad_gpadc_check_and_apply_config(conf, AD_IO_CONF_ON) < 0) {
                /* Power-down the ADC block */
                hw_sys_pd_periph_disable();

                hw_sys_pd_com_disable();
                GPADC_RES_RELEASE();
                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        hw_gpadc_init((hw_gpadc_config_t*)conf->drv, true);

        ad_gpadc_pad_latches(AD_GPADC_PAD_LATCHES_OP_ENABLE, conf->io, true);

        dynamic_data.conf = (ad_gpadc_controller_conf_t *) conf;
        dynamic_data.handle = (ad_gpadc_handle_t *) conf;

        return dynamic_data.handle;
}

int ad_gpadc_close(ad_gpadc_handle_t handle, bool force)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle)

        OS_ENTER_CRITICAL_SECTION();
        if (dynamic_data.read_in_progress) {
                if (force) {
                        hw_gpadc_unregister_interrupt();
                        dynamic_data.read_in_progress = false;
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
                }
        }
        OS_LEAVE_CRITICAL_SECTION();

        hw_gpadc_init(NULL, false);

        /* The ad_gpadc_check_and_apply_config() has verified the I/O configuration in open(),
         * we call here the ad_gpadc_io_config() with no checks to apply the OFF configuration.
         */
        ad_gpadc_io_config(handle, dynamic_data.conf->io, AD_IO_CONF_OFF);


        dynamic_data.conf = NULL;
        dynamic_data.handle = NULL;

        /* Power-down the ADC block */
        hw_sys_pd_periph_disable();
        hw_sys_pd_com_disable();
        GPADC_RES_RELEASE();

        pm_sleep_mode_release(pm_mode_idle);

        return AD_GPADC_ERROR_NONE;
}

#if (CONFIG_GPADC_USE_ASYNC_TRANSACTIONS == 1)
static void gpadc_cb_wrapper_async(void *param, uint32_t to_go)
{
        if (!dynamic_data.read_in_progress) {
                return;
        }

        if (dynamic_data.read_cb != NULL) {
                dynamic_data.read_cb(dynamic_data.user_data, to_go);
        }

        dynamic_data.read_in_progress = false;
}

int ad_gpadc_read_nof_conv_async(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf, ad_gpadc_user_cb read_async_cb, void *user_data)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle);

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (nof_conv > 1) {
                hw_gpadc_set_continuous(true);
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        dynamic_data.read_in_progress = true;

        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        if (!hw_gpadc_read(nof_conv, outbuf, gpadc_cb_wrapper_async, NULL)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_OTHER;
        }

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}

#endif /* CONFIG_GPADC_USE_ASYNC_TRANSACTIONS */

#if (CONFIG_GPADC_USE_SYNC_TRANSACTIONS == 1)
static void gpadc_cb_wrapper_sync(void *param, uint32_t to_go)
{
        OS_EVENT_SIGNAL_FROM_ISR(dynamic_data.sync_event);
}

int ad_gpadc_read_nof_conv(const ad_gpadc_handle_t handle, int nof_conv, uint16_t *outbuf)
{
        AD_GPADC_ASSERT_HANDLE_VALID(handle);

        GPADC_MUTEX_GET(dynamic_data.busy);

        if (nof_conv > 1) {
                hw_gpadc_set_continuous(true);
        }

        if (dynamic_data.read_in_progress) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS;
        }

        if (!hw_gpadc_read(nof_conv, outbuf, gpadc_cb_wrapper_sync, NULL)) {
                GPADC_MUTEX_PUT(dynamic_data.busy);
                return AD_GPADC_ERROR_OTHER;
        }

        OS_EVENT_WAIT(dynamic_data.sync_event, OS_EVENT_FOREVER);

        GPADC_MUTEX_PUT(dynamic_data.busy);

        return AD_GPADC_ERROR_NONE;
}
#endif /* CONFIG_GPADC_USE_SYNC_TRANSACTIONS */

uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv)
{
        HW_GPADC_OVERSAMPLING ovs = drv ? drv->oversampling : hw_gpadc_get_oversampling();
        return 0xFFFF >> (HW_GPADC_UNUSED_BITS - MIN(HW_GPADC_UNUSED_BITS, ovs));
}


uint16_t ad_gpadc_conv_raw_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value)
{
        uint16_t source_max =  UINT16_MAX;
        uint16_t value =  hw_gpadc_apply_correction(drv, raw_value);
        uint32_t attn_scaler = drv ? drv->input_attenuator : hw_gpadc_get_input_attenuator_state();

        attn_scaler++;
        return ((uint32_t) HW_GPADC_VREF_MILLIVOLT * attn_scaler * value) / source_max;
}

inline int ad_gpadc_conv_to_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t raw_value)
{
        return hw_gpadc_convert_to_millivolt(drv, raw_value);
}

ADAPTER_INIT(ad_gpadc_adapter, ad_gpadc_init);

#endif /* dg_configGPADC_ADAPTER */
