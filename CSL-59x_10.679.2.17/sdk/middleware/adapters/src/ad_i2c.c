/**
 ****************************************************************************************
 *
 * @file ad_i2c.c
 *
 * @brief I2C device access API implementation
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

#if dg_configI2C_ADAPTER

#include <stdarg.h>
#include "ad_i2c.h"
#include "hw_i2c.h"
#if (HW_I2C_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif
#include "hw_clk.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "sys_power_mgr.h"

#include "hw_sys.h"
#include "sdk_list.h"

/**
 * \def CONFIG_AD_I2C_LOCKING
 *
 * \brief Controls whether I2C adapter resource locking is enabled
 *
 * By default, the I2C adapter internally handles concurrent accesses on an I2C bus by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_I2C_LOCKING
# define CONFIG_AD_I2C_LOCKING                  ( 1 )
#endif /* CONFIG_AD_I2C_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_I2C_LOCKING == 1)
# define I2C_MUTEX_CREATE(mutex)                do { \
                                                        OS_ASSERT((mutex) == NULL); \
                                                        OS_MUTEX_CREATE(mutex); \
                                                        OS_ASSERT(mutex); \
                                                } while (0)
# define I2C_MUTEX_GET(mutex)                   do { \
                                                        OS_ASSERT(mutex); \
                                                        OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                } while (0)
# define I2C_MUTEX_PUT(mutex)                   OS_MUTEX_PUT(mutex)

# define I2C_BSR_ACQUIRE(periph_id)             do {} while (0)
# define I2C_BSR_RELEASE(periph_id)             do {} while (0)

# if (HW_I2C_DMA_SUPPORT == 1)
#   define I2C_RES_ACQUIRE(res_id, dma_chan)    ad_i2c_bus_acquire((res_id), (dma_chan))
#   define I2C_RES_RELEASE(res_id, dma_chan)    ad_i2c_bus_release((res_id), (dma_chan))
# else
#   define I2C_RES_ACQUIRE(res_id, dma_chan)    ad_i2c_bus_acquire((res_id))
#   define I2C_RES_RELEASE(res_id, dma_chan)    ad_i2c_bus_release((res_id))
# endif
#else /* CONFIG_AD_I2C_LOCKING = 0 */
# define I2C_MUTEX_CREATE(mutex)                do {} while (0)
# define I2C_MUTEX_GET(mutex)                   do {} while (0)
# define I2C_MUTEX_PUT(mutex)                   do {} while (0)

# define I2C_BSR_ACQUIRE(periph_id)             do {} while (0)
# define I2C_BSR_RELEASE(periph_id)             do {} while (0)

# define I2C_RES_ACQUIRE(res_id, dma_channel)   do {} while (0)
# define I2C_RES_RELEASE(res_id, dma_channel)   do {} while (0)
#endif /* CONFIG_AD_I2C_LOCKING */

/**
 * \brief I2C adapter (internal) dynamic data
 *
 * Dynamic data structure of I2C controller
 *
 */
typedef struct {
        /**< I2C controller current configuration */
        const ad_i2c_controller_conf_t *conf;
        /**< Internal data */
#if (CONFIG_AD_I2C_LOCKING == 1)
        OS_TASK owner; /**< The task which opened the controller */
#endif
        ad_i2c_driver_conf_t *current_drv;
#if (HW_I2C_SLAVE_SUPPORT == 1)
        i2c_slave_state_data_t slave_data;
#endif
}ad_i2c_dynamic_data_t;

#if (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 1) || (CONFIG_AD_I2C_LOCKING == 1)
/**
 * \brief I2C adapter (internal) static data
 *
 * Static data structure of I2C controller
 *
 */
typedef struct {
#if CONFIG_I2C_USE_SYNC_TRANSACTIONS
        OS_EVENT event; /**< Semaphore for blocking calls  */
#endif /* CONFIG_I2C_USE_SYNC_TRANSACTIONS */
#if (CONFIG_AD_I2C_LOCKING == 1)
        OS_MUTEX busy; /**< Semaphore for thread safety */
#endif
}ad_i2c_static_data_t;

#if defined(HW_I2C3)
__RETAINED static ad_i2c_static_data_t i2c3_static_data;
static ad_i2c_dynamic_data_t i2c3_dynamic_data;
#endif

#if defined(HW_I2C2)
__RETAINED static ad_i2c_static_data_t i2c2_static_data;
static ad_i2c_dynamic_data_t i2c2_dynamic_data;
#endif

__RETAINED static ad_i2c_static_data_t i2c_static_data;
static ad_i2c_dynamic_data_t i2c_dynamic_data;

#endif /* (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 1) || (CONFIG_AD_I2C_LOCKING == 1) */

#if defined(HW_I2C3)
#define AD_I2C_HANDLE_IS_VALID(x) (((((x) == &i2c_dynamic_data) || ((x) == &i2c2_dynamic_data)) || \
                                   ((x) == &i2c3_dynamic_data)) && (((ad_i2c_dynamic_data_t *)(x))->conf != NULL))
#elif defined(HW_I2C2)
#define AD_I2C_HANDLE_IS_VALID(x) ((((x) == &i2c_dynamic_data) || ((x) == &i2c2_dynamic_data)) && \
                                   (((ad_i2c_dynamic_data_t *)(x))->conf != NULL))
