/**
 ****************************************************************************************
 *
 * @file ad_sdadc.c
 *
 * @brief SDADC adapter implementation
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

#if dg_configSDADC_ADAPTER

#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "ad_sdadc.h"
#include "interrupts.h"
#include "hw_gpio.h"
#include "hw_sdadc.h"
#include "hw_sys.h"
#include "resmgmt.h"
#include "sdk_defs.h"
#include "sdk_list.h"
#include "sys_bsr.h"
#include "sys_power_mgr.h"
#if (HW_SDADC_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif

/**
 * \def CONFIG_AD_SDADC_LOCKING
 *
 * \brief Controls whether SDADC adapter resource locking is enabled
 *
 * By default, the SDADC adapter internally handles concurrent accesses to SDADC by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_SDADC_LOCKING
# define CONFIG_AD_SDADC_LOCKING        ( 1 )
#endif /* CONFIG_AD_SDADC_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_SDADC_LOCKING == 1)
# define SDADC_MUTEX_CREATE(mutex)                      do { \
                                                                OS_ASSERT((mutex) == NULL); \
                                                                OS_MUTEX_CREATE(mutex); \
                                                                OS_ASSERT(mutex); \
                                                        } while (0)

# define SDADC_MUTEX_GET(mutex)                         do { \
                                                                OS_ASSERT(mutex); \
                                                                OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                        } while (0)

# define SDADC_MUTEX_GET_TIMEOUT(mutex, timeout)        ({ \
                                                                OS_BASE_TYPE ret; \
                                                                OS_ASSERT(mutex); \
                                                                ret = OS_MUTEX_GET((mutex), (timeout)); \
                                                                ret; \
                                                        })

# define SDADC_MUTEX_PUT(mutex)         OS_MUTEX_PUT(mutex)

#if HW_SDADC_DMA_SUPPORT
# define SDADC_RES_ACQUIRE(dma_chan)    ad_sdadc_acquire((dma_chan))
# define SDADC_RES_RELEASE(dma_chan)    ad_sdadc_release((dma_chan))
#else
# define SDADC_RES_ACQUIRE()            ad_sdadc_acquire()
# define SDADC_RES_RELEASE()            ad_sdadc_release()
#endif
#else /* CONFIG_AD_SDADC_LOCKING == 0 */
# define SDADC_MUTEX_CREATE(mutex)      do {} while (0)
# define SDADC_MUTEX_GET(mutex)         do {} while (0)
# define SDADC_MUTEX_PUT(mutex)         do {} while (0)
# define SDADC_RES_ACQUIRE(timeout)     ({ true; })
# define SDADC_RES_RELEASE()            do {} while (0)

#endif /* CONFIG_AD_SDADC_LOCKING */

typedef enum {
        AD_SDADC_PAD_LATCHES_OP_ENABLE,
        AD_SDADC_PAD_LATCHES_OP_DISABLE
} AD_SDADC_PAD_LATCHES_OP;

typedef struct ad_sdadc_use_input_t {
        bool                            input0;
        bool                            input1;
} ad_sdadc_use_input_t;

/**
 * \brief SDADC adapter (internal) data
 *
 * Data structure of SDADC controller
 *
 */
typedef struct ad_sdadc_data {
        /**< SDADC controller current configuration */
        ad_sdadc_controller_conf_t      *conf;                  /**< Current SDADC configuration */
        ad_sdadc_driver_conf_t          *current_drv;           /**< Current low-level driver configuration */
        ad_sdadc_user_cb                read_cb;                /**< User function to call after asynchronous read finishes */
        void                            *user_data;             /**< User data for callback */
        ad_sdadc_handle_t               handle;                 /**< The handle for the active controller */
        /**< Internal data */
#if (CONFIG_AD_SDADC_LOCKING == 1)
        OS_MUTEX                        busy;                   /**< Semaphore for thread safety */
#endif
        OS_EVENT                        sync_event;             /**< Semaphore for thread safety */
        bool                            read_in_progress;       /**< Controller is busy reading */
        ad_sdadc_use_input_t            using;                  /**< Flags indicating if input channels are used */
} ad_sdadc_data;