#else
#define AD_I2C_HANDLE_IS_VALID(x) (((x) == &i2c_dynamic_data) && \
                                   (((ad_i2c_dynamic_data_t *)(x))->conf != NULL))
#endif

#define AD_I2C_IO_SIZE            (2)

#if (CONFIG_I2C_USE_SYNC_TRANSACTIONS == 1) || (CONFIG_AD_I2C_LOCKING == 1)
static const ad_i2c_static_data_t* ad_i2c_get_static_data_by_hw_id(const HW_I2C_ID id)
{
#if defined(HW_I2C3)
        return ((id == HW_I2C1) ? &i2c_static_data : ((id == HW_I2C2) ? &i2c2_static_data : &i2c3_static_data));
#elif defined(HW_I2C2)
        return ((id == HW_I2C1) ? &i2c_static_data : &i2c2_static_data);
#else
        return &i2c_static_data;
#endif
}
#endif

#if (HW_I2C_SLAVE_SUPPORT == 1)
static const ad_i2c_dynamic_data_t* ad_i2c_get_handle_by_hw_id(const HW_I2C_ID id)
{
#if defined(HW_I2C3)
        return ((id == HW_I2C1) ? &i2c_dynamic_data : ((id == HW_I2C2) ? &i2c2_dynamic_data : &i2c3_dynamic_data));
#elif defined(HW_I2C2)
        return ((id == HW_I2C1) ? &i2c_dynamic_data : &i2c2_dynamic_data);
#else
        return &i2c_dynamic_data;
#endif
}
#endif /* HW_I2C_SLAVE_SUPPORT */

__RETAINED_RW static ad_i2c_io_conf_t i2c_last_io_config = {
        .scl = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C_SCL,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .sda = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C_SDA,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
};
#if defined(HW_I2C2)
__RETAINED_RW static ad_i2c_io_conf_t i2c2_last_io_config = {
        .scl = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C2_SCL,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .sda = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C2_SDA,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .voltage_level = HW_GPIO_POWER_V33
};
#endif
#if defined(HW_I2C3)
__RETAINED_RW static ad_i2c_io_conf_t i2c3_last_io_config = {
        .scl = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C3_SCL,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .sda = {
                .port = HW_GPIO_PORT_MAX,
                .pin = HW_GPIO_PIN_MAX,
                .on = {
                        .mode = HW_GPIO_MODE_NONE,
                        .function = HW_GPIO_FUNC_I2C3_SDA,
                        .high = false
                },
                .off = {
                        .mode = HW_GPIO_MODE_INPUT_PULLUP,
                        .function = HW_GPIO_FUNC_GPIO,
                        .high = false
                }
        },
        .voltage_level = HW_GPIO_POWER_V33
};
#endif

int ad_i2c_io_config(HW_I2C_ID id, const ad_i2c_io_conf_t *io_config, AD_IO_CONF_STATE state)
{
        hw_sys_pd_com_enable();
        if (ad_io_configure(&io_config->scl, AD_I2C_IO_SIZE, state) != AD_IO_ERROR_NONE) {
                return AD_I2C_ERROR_IO_CFG_INVALID;
        }
        ad_io_set_pad_latch(&io_config->scl, AD_I2C_IO_SIZE, AD_IO_PAD_LATCHES_OP_TOGGLE);
        hw_sys_pd_com_disable();
        return AD_I2C_ERROR_NONE;
}

HW_I2C_ID ad_i2c_get_hw_i2c_id(const ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        return i2c->conf->id;
}

static bool ad_i2c_controller_is_busy(HW_I2C_ID id)
{
        return (hw_i2c_is_occupied(id) || hw_i2c_controler_is_busy(id) ||
                !hw_i2c_is_tx_fifo_empty(id) || hw_i2c_is_rx_fifo_not_empty(id));
}

static bool ad_i2c_master_abort_transfer(HW_I2C_ID id)
{
        hw_i2c_master_abort_transfer(id);
        uint8_t abort_cnt = 0;
        /* wait for master to abort transaction */
        while (HW_I2C_REG_GETF(id, I2C_ENABLE, I2C_ABORT)) {
                if (abort_cnt == 100) {
                        return false;
                }
                abort_cnt++;
                hw_clk_delay_usec(10);
        }
        return true;
}

int ad_i2c_reconfig(ad_i2c_handle_t p, const ad_i2c_driver_conf_t *config)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const HW_I2C_ID id = ad_i2c_get_hw_i2c_id(i2c);
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(id);
#endif

        I2C_MUTEX_GET(i2c_static->busy);

        OS_ASSERT(config);

        /* If I2C driver is already configured, check if the new configuration could be applied */
        if (i2c->current_drv) {
                /* Check if i2c is in use */
                if (ad_i2c_controller_is_busy(i2c->conf->id)) {
                        I2C_MUTEX_PUT(i2c_static->busy);
                        return AD_I2C_ERROR_CONTROLLER_BUSY;
                }

                if (i2c->current_drv->i2c.mode != config->i2c.mode
#if (HW_I2C_DMA_SUPPORT == 1)
                        && i2c->current_drv->dma_channel != config->dma_channel
#endif
                ) {
                        ASSERT_WARNING(0);
                        I2C_MUTEX_PUT(i2c_static->busy);
                        return AD_I2C_ERROR_DRIVER_CONF_INVALID;
                }
        }

        hw_i2c_init(id, &(config->i2c));

#if (HW_I2C_SLAVE_SUPPORT == 1)
        if (config->i2c.mode == HW_I2C_MODE_SLAVE) {
                if (!(i2c->slave_data.state & AD_I2C_SLAVE_STATE_INIT)) {
                        goto after_enabling;
                }
        }
        hw_i2c_enable(id);
after_enabling:
#else
        hw_i2c_enable(id);
#endif
        hw_i2c_reset_abort_source(id);
        hw_i2c_reset_int_all(id);
        i2c->current_drv = (ad_i2c_driver_conf_t *)config;
        I2C_MUTEX_PUT(i2c_static->busy);

        return AD_I2C_ERROR_NONE;
}

#if (CONFIG_AD_I2C_LOCKING == 1)

#if (HW_I2C_DMA_SUPPORT == 1)
static void ad_i2c_bus_acquire(const HW_I2C_ID bus_id, HW_DMA_CHANNEL dma_channel)
#else
static void ad_i2c_bus_acquire(const HW_I2C_ID bus_id)
#endif /* HW_I2C_DMA_SUPPORT */
{
#if defined(HW_I2C3)
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : (bus_id == HW_I2C2) ? RES_ID_I2C2 : RES_ID_I2C3;
#elif defined(HW_I2C2)
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : RES_ID_I2C2;
#else
        RES_ID res_id = RES_ID_I2C1;
#endif
        resource_mask_t resources = RES_MASK(res_id);

#if (HW_I2C_DMA_SUPPORT == 1)
        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                ASSERT_WARNING(dma_channel < HW_DMA_CHANNEL_INVALID - 1);
                resources |=  RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                              RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1);
        }
#endif /* HW_I2C_DMA_SUPPORT */
        resource_acquire(resources, RES_WAIT_FOREVER);
}

#if (HW_I2C_DMA_SUPPORT == 1)
static void ad_i2c_bus_release(const HW_I2C_ID bus_id, HW_DMA_CHANNEL dma_channel)
#else
static void ad_i2c_bus_release(const HW_I2C_ID bus_id)
#endif /* HW_I2C_DMA_SUPPORT */
{
#if defined(HW_I2C3)
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : (bus_id == HW_I2C2) ? RES_ID_I2C2 : RES_ID_I2C3;
#elif defined(HW_I2C2)
        RES_ID res_id = (bus_id == HW_I2C1) ? RES_ID_I2C1 : RES_ID_I2C2;
#else
        RES_ID res_id = RES_ID_I2C1;
#endif
        resource_mask_t resources = RES_MASK(res_id);

#if (HW_I2C_DMA_SUPPORT == 1)
        if (dma_channel < HW_DMA_CHANNEL_INVALID) {
                ASSERT_WARNING(dma_channel < HW_DMA_CHANNEL_INVALID - 1);
                resources |=  RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                              RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1);
        }
#endif /* HW_I2C_DMA_SUPPORT */
        resource_release(resources);
}
#endif /* CONFIG_AD_I2C_LOCKING */

#if CONFIG_I2C_USE_SYNC_TRANSACTIONS
typedef struct {
        ad_i2c_dynamic_data_t *i2c;
        uint32_t abort_source;
        bool success;
} i2c_cb_data_t;

static void ad_i2c_transaction_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        i2c_cb_data_t *data = (i2c_cb_data_t *) cb_data;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(id);
        uint32_t abort_source = hw_i2c_get_abort_source(id);

        if (!success && (abort_source == HW_I2C_ABORT_NONE)) {
                abort_source = HW_I2C_ABORT_SW_ERROR;
        }
        data->success = success;
        data->abort_source = abort_source;
        if (in_interrupt()) {
                OS_EVENT_SIGNAL_FROM_ISR(i2c_static->event);
        } else {
                OS_EVENT_SIGNAL(i2c_static->event);
        }
}