__RETAINED static ad_sdadc_data dynamic_data;

#define AD_SDADC_HANDLE_IS_INVALID(_h_)      ( _h_ == NULL || dynamic_data.handle != _h_ )

static void ad_sdadc_pad_latches(AD_IO_PAD_LATCHES_OP pad_latches_op, const ad_sdadc_io_conf_t *io_conf)
{
        if (io_conf == NULL) {
                return;
        }

        if (dynamic_data.using.input0) {
                ad_io_set_pad_latch(&io_conf->input0, 1, pad_latches_op);
        }
        if (dynamic_data.using.input1) {
                ad_io_set_pad_latch(&io_conf->input1, 1, pad_latches_op);
        }
}

int ad_sdadc_io_config(const HW_SDADC_ID id, const ad_sdadc_io_conf_t *io, AD_IO_CONF_STATE state)
{
        uint8_t size;

        if (id != HW_SDADC) {
                return AD_SDADC_ERROR_ID_INVALID;
        }
        if (io == NULL) {
                return AD_SDADC_ERROR_IO_CONF_INVALID;
        }

        SDADC_MUTEX_GET(dynamic_data.busy);

        hw_sys_pd_com_enable();

        size = AD_IO_PIN_PORT_VALID(io->input1.port, io->input1.pin) ? 2 : 1;

        if (ad_io_configure(&io->input0, size, state) != AD_IO_ERROR_NONE) {
                hw_sys_pd_com_disable();
                SDADC_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_IO_CFG_INVALID;
        }

        ad_sdadc_pad_latches(AD_IO_PAD_LATCHES_OP_ENABLE, io);
        ad_sdadc_pad_latches(AD_IO_PAD_LATCHES_OP_DISABLE, io);
        hw_sys_pd_com_disable();

        SDADC_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}

void ad_sdadc_init(void)
{
        dynamic_data.conf = NULL;
        SDADC_MUTEX_CREATE(dynamic_data.busy);
        OS_EVENT_CREATE(dynamic_data.sync_event);
}

#if (CONFIG_AD_SDADC_LOCKING == 1)
#if (HW_SDADC_DMA_SUPPORT == 1)
__STATIC_INLINE void ad_sdadc_acquire(HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel < HW_DMA_CHANNEL_INVALID - 1) {
                /* make sure that the selected dma channel number is even */
                ASSERT_WARNING((dma_channel & 0x1) == 0);
                resource_acquire(RES_MASK(RES_ID_SDADC) | RES_MASK(RES_ID_DMA_CH0 + dma_channel), RES_WAIT_FOREVER);
        } else {
                resource_acquire(RES_MASK(RES_ID_SDADC), RES_WAIT_FOREVER);
        }
}

__STATIC_INLINE void ad_sdadc_release(HW_DMA_CHANNEL dma_channel)
{
        if (dma_channel < HW_DMA_CHANNEL_INVALID - 1) {
                /* make sure that the selected dma channel number is even */
                ASSERT_WARNING((dma_channel & 0x1) == 0);
                resource_release(RES_MASK(RES_ID_SDADC) | RES_MASK(RES_ID_DMA_CH0 + dma_channel));
        } else {
                resource_release(RES_MASK(RES_ID_SDADC));
        }
}
#else
__STATIC_INLINE void ad_sdadc_acquire(void)
{
       resource_acquire(RES_MASK(RES_ID_SDADC) , RES_WAIT_FOREVER);
}

__STATIC_INLINE void ad_sdadc_release(void)
{
        resource_release(RES_MASK(RES_ID_SDADC));
}
#endif /* HW_SDADC_DMA_SUPPORT */
#endif /* CONFIG_AD_SDADC_LOCKING */