static int _ad_i2c_write(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        uint32_t ret = HW_I2C_ABORT_NONE;
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;

        ASSERT_WARNING( ( condition_flags &
                         ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }
        if (condition_flags & HW_I2C_F_ADD_STOP) {
                condition_flags |= HW_I2C_F_WAIT_FOR_STOP;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        hw_i2c_write_buffer_async(i2c->conf->id, wbuf, wlen, ad_i2c_transaction_cb,
                                  (void *)&transaction_data, condition_flags);

        OS_BASE_TYPE res;
        res = OS_EVENT_WAIT(i2c_static->event, timeout);
        if (res == OS_EVENT_SIGNALED) {
                /* transfer is finished without stop condition, wait for abort error if any */
                if ((condition_flags & HW_I2C_F_WAIT_FOR_STOP) == 0) {
                        if (hw_i2c_controler_is_busy(i2c->conf->id)) {
                                // Maximum number of bits to be sent are 32*(8+1) bits = 288
                                // if speed is HW_I2C_SPEED_HIGH (3.4Mb/s) or HW_I2C_SPEED_FAST (400kb/s) then they
                                // can be sent in one ms. Add one more ms just to be safe.
                                if (i2c->current_drv->i2c.speed == HW_I2C_SPEED_STANDARD) {
                                        OS_DELAY_MS(4);
                                } else {
                                        OS_DELAY_MS(2);
                                }
                        }
                        ret = hw_i2c_get_abort_source(i2c->conf->id);
                }
        } else {
                /* time out occurred*/
                hw_i2c_unregister_int(i2c->conf->id);
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_TRANSFER_TIMEOUT;
        }
        I2C_MUTEX_PUT(i2c_static->busy);

        if (ret != HW_I2C_ABORT_NONE) {
                return (int)(transaction_data.abort_source |= ret);
        } else {
                return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
        }
}

int ad_i2c_write(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags)
{
        return _ad_i2c_write(p, wbuf, wlen, condition_flags, OS_EVENT_FOREVER);
}

int ad_i2c_write_with_to(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        return _ad_i2c_write(p, wbuf, wlen, condition_flags, timeout);
}

static int _ad_i2c_read(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;
        ASSERT_WARNING( (condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

#if (HW_I2C_DMA_SUPPORT == 1)
        if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID || rlen <= 1) {
#endif
                hw_i2c_read_buffer_async(i2c->conf->id, rbuf, rlen, ad_i2c_transaction_cb,
                                         (void *)&transaction_data, condition_flags);
#if (HW_I2C_DMA_SUPPORT == 1)
        } else {
                hw_i2c_read_buffer_dma(i2c->conf->id, i2c->conf->drv->dma_channel,
                                       rbuf, rlen, ad_i2c_transaction_cb, (void *)&transaction_data,
                                       condition_flags);
        }
#endif

        if (OS_EVENT_WAIT(i2c_static->event, timeout) == OS_EVENT_NOT_SIGNALED) {
                /* time out occurred*/
#if (HW_I2C_DMA_SUPPORT == 1)
                if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID || rlen <= 1) {
#endif
                        hw_i2c_unregister_int(i2c->conf->id);
                        if (!ad_i2c_master_abort_transfer(i2c->conf->id)) {
                                I2C_MUTEX_PUT(i2c_static->busy);
                                return AD_I2C_ERROR_CONTROLLER_ABORT_FAIL;
                        }
                        hw_i2c_flush_rx_fifo(i2c->conf->id);
#if (HW_I2C_DMA_SUPPORT == 1)
                } else {
                        hw_i2c_reset_dma_cb(i2c->conf->id);
                }
#endif
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_TRANSFER_TIMEOUT;
        }

        I2C_MUTEX_PUT(i2c_static->busy);

        return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
}

int ad_i2c_read(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags)
{
        return _ad_i2c_read(p, rbuf, rlen, condition_flags, OS_EVENT_FOREVER);
}

int ad_i2c_read_with_to(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        return _ad_i2c_read(p, rbuf, rlen, condition_flags, timeout);
}

static int _ad_i2c_write_read(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
        i2c_cb_data_t transaction_data;

        ASSERT_WARNING( ( condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        transaction_data.i2c = i2c;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        hw_i2c_write_then_read_async(i2c->conf->id, wbuf, wlen, rbuf, rlen, ad_i2c_transaction_cb,
                                     (void *)&transaction_data, condition_flags);

        if (OS_EVENT_WAIT(i2c_static->event, timeout) == OS_EVENT_NOT_SIGNALED) {
                /* time out occurred*/
                hw_i2c_unregister_int(i2c->conf->id);
                if (!ad_i2c_master_abort_transfer(i2c->conf->id)) {
                        I2C_MUTEX_PUT(i2c_static->busy);
                        return AD_I2C_ERROR_CONTROLLER_ABORT_FAIL;
                }
                hw_i2c_flush_rx_fifo(i2c->conf->id);
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_TRANSFER_TIMEOUT;
        }
        I2C_MUTEX_PUT(i2c_static->busy);

        return ((transaction_data.success) ? 0 : (int)transaction_data.abort_source);
}

int ad_i2c_write_read(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags)
{
         return _ad_i2c_write_read(p, wbuf, wlen, rbuf, rlen, condition_flags, OS_EVENT_FOREVER);
}

int ad_i2c_write_read_with_to(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                      size_t rlen, uint8_t condition_flags, OS_TICK_TIME timeout)
{
        return _ad_i2c_write_read(p, wbuf, wlen, rbuf, rlen, condition_flags, timeout);
}
#endif /* CONFIG_I2C_USE_SYNC_TRANSACTIONS */

#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
/* define maximum number of supported independent i2c devices */
#if defined(HW_I2C3)
#define I2C_TRANSACT_DEV_NUM 3
#elif defined(HW_I2C2)
#define I2C_TRANSACT_DEV_NUM 2
#else
#define I2C_TRANSACT_DEV_NUM 1
#endif

typedef struct {
        ad_i2c_dynamic_data_t *i2c;
        void *app_user_data;
        ad_i2c_user_cb app_cb;
} I2C_TRANSACT_TYPE;

static I2C_TRANSACT_TYPE i2c_transact_data[I2C_TRANSACT_DEV_NUM];

static void ad_i2c_async_cb(HW_I2C_ID id, void *user_data, uint16_t transferred, bool success)
{
        I2C_TRANSACT_TYPE *cb_transact_data = user_data;

        cb_transact_data->i2c = NULL;

        if (success) {
                cb_transact_data->app_cb(cb_transact_data->app_user_data, HW_I2C_ABORT_NONE);
        } else {
                cb_transact_data->app_cb(cb_transact_data->app_user_data, HW_I2C_ABORT_SW_ERROR);
        }
}

static void clear_transact_data(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        uint8_t idx = 0;

        while (idx < I2C_TRANSACT_DEV_NUM) {
                if (i2c_transact_data[idx].i2c == i2c) {
                        i2c_transact_data[idx].i2c = NULL;
                        i2c_transact_data[idx].app_cb = NULL;
                        i2c_transact_data[idx].app_user_data = NULL;
                        return;
                }
                idx++;
        }
}

static I2C_TRANSACT_TYPE *get_transact_data(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        uint8_t idx = 0;

        while (idx < I2C_TRANSACT_DEV_NUM) {
                if (i2c_transact_data[idx].i2c == NULL) {
                        return &i2c_transact_data[idx];
                }

                if (i2c_transact_data[idx].i2c == i2c) {
                        return NULL;
                }

                idx++;
        }

        return NULL;
}

int ad_i2c_write_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen,
                       ad_i2c_user_cb cb, void *user_data, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( ( condition_flags &
                         ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        if (condition_flags & HW_I2C_F_ADD_STOP) {
                condition_flags |= HW_I2C_F_WAIT_FOR_STOP;
        }

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

        hw_i2c_write_buffer_async(i2c->conf->id, wbuf, wlen, ad_i2c_async_cb, cb_transact_data,
                                  condition_flags);
        I2C_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_read_async(ad_i2c_handle_t p, uint8_t *rbuf, size_t rlen, ad_i2c_user_cb cb,
                      void *user_data, uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( ( condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

#if (HW_I2C_DMA_SUPPORT == 1)
        if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID || rlen <= 1) {
#endif
                hw_i2c_read_buffer_async(i2c->conf->id, rbuf, rlen, ad_i2c_async_cb,
                                         cb_transact_data, condition_flags);
#if (HW_I2C_DMA_SUPPORT == 1)
        } else {
                hw_i2c_read_buffer_dma(i2c->conf->id, i2c->conf->drv->dma_channel, rbuf,
                                       rlen, ad_i2c_async_cb, cb_transact_data, condition_flags);
        }
#endif
        I2C_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_write_read_async(ad_i2c_handle_t p, const uint8_t *wbuf, size_t wlen,
                            uint8_t *rbuf, size_t rlen, ad_i2c_user_cb cb, void *user_data,
                            uint8_t condition_flags)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        I2C_TRANSACT_TYPE *cb_transact_data;

        ASSERT_WARNING( (condition_flags & ~(HW_I2C_F_NONE | HW_I2C_F_ADD_STOP | HW_I2C_F_ADD_RESTART) ) == 0);

        I2C_MUTEX_GET(i2c_static->busy);
        /* Check if i2c hw driver is in use and already occupied */
        if (hw_i2c_is_occupied(i2c->conf->id)) {
                I2C_MUTEX_PUT(i2c_static->busy);
                return AD_I2C_ERROR_CONTROLLER_BUSY;
        }

        cb_transact_data = get_transact_data(i2c);
        /* Check if async data write/read is occupied by requested i2c device */
        OS_ASSERT(cb_transact_data);

        cb_transact_data->i2c = i2c;
        cb_transact_data->app_cb = cb;
        cb_transact_data->app_user_data = user_data;

        hw_i2c_write_then_read_async(i2c->conf->id, wbuf, wlen, rbuf, rlen,
                                     ad_i2c_async_cb, (void *)cb_transact_data, condition_flags);
        I2C_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}
#endif /* CONFIG_I2C_USE_ASYNC_TRANSACTIONS */

#if (HW_I2C_SLAVE_SUPPORT == 1)

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event);

static void ad_i2c_slave_sent_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)cb_data;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        slave->state &= ~AD_I2C_SLAVE_STATE_WRITE_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_READ_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks && NULL != slave->event_callbacks->data_sent) {
                slave->event_callbacks->data_sent((ad_i2c_dynamic_data_t *) i2c, len, success,
                                                  slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void ad_i2c_slave_received_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        OS_ASSERT(cb_data);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *) cb_data;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        slave->state &= ~AD_I2C_SLAVE_STATE_READ_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_WRITE_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks &&
                NULL != slave->event_callbacks->data_received) {
                slave->event_callbacks->data_received((ad_i2c_handle_t)i2c, len, success,
                                                      slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void i2c_slave_send(HW_I2C_ID id, ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        /*
         * Master initiated read, if user already prepared buffer, send it.
         * If not notify user that read request is pending.
         */
        if (slave->output_buffer && slave->output_buffer_len) {
                hw_i2c_write_buffer_async(id, slave->output_buffer, slave->output_buffer_len,
                                          ad_i2c_slave_sent_cb, i2c, 0);
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->read_request != NULL) {
                        slave->event_callbacks->read_request(i2c, slave->user_data);
                }
        }
}

/*
 * Master initiated write, if user already prepared buffer, use it.
 * If not notify user that master started write.
 */
static void i2c_slave_receive(HW_I2C_ID id, ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        if (slave->input_buffer && slave->input_buffer_len) {
#if (HW_I2C_DMA_SUPPORT == 1)
                if (i2c->conf->drv->dma_channel >= HW_DMA_CHANNEL_INVALID ||
                    slave->input_buffer_len <= 1) {
#endif
                        hw_i2c_read_buffer_async(id, slave->input_buffer, slave->input_buffer_len,
                                                 ad_i2c_slave_received_cb, i2c, 0);
#if (HW_I2C_DMA_SUPPORT == 1)
                } else {
                        hw_i2c_register_slave_dma_read_callback(id);
                }
#endif
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->data_ready != NULL) {
                        slave->event_callbacks->data_ready((ad_i2c_handle_t)i2c, slave->user_data);
                }
        }
}

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event)
{
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)ad_i2c_get_handle_by_hw_id(id);

        switch (event) {
        case HW_I2C_EVENT_READ_REQUEST:
                i2c_slave_send(id, i2c);
                break;
        case HW_I2C_EVENT_DATA_READY:
                i2c_slave_receive(id, i2c);
                break;
        case HW_I2C_EVENT_TX_ABORT:
        case HW_I2C_EVENT_RX_OVERFLOW:
        case HW_I2C_EVENT_INVALID:
                break;
        default:
                ASSERT_WARNING(0);
        }
}

int ad_i2c_start_slave(ad_i2c_handle_t p, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata,
                       uint16_t rlen, const i2c_dev_slave_event_callbacks_t *events,
                       void *user_data)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        I2C_MUTEX_GET(i2c_static->busy);
        slave->state = AD_I2C_SLAVE_STATE_INIT;

        /* Enable I2C controller */
        hw_i2c_enable(i2c->conf->id);

        slave->event_callbacks = events;
        slave->user_data = user_data;
        slave->output_buffer = wdata;
        slave->output_buffer_len = wlen;
        slave->input_buffer = rdata;
        slave->input_buffer_len = rlen;
        if (wdata != NULL && wlen) {
                slave->state |= AD_I2C_SLAVE_STATE_WRITE_PENDING;
        }
        if (rdata != NULL && rlen > 0) {
                slave->state |= AD_I2C_SLAVE_STATE_READ_PENDING;
#if (HW_I2C_DMA_SUPPORT == 1)
                if (i2c->conf->drv->dma_channel < HW_DMA_CHANNEL_INVALID) {
                        /* When DMA is used for the Rx it is better to set it up here (rather than
                         * in the read request handler) so that the slave is more responsive. */
                        hw_i2c_prepare_dma(i2c->conf->id, i2c->conf->drv->dma_channel,
                                           (uint16_t *) rdata, rlen, HW_I2C_DMA_TRANSFER_SLAVE_READ,
                                           ad_i2c_slave_received_cb, i2c, HW_I2C_F_NONE);
                        hw_i2c_dma_start(i2c->conf->id);
                }
#endif /* HW_I2C_DMA_SUPPORT */
        }

        hw_i2c_set_slave_callback(i2c->conf->id, ad_i2c_slave_cb);
        I2C_MUTEX_PUT(i2c_static->busy);
        return AD_I2C_ERROR_NONE;
}

int ad_i2c_stop_slave(ad_i2c_handle_t p)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        i2c_slave_state_data_t *slave = &(i2c->slave_data);

        I2C_MUTEX_GET(i2c_static->busy);

        OS_EVENT_CREATE(slave->operation_done_event);

        if (hw_i2c_is_slave_busy(i2c->conf->id)) {
                OS_EVENT_WAIT(slave->operation_done_event, OS_EVENT_FOREVER);
                /* Wait for flushing TX fifo */
                while (!hw_i2c_is_tx_fifo_empty(i2c->conf->id));
                while (hw_i2c_is_slave_busy(i2c->conf->id));
        }

        OS_EVENT_DELETE(slave->operation_done_event);
        slave->event_callbacks = NULL;
        slave->user_data = NULL;
        slave->output_buffer = NULL;
        slave->output_buffer_len = 0;
        slave->input_buffer = NULL;
        slave->input_buffer_len = 0;
        slave->state = AD_I2C_SLAVE_STATE_STOPPED;
        slave->operation_done_event = NULL;
        hw_i2c_set_slave_callback(i2c->conf->id, NULL);

        hw_i2c_disable(i2c->conf->id);

        I2C_MUTEX_PUT(i2c_static->busy);

        return AD_I2C_ERROR_NONE;
}

void ad_i2c_clear_read_slave(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        I2C_MUTEX_GET(i2c_static->busy);
        while (hw_i2c_is_rx_fifo_not_empty(i2c->conf->id)) {
                hw_i2c_read_byte(i2c->conf->id);
        }
        I2C_MUTEX_PUT(i2c_static->busy);
}
#endif /* HW_I2C_SLAVE_SUPPORT */

void ad_i2c_wait_while_master_busy(ad_i2c_handle_t p)
{
        OS_ASSERT(p);
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
#if (CONFIG_AD_I2C_LOCKING == 1)
        const ad_i2c_static_data_t *i2c_static = ad_i2c_get_static_data_by_hw_id(i2c->conf->id);
#endif
        I2C_MUTEX_GET(i2c_static->busy);
        while (hw_i2c_is_master_busy(i2c->conf->id));
        I2C_MUTEX_PUT(i2c_static->busy);
}

static void release_ad_i2c_open_resources(ad_i2c_dynamic_data_t *i2c,
                const ad_i2c_controller_conf_t *ctrl_config)
{
        hw_sys_pd_com_disable();
        /* Driver configuration failed, release bus and */
        I2C_RES_RELEASE(ctrl_config->id, ctrl_config->drv->dma_channel);
#if defined(HW_I2C3)
        I2C_BSR_RELEASE(ctrl_config->id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 :
                        ctrl_config->id == HW_I2C2 ? SYS_BSR_PERIPH_ID_I2C2 : SYS_BSR_PERIPH_ID_I2C3);
#elif defined(HW_I2C2)
        I2C_BSR_RELEASE(ctrl_config->id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 : SYS_BSR_PERIPH_ID_I2C2);
#else
        I2C_BSR_RELEASE(SYS_BSR_PERIPH_ID_I2C1);
#endif
        i2c->conf = NULL;
        pm_sleep_mode_release(pm_mode_idle);
}