static AD_SDADC_ERROR  ad_sdadc_check_apply_controller_conf(const ad_sdadc_controller_conf_t *conf, AD_IO_CONF_STATE onoff, ad_sdadc_use_input_t *input_channels)
{
        if (conf == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        if (conf->drv == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        if (onoff == AD_IO_CONF_OFF && conf->io) {
                /* I/O is set OFF */
                return ad_sdadc_io_config(conf->id, conf->io, AD_IO_CONF_OFF);
        }

        if (conf->drv->inp != HW_SDADC_INP_VBAT) {
                /* check inp for validity */
                switch (conf->drv->inp) {
                case HW_SDADC_IN_ADC0:
                case HW_SDADC_IN_ADC1:
                case HW_SDADC_IN_ADC2:
                case HW_SDADC_IN_ADC3:
                case HW_SDADC_IN_ADC4:
                case HW_SDADC_IN_ADC5:
                case HW_SDADC_IN_ADC6:
                case HW_SDADC_IN_ADC7:
                        input_channels->input0 = true;
                        break;
                default:
                        return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
                }

                /* check inn for validity - if 2 pins are used */
                if (conf->drv->input_mode == HW_SDADC_INPUT_MODE_DIFFERENTIAL) {
                        switch (conf->drv->inn) {
                        case HW_SDADC_IN_ADC0:
                        case HW_SDADC_IN_ADC1:
                        case HW_SDADC_IN_ADC2:
                        case HW_SDADC_IN_ADC3:
                        case HW_SDADC_IN_ADC4:
                        case HW_SDADC_IN_ADC5:
                        case HW_SDADC_IN_ADC6:
                        case HW_SDADC_IN_ADC7:
                                input_channels->input1 = true;
                                break;
                        default:
                                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
                        }
                }
        }

        /* reference voltage validity */
        switch (conf->drv->vref_selection) {
        case HW_SDADC_VREF_INTERNAL:
                /* force the correct value for V reference when internal Vref is used */
                OS_ASSERT(conf->drv->vref_voltage == HW_SDADC_VREF_VOLTAGE_INTERNAL);
                break;
        case HW_SDADC_VREF_EXTERNAL:
        case HW_SDADC_VREFN_INTERNAL_VREFP_EXTERNAL:
        case HW_SDADC_VREFN_EXTERNAL_VREFP_INTERNAL:
                OS_ASSERT(HW_SDADC_VREF_IN_RANGE(conf->drv->vref_voltage));
                break;
        default:
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        hw_sdadc_init(NULL);
        hw_sdadc_enable();
        hw_sdadc_configure(conf->drv);

        if (conf->drv->inp == HW_SDADC_INP_VBAT) {
                /* VBAT input does not need I/O configuration */
                return AD_SDADC_ERROR_NONE;
        }

        return ad_sdadc_io_config(HW_SDADC, conf->io, AD_IO_CONF_ON);
}

int ad_sdadc_reconfig(const ad_sdadc_handle_t handle, const ad_sdadc_driver_conf_t *drv)
{
        int ret;
        ad_sdadc_controller_conf_t new_controller_conf;

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        if (drv == NULL) {
                return AD_SDADC_ERROR_DRIVER_CONF_INVALID;
        }

        SDADC_MUTEX_GET(dynamic_data.busy);

        if (dynamic_data.conf == NULL) {
                /* use ad_sdadc_open instead */
                ret = AD_SDADC_ERROR_DRIVER_UNINITIALIZED;
                goto out_of_here;
        }

        if (dynamic_data.read_in_progress) {
                ret = AD_SDADC_ERROR_READ_IN_PROGRESS;
                goto out_of_here;
        }

        if (dynamic_data.current_drv->dma_setup != drv->dma_setup) {
                    /* use ad_sdadc_open instead */
                    ret = AD_SDADC_ERROR_DRIVER_CONF_INVALID;
                    goto out_of_here;
            }

        if (dynamic_data.current_drv->input_mode != drv->input_mode) {
                /* use ad_sdadc_open instead */
                ret = AD_SDADC_ERROR_DRIVER_MODE_INVALID;
                goto out_of_here;
        }

        if (dynamic_data.current_drv->inp != drv->inp ||
            ((dynamic_data.current_drv->input_mode == HW_SDADC_INPUT_MODE_DIFFERENTIAL) && (dynamic_data.current_drv->inn != drv->inn)) ) {
                /* can't change input source with reconfig */
                ret = AD_SDADC_ERROR_DRIVER_INPUT_INVALID;
                goto out_of_here;
        }

        /* keep the same io configuration */
        new_controller_conf.io = dynamic_data.conf->io;
        /* refresh the driver configuration */
        new_controller_conf.drv = drv;

        if ((ret = ad_sdadc_check_apply_controller_conf(&new_controller_conf, AD_IO_CONF_ON, &dynamic_data.using)) < AD_SDADC_ERROR_NONE) {
                goto out_of_here;
        }
        dynamic_data.current_drv = (ad_sdadc_driver_conf_t *) drv;

        ret = AD_SDADC_ERROR_NONE;

out_of_here:
        SDADC_MUTEX_PUT(dynamic_data.busy);
        return ret;
}

ad_sdadc_handle_t ad_sdadc_open(const ad_sdadc_controller_conf_t *conf)
{
#if (HW_SDADC_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = HW_DMA_CHANNEL_INVALID;
#endif
        if (conf == NULL) {
                return NULL;
        }

        if (dynamic_data.conf != NULL) {
                /* use ad_sdadc_reconfig instead */
                return NULL;
        }
#if (HW_SDADC_DMA_SUPPORT == 1)
        if (conf->drv->dma_setup != NULL) {
                dma_channel = conf->drv->dma_setup->channel;
        }
#endif
        pm_sleep_mode_request(pm_mode_idle);
#if (HW_SDADC_DMA_SUPPORT == 1)
        SDADC_RES_ACQUIRE(dma_channel);
#else
        SDADC_RES_ACQUIRE();
#endif /* HW_SDADC_DMA_SUPPORT */


        hw_sys_pd_com_enable();

        if (ad_sdadc_check_apply_controller_conf(conf, AD_IO_CONF_ON, &dynamic_data.using) < AD_SDADC_ERROR_NONE) {
#if (HW_SDADC_DMA_SUPPORT == 1)
                SDADC_RES_RELEASE(dma_channel);
#else
                SDADC_RES_RELEASE();
#endif /* HW_SDADC_DMA_SUPPORT */
                hw_sys_pd_com_disable();
                pm_sleep_mode_release(pm_mode_idle);
                dynamic_data.using.input0 = false;
                dynamic_data.using.input1 = false;
                return NULL;
        }

        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_ENABLE, conf->io);
        /* The value of the handle has no specific meaning, it's just an arbitrary number. */
        dynamic_data.handle              = (ad_sdadc_handle_t) conf;
        dynamic_data.conf                = (ad_sdadc_controller_conf_t *) conf;
        dynamic_data.current_drv         = (ad_sdadc_driver_conf_t *) conf->drv;
        dynamic_data.read_cb             = NULL;
        dynamic_data.user_data           = NULL;
        dynamic_data.read_in_progress    = false;

        return dynamic_data.handle;
}

int ad_sdadc_close(const ad_sdadc_handle_t handle, bool forced)
{
#if (HW_SDADC_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = HW_DMA_CHANNEL_INVALID;
#endif
        int off_configuration_valid;

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }
#if (HW_SDADC_DMA_SUPPORT == 1)
        if (dynamic_data.conf->drv->dma_setup != NULL) {
                dma_channel = dynamic_data.conf->drv->dma_setup->channel;
        }
#endif
        OS_ENTER_CRITICAL_SECTION();

        if (dynamic_data.read_in_progress) {
                if (forced) {
                        hw_sdadc_clear_interrupt();
                        hw_sdadc_unregister_interrupt();
                } else {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_SDADC_ERROR_READ_IN_PROGRESS;
                }
        }

        off_configuration_valid =
                        ad_sdadc_check_apply_controller_conf(dynamic_data.conf, AD_IO_CONF_OFF, &dynamic_data.using);
        ad_sdadc_pad_latches(AD_SDADC_PAD_LATCHES_OP_DISABLE, dynamic_data.conf->io);

        hw_sdadc_init(NULL);

        OS_MUTEX keep = dynamic_data.busy;
        dynamic_data.conf = NULL;
        dynamic_data.handle = NULL;

        dynamic_data.busy = keep;

        OS_LEAVE_CRITICAL_SECTION();

        hw_sys_pd_com_disable();

#if (HW_SDADC_DMA_SUPPORT == 1)
        SDADC_RES_RELEASE(dma_channel);
#else
        SDADC_RES_RELEASE();
#endif
        pm_sleep_mode_release(pm_mode_idle);

        return off_configuration_valid;
}

static void sdadc_cb_wrapper_async(void *param, uint32_t to_go)
{
        if (!dynamic_data.read_in_progress) {
                return;
        }

        if (dynamic_data.read_cb != NULL) {
                dynamic_data.read_cb(dynamic_data.user_data, to_go);
        }

        dynamic_data.read_in_progress = false;
}

int ad_sdadc_read_async(const ad_sdadc_handle_t handle, uint32_t nof_conv, uint16_t *outbuf, ad_sdadc_user_cb read_async_cb, void *user_data)
{

        ASSERT_WARNING(nof_conv > 0);

        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        SDADC_MUTEX_GET(dynamic_data.busy);

        if (dynamic_data.read_in_progress || hw_sdadc_in_progress()) {
                SDADC_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_READ_IN_PROGRESS;
        }

        if (nof_conv > 1) {
                hw_sdadc_set_continuous(true);
        } else {
                hw_sdadc_set_continuous(false);
        }

        dynamic_data.read_in_progress = true;
        dynamic_data.read_cb = read_async_cb;
        dynamic_data.user_data = user_data;

        if (!hw_sdadc_read(nof_conv, outbuf, sdadc_cb_wrapper_async, NULL)) {
                dynamic_data.read_in_progress = false;
                SDADC_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_OTHER;
        }

        SDADC_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}

static void sdadc_cb_wrapper_sync(void *param, uint32_t to_go)
{
        OS_EVENT_SIGNAL_FROM_ISR(dynamic_data.sync_event);
}

int ad_sdadc_read(const ad_sdadc_handle_t handle, uint32_t nof_conv,  uint16_t *outbuf)
{
        if (AD_SDADC_HANDLE_IS_INVALID(handle)) {
                return AD_SDADC_ERROR_HANDLE_INVALID;
        }

        SDADC_MUTEX_GET(dynamic_data.busy);

        if (nof_conv > 1) {
                hw_sdadc_set_continuous(true);
        } else {
                hw_sdadc_set_continuous(false);
        }

        if (dynamic_data.read_in_progress) {
                SDADC_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_READ_IN_PROGRESS;
        }

        if (!hw_sdadc_read(nof_conv, outbuf, sdadc_cb_wrapper_sync, NULL)) {
                SDADC_MUTEX_PUT(dynamic_data.busy);
                return AD_SDADC_ERROR_OTHER;
        }

        OS_EVENT_WAIT(dynamic_data.sync_event, OS_EVENT_FOREVER);

        SDADC_MUTEX_PUT(dynamic_data.busy);

        return AD_SDADC_ERROR_NONE;
}

ADAPTER_INIT(ad_sdadc_adapter, ad_sdadc_init);

#endif /* dg_configSDADC_ADAPTER */