ad_i2c_handle_t ad_i2c_open(const ad_i2c_controller_conf_t *ctrl_config)
{
        OS_ASSERT(ctrl_config);
#if defined(HW_I2C3)
        ad_i2c_dynamic_data_t *i2c = ((ctrl_config->id == HW_I2C1) ? &i2c_dynamic_data :
                                      (ctrl_config->id == HW_I2C2) ? &i2c2_dynamic_data : &i2c3_dynamic_data);
#elif defined(HW_I2C2)
        ad_i2c_dynamic_data_t *i2c = ((ctrl_config->id == HW_I2C1) ? &i2c_dynamic_data : &i2c2_dynamic_data);
#else
        ad_i2c_dynamic_data_t *i2c = &i2c_dynamic_data;
#endif
        ASSERT_WARNING(i2c);
        pm_sleep_mode_request(pm_mode_idle);

#if defined(HW_I2C3)
        ad_i2c_io_conf_t *last =  ((ctrl_config->id == HW_I2C1) ? &i2c_last_io_config :
                                   (ctrl_config->id == HW_I2C2) ? &i2c2_last_io_config : &i2c3_last_io_config);

        I2C_BSR_ACQUIRE(ctrl_config->id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 :
                        ctrl_config->id == HW_I2C2 ? SYS_BSR_PERIPH_ID_I2C2 : SYS_BSR_PERIPH_ID_I2C3);
#elif defined(HW_I2C2)
        ad_i2c_io_conf_t *last = ((ctrl_config->id == HW_I2C1) ? &i2c_last_io_config : &i2c2_last_io_config);
        I2C_BSR_ACQUIRE(ctrl_config->id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 : SYS_BSR_PERIPH_ID_I2C2);
#else
        ad_i2c_io_conf_t *last = &i2c_last_io_config;
        I2C_BSR_ACQUIRE(SYS_BSR_PERIPH_ID_I2C1);
#endif

        ASSERT_WARNING(ctrl_config->drv);

#if (HW_I2C_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = ctrl_config->drv->dma_channel;
#endif /* HW_I2C_DMA_SUPPORT */

        I2C_RES_ACQUIRE(ctrl_config->id, dma_channel);

        ASSERT_WARNING(ctrl_config->io);
        hw_sys_pd_com_enable();
        if (ad_io_configure(&ctrl_config->io->scl, AD_I2C_IO_SIZE, AD_IO_CONF_ON) != AD_IO_ERROR_NONE) {

                release_ad_i2c_open_resources(i2c, ctrl_config);
                return NULL;
        }
        if (last->scl.port < HW_GPIO_PORT_MAX && last->scl.pin < HW_GPIO_PIN_MAX &&
            last->sda.port < HW_GPIO_PORT_MAX && last->sda.pin < HW_GPIO_PIN_MAX) {
                if (last->scl.port != ctrl_config->io->scl.port ||
                    last->scl.pin != ctrl_config->io->scl.pin) {
                        ad_io_configure(&(last->scl), 1, AD_IO_CONF_OFF);
                        ad_io_set_pad_latch(&(last->scl), 1, AD_IO_PAD_LATCHES_OP_TOGGLE);
                }
                if (last->sda.port != ctrl_config->io->sda.port ||
                    last->sda.pin != ctrl_config->io->sda.pin) {
                        ad_io_configure(&(last->sda), 1, AD_IO_CONF_OFF);
                        ad_io_set_pad_latch(&(last->sda), 1, AD_IO_PAD_LATCHES_OP_TOGGLE);
                }
        }
        ad_io_set_pad_latch(&ctrl_config->io->scl, AD_I2C_IO_SIZE,
                                                AD_IO_PAD_LATCHES_OP_ENABLE);

        i2c->conf = ctrl_config;
        ASSERT_WARNING(ctrl_config->drv);

        /* Configure I2C controller driver */
        if (ad_i2c_reconfig(i2c, ctrl_config->drv) != AD_I2C_ERROR_NONE) {
                *last = *(i2c->conf->io);
                ad_io_set_pad_latch(&i2c->conf->io->scl, AD_I2C_IO_SIZE,
                                                       AD_IO_PAD_LATCHES_OP_DISABLE);
                release_ad_i2c_open_resources(i2c, ctrl_config);
                return NULL;
        }
        i2c->current_drv = (ad_i2c_driver_conf_t *)ctrl_config->drv;
#if (CONFIG_AD_I2C_LOCKING == 1)
        i2c->owner = OS_GET_CURRENT_TASK();
#endif

        return i2c;
}

int ad_i2c_close(ad_i2c_handle_t p, bool force)
{
        if (!(AD_I2C_HANDLE_IS_VALID(p))) {
                OS_ASSERT(0);
                return AD_I2C_ERROR_HANDLE_INVALID;
        }
        ad_i2c_dynamic_data_t *i2c = (ad_i2c_dynamic_data_t *)p;
        HW_I2C_ID id = i2c->conf->id;

#if (HW_I2C_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = i2c->conf->drv->dma_channel;
#endif

        OS_ENTER_CRITICAL_SECTION();
        if (!force) {
                if (ad_i2c_controller_is_busy(i2c->conf->id)) {
                        OS_LEAVE_CRITICAL_SECTION();
                        return AD_I2C_ERROR_CONTROLLER_BUSY;
                }
        } else {
                /* Abort ongoing transactions */
                if (hw_i2c_is_master(i2c->conf->id) && ad_i2c_controller_is_busy(i2c->conf->id)) {
                        if (!ad_i2c_master_abort_transfer(i2c->conf->id)) {
                                OS_LEAVE_CRITICAL_SECTION();
                                return AD_I2C_ERROR_CONTROLLER_ABORT_FAIL;
                        }
                }
                hw_i2c_unregister_int(id);
#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
                clear_transact_data(i2c);
#endif
        }

#if defined(HW_I2C3)
        ad_i2c_io_conf_t *last_io = ((id == HW_I2C1) ? &i2c_last_io_config :
                                     (id == HW_I2C2) ? &i2c2_last_io_config : &i2c3_last_io_config);
#elif defined(HW_I2C2)
        ad_i2c_io_conf_t *last_io = ((id == HW_I2C1) ? &i2c_last_io_config : &i2c2_last_io_config);
#else
        ad_i2c_io_conf_t *last_io = &i2c_last_io_config;
#endif
        hw_i2c_deinit(id);
#if (HW_I2C_DMA_SUPPORT == 1)
        if (dma_channel < HW_DMA_CHANNEL_INVALID - 1) {
                hw_dma_channel_stop(i2c->conf->drv->dma_channel);
                hw_dma_channel_stop(i2c->conf->drv->dma_channel + 1);
        }
#endif /* HW_I2C_DMA_SUPPORT */
        /* Don't deconfigure pins, just keep the last configuration */
        *last_io = *(i2c->conf->io);
        ad_io_set_pad_latch(&i2c->conf->io->scl, AD_I2C_IO_SIZE,
                                                AD_IO_PAD_LATCHES_OP_DISABLE);
        hw_sys_pd_com_disable();
#if (CONFIG_AD_I2C_LOCKING == 1)
        i2c->owner = NULL;
#endif
        i2c->current_drv = NULL;
        i2c->conf = NULL;

        OS_LEAVE_CRITICAL_SECTION();

        I2C_RES_RELEASE(id, dma_channel);
#if defined(HW_I2C3)
        I2C_BSR_RELEASE(id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 :
                        id == HW_I2C2 ? SYS_BSR_PERIPH_ID_I2C2 : SYS_BSR_PERIPH_ID_I2C3);
#elif defined(HW_I2C2)
        I2C_BSR_RELEASE(id == HW_I2C1 ? SYS_BSR_PERIPH_ID_I2C1 : SYS_BSR_PERIPH_ID_I2C2);
#else
        I2C_BSR_RELEASE(SYS_BSR_PERIPH_ID_I2C1);
#endif
        pm_sleep_mode_release(pm_mode_idle);

        return AD_I2C_ERROR_NONE;
}

void ad_i2c_init(void)
{
#if CONFIG_I2C_USE_SYNC_TRANSACTIONS
        OS_EVENT_CREATE(i2c_static_data.event);
#if defined(HW_I2C2)
        OS_EVENT_CREATE(i2c2_static_data.event);
#endif
#if defined(HW_I2C3)
        OS_EVENT_CREATE(i2c3_static_data.event);
#endif
#endif
        I2C_MUTEX_CREATE(i2c_static_data.busy);
#if defined(HW_I2C2)
        I2C_MUTEX_CREATE(i2c2_static_data.busy);
#endif
#if defined(HW_I2C3)
        I2C_MUTEX_CREATE(i2c3_static_data.busy);
#endif
}

ADAPTER_INIT(ad_i2c_adapter, ad_i2c_init);

#endif /* dg_configI2C_ADAPTER */
