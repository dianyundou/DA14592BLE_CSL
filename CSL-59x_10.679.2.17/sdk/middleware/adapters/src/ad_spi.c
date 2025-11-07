/**
 ****************************************************************************************
 *
 * @file ad_spi.c
 *
 * @brief SPI Adapter implementation
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

#if dg_configSPI_ADAPTER

#include <stdint.h>
#include <string.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "resmgmt.h"
#include "hw_clk.h"
#if (HW_SPI_DMA_SUPPORT == 1)
#include "hw_dma.h"
#endif
#include "hw_gpio.h"
#include "hw_sys.h"
#include "sys_power_mgr.h"

#include "ad_spi.h"
#include "hw_spi.h"

/**
 * \def CONFIG_AD_SPI_LOCKING
 *
 * \brief Controls whether SPI adapter resource locking will be enabled
 *
 * By default, the SPI adapter internally handles concurrent accesses on an SPI bus by different masters
 * and tasks. If resource locking is disabled by setting this macro to 0, all such internal handling
 * is disabled, thus becoming the application's responsibility to handle concurrent accesses, using
 * the busy status register (BSR) driver and controlling the resource management.
 */
#ifndef CONFIG_AD_SPI_LOCKING
# define CONFIG_AD_SPI_LOCKING                  ( 1 )
#endif /* CONFIG_AD_SPI_LOCKING */

/*
 * Resource allocation functions
 */
#if (CONFIG_AD_SPI_LOCKING == 1)
# define SPI_MUTEX_CREATE(mutex)                do { \
                                                        OS_ASSERT((mutex) == NULL); \
                                                        OS_MUTEX_CREATE(mutex); \
                                                        OS_ASSERT(mutex); \
                                                } while (0)
# define SPI_MUTEX_GET(mutex)                   do { \
                                                        OS_ASSERT(mutex); \
                                                        OS_MUTEX_GET((mutex), OS_MUTEX_FOREVER); \
                                                } while (0)
# define SPI_MUTEX_PUT(mutex)                   OS_MUTEX_PUT(mutex)

# define SPI_BSR_ACQUIRE(periph_id)             do {} while (0)
# define SPI_BSR_RELEASE(periph_id)             do {} while (0)

# if (HW_SPI_DMA_SUPPORT == 1)
#   define SPI_RES_ACQUIRE(res_id, dma_chan)    ad_spi_res_acquire((res_id), (dma_chan))
#   define SPI_RES_RELEASE(res_id, dma_chan)    ad_spi_res_release((res_id), (dma_chan))
# else
#   define SPI_RES_ACQUIRE(res_id, dma_chan)    ad_spi_res_acquire((res_id))
#   define SPI_RES_RELEASE(res_id, dma_chan)    ad_spi_res_release((res_id))
# endif

#else
# define SPI_MUTEX_CREATE(mutex)                do {} while (0)
# define SPI_MUTEX_GET(mutex)                   do {} while (0)
# define SPI_MUTEX_PUT(mutex)                   do {} while (0)

# define SPI_RES_ACQUIRE(res_id, dma_channel)   do {} while (0)
# define SPI_RES_RELEASE(res_id, dma_channel)   do {} while (0)
#endif /* CONFIG_AD_SPI_LOCKING */


#define AD_SPI_IO_SIZE          (3)

/**
 * \brief Local alias for dg_configSPI_ADAPTER_CS_MAX
 */
#define SPI_CS_MAX                              dg_configSPI_ADAPTER_CS_MAX

#if (SPI_CS_MAX >= UINT8_MAX)
#error "Software limits exceeded."
#else
#define SPI_CS_INVALID                          UINT8_MAX
#endif

/**
 * Adapter static data
 */
typedef struct {
        /**< SPI controller configuration - NON-volatile */
        const ad_spi_controller_conf_t *conf;
        /**< Local copy of current SPI driver configuration - volatile */
        ad_spi_driver_conf_t drv_current;
        /**< Internal data */
#if (CONFIG_AD_SPI_LOCKING == 1)
        OS_TASK  owner; /**< The task which opened the controller */
#endif
#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT event; /**< Semaphore for async calls  */
#endif
#if (CONFIG_AD_SPI_LOCKING == 1)
        OS_MUTEX busy;  /**< Semaphore for thread safety */
#endif
} ad_spi_data_t;

#ifdef HW_SPI3
__RETAINED static ad_spi_data_t spi3_data;
#endif
#ifdef HW_SPI2
__RETAINED static ad_spi_data_t spi2_data;
#endif
__RETAINED static ad_spi_data_t spi1_data;

#if defined(HW_SPI3)
#define AD_SPI_HANDLE_IS_VALID(__handle) (((__handle == &spi1_data) || (__handle == &spi2_data) || (__handle == &spi3_data)) && (((ad_spi_data_t*) __handle)->conf != NULL))
#elif defined(HW_SPI2)
#define AD_SPI_HANDLE_IS_VALID(__handle) (((__handle == &spi1_data) || (__handle == &spi2_data)) && (((ad_spi_data_t*) __handle)->conf != NULL))
#else
#define AD_SPI_HANDLE_IS_VALID(__handle) ((__handle == &spi1_data) && (((ad_spi_data_t*) __handle)->conf != NULL))
#endif

/**
 * Adapter functions
 */
void ad_spi_init(void)
{
        SPI_MUTEX_CREATE(spi1_data.busy);
#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT_CREATE(spi1_data.event);
#endif
#ifdef HW_SPI2
        SPI_MUTEX_CREATE(spi2_data.busy);
#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT_CREATE(spi2_data.event);
#endif
#endif /* HW_SPI2 */
#ifdef HW_SPI3
        SPI_MUTEX_CREATE(spi3_data.busy);
#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)
        OS_EVENT_CREATE(spi3_data.event);
#endif
#endif /* HW_SPI3 */
}

#if (CONFIG_AD_SPI_LOCKING == 1)
#if (HW_SPI_DMA_SUPPORT == 1)
static void ad_spi_res_acquire(RES_ID id, HW_DMA_CHANNEL dma_channel)
#else
static void ad_spi_res_acquire(RES_ID id)
#endif /* HW_SPI_DMA_SUPPORT */
{
#if (HW_SPI_DMA_SUPPORT == 1)
        /* The resource is acquired via the low channel.
         * The high channel is checked for validity.
         */
        if (dma_channel + 1 < HW_DMA_CHANNEL_INVALID) {
                resource_acquire(RES_MASK(id) | RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1), RES_WAIT_FOREVER);
        }
        else
#endif /* HW_SPI_DMA_SUPPORT */
        {
                resource_acquire(RES_MASK(id), RES_WAIT_FOREVER);
        }
}

#if (HW_SPI_DMA_SUPPORT == 1)
static void ad_spi_res_release(RES_ID id, HW_DMA_CHANNEL dma_channel)
#else
static void ad_spi_res_release(RES_ID id)
#endif /* HW_SPI_DMA_SUPPORT */
{
#if (HW_SPI_DMA_SUPPORT == 1)
        /* The resource is acquired via the low channel.
         * The high channel is checked for validity.
         */
        if (dma_channel + 1 < HW_DMA_CHANNEL_INVALID) {
                resource_release(RES_MASK(id) | RES_MASK(RES_ID_DMA_CH0 + dma_channel) |
                        RES_MASK(RES_ID_DMA_CH0 + dma_channel + 1));
        }
        else
#endif /* HW_SPI_DMA_SUPPORT */
        {
                resource_release(RES_MASK(id));
        }
}
#endif /* CONFIG_AD_SPI_LOCKING */

static bool config_io(const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state)
{
        uint8_t ret = AD_IO_ERROR_NONE;
        OS_ASSERT(io->cs_cnt < SPI_CS_INVALID);

        if (!AD_IO_PIN_PORT_VALID(io->spi_do.port, io->spi_do.pin)) {
                /* assume only CLK and DI are needed */
                ret |= ad_io_configure(&io->spi_clk, 2, state);
        } else if (!AD_IO_PIN_PORT_VALID(io->spi_di.port, io->spi_di.pin)) {
                /* assume only CLK and DO are needed */
                ret |= ad_io_configure(&io->spi_do, 2, state);
        } else {
                ret |= ad_io_configure(&io->spi_do, AD_SPI_IO_SIZE, state);
        }

        ad_io_conf_t *cs = io->spi_cs;
        uint8_t i = io->cs_cnt;
        while (i--) {
                ret |= ad_io_configure(cs, 1, state);
                cs++;
        }

        return (AD_IO_ERROR_NONE == ret);
}

static void set_pad_latches(const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state)
{
        if (!AD_IO_PIN_PORT_VALID(io->spi_do.port, io->spi_do.pin)) {
                /* assume only CLK and DI are needed */
                ad_io_set_pad_latch(&io->spi_clk, 2, state);
        } else if (!AD_IO_PIN_PORT_VALID(io->spi_di.port, io->spi_di.pin)) {
                /* assume only CLK and DO are needed */
                ad_io_set_pad_latch(&io->spi_do, 2, state);
        } else {
                ad_io_set_pad_latch(&io->spi_do, AD_SPI_IO_SIZE, state);
        }

        ad_io_conf_t *cs = io->spi_cs;
        uint8_t i = io->cs_cnt;
        while (i--) {
                ad_io_set_pad_latch(cs, 1, state);
                cs++;
        }
}

__STATIC_INLINE RES_ID get_res_id()
{
#if defined(HW_SPI3)
        RES_ID res_id = ((id == HW_SPI1) ? RES_ID_SPI1 : ((id == HW_SPI2) ? RES_ID_SPI2 : RES_ID_SPI3));
#elif defined(HW_SPI2)
        RES_ID res_id = ((id == HW_SPI1) ? RES_ID_SPI1 : RES_ID_SPI2);
#else
        RES_ID res_id = RES_ID_SPI1;
#endif
        return res_id;
}

static HW_SPI_CS_MODE gpio_func_to_cs_mode(HW_GPIO_FUNC func)
{
        switch (func) {
        case HW_GPIO_FUNC_SPI_EN:
                return HW_SPI_CS_0;
        case HW_GPIO_FUNC_SPI_EN2:
                return HW_SPI_CS_1;
        case HW_GPIO_FUNC_GPIO:
                return HW_SPI_CS_GPIO;
        default:
                return HW_SPI_CS_NONE;
        }
}

ad_spi_handle_t ad_spi_open(const ad_spi_controller_conf_t *conf)
{
        ad_spi_data_t *spi;

        OS_ASSERT(conf);
        OS_ASSERT(conf->drv);
        OS_ASSERT(conf->io);

        if (conf->id == HW_SPI1) {
                spi = &spi1_data;
#ifdef HW_SPI2
        } elif (conf->id == HW_SPI2) {
                spi = &spi2_data;
#endif /* HW_SPI2 */
#ifdef HW_SPI3
        } elif (conf->id == HW_SPI3) {
                spi = &spi3_data;
#endif
        } else {
                OS_ASSERT(0);
        }

        pm_sleep_mode_request(pm_mode_idle);

#if (CONFIG_AD_SPI_LOCKING == 1)
        RES_ID res_id = get_res_id();
#endif

#if (HW_SPI_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = HW_DMA_CHANNEL_INVALID;
        if (conf->drv->spi.use_dma) {
                dma_channel = conf->drv->spi.rx_dma_channel;
                OS_ASSERT(dma_channel < HW_DMA_CHANNEL_INVALID - 1)
        }
#endif


        SPI_RES_ACQUIRE(res_id, dma_channel);

        hw_sys_pd_com_enable();

        if (!config_io(conf->io, AD_IO_CONF_ON)) {
                SPI_RES_RELEASE(res_id, dma_channel);

                /* Configure the GPIOs for the desired OFF state.
                 * Some of them might have been configured in the process of
                 * applying the IO configuration settings.
                 * Checking the return for error is not needed,
                 * since we cannot return anything different
                 * than NULL to indicate a problem */
                config_io(conf->io, AD_IO_CONF_OFF);

                hw_sys_pd_com_disable();

                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

#if (CONFIG_AD_SPI_LOCKING == 1)
        spi->owner = OS_GET_CURRENT_TASK();
#endif

        spi->conf = conf;

        if (ad_spi_reconfig(spi, conf->drv) != AD_SPI_ERROR_NONE) {
                ASSERT_WARNING(0);
                /* If the controller is not in use, then make sure
                 * it is turned OFF */
                hw_spi_enable(conf->id, 0);

                /* Configure the GPIOs for the desired OFF state.
                 * Checking the return for error is not needed,
                 * since we cannot return anything different
                 * than NULL to indicate a problem */
                config_io(conf->io, AD_IO_CONF_OFF);

                spi->conf = NULL;
                memset(&(spi->drv_current.spi), 0, sizeof(hw_spi_config_t));
                SPI_RES_RELEASE(res_id, dma_channel);

                hw_sys_pd_com_disable();

                pm_sleep_mode_release(pm_mode_idle);
                return NULL;
        }

        /* Everything is configured and ready.
         * Now it is safe to enable the Latches. */
        set_pad_latches(conf->io, AD_IO_PAD_LATCHES_OP_ENABLE);
        return spi;
}

static AD_SPI_ERROR ad_spi_validate_driver_config(ad_spi_handle_t handle, const ad_spi_driver_conf_t *drv_conf)
{
        const ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (drv_conf == NULL) {
                return AD_SPI_ERROR_DRIVER_CONF_INVALID;
        }

#if (HW_SPI_DMA_SUPPORT == 1)
        if (spi->conf->drv->spi.rx_dma_channel != drv_conf->spi.rx_dma_channel) {
                return AD_SPI_ERROR_CONFIG_DMA_CHANNEL_INVALID;
        }

        if (spi->conf->drv->spi.tx_dma_channel != drv_conf->spi.tx_dma_channel) {
                return AD_SPI_ERROR_CONFIG_DMA_CHANNEL_INVALID;
        }
#endif /* HW_SPI_DMA_SUPPORT */

        if (spi->conf->drv->spi.smn_role != drv_conf->spi.smn_role) {
                return AD_SPI_ERROR_CONFIG_SPI_ROLE_INVALID;
        }

        /* Check clock divider consistency */
        const HW_SPI_FREQ spi_clk_div = drv_conf->spi.xtal_freq;
        if ((hw_clk_get_sysclk() == SYS_CLK_IS_DBLR) && (REG_GETF(CRG_COM, CLK_COM_REG, SPI_CLK_SEL) == 1)) {
                if (spi_clk_div >= SPI_SPI_CLOCK_REG_SPI_CLK_DIV_Msk) {
                        return AD_SPI_ERROR_DRIVER_CLOCK_DIV_INVALID;
                }
        } else {
                if (spi_clk_div > SPI_SPI_CLOCK_REG_SPI_CLK_DIV_Msk) {
                        return AD_SPI_ERROR_DRIVER_CLOCK_DIV_INVALID;
                }
        }

        /* Check Rx/Tx FIFO threshold level */
#if HW_SPI_DMA_SUPPORT
        if ((drv_conf->spi.use_dma) && (drv_conf->spi.rx_tl != HW_SPI_FIFO_LEVEL0)) {
                return AD_SPI_ERROR_CONFIG_RX_TX_TL_INVALID;
        }
#else
        if ((drv_conf->spi.rx_tl > HW_SPI_FIFO_LEVEL4) || (drv_conf->spi.tx_tl > HW_SPI_FIFO_LEVEL4)) {
                return AD_SPI_ERROR_CONFIG_RX_TX_TL_INVALID;
        }
#endif /* HW_SPI_DMA_SUPPORT */

        /* Validate the CS configuration of the first CS pin of the list */
        ad_io_conf_t *cs_in_io = &(spi->conf->io->spi_cs[0]);
        HW_SPI_CS_MODE io_cs_mode = gpio_func_to_cs_mode(cs_in_io->on.function);

        /* CS is controlled by master - NONE is not an option for the I/O configured CS pad */
        if (drv_conf->spi.smn_role == HW_SPI_MODE_MASTER && io_cs_mode == HW_SPI_CS_NONE) {
                return AD_SPI_ERROR_CONFIG_SPI_CS_INVALID;
        }

        return AD_SPI_ERROR_NONE;
}

int ad_spi_reconfig(ad_spi_handle_t handle, const ad_spi_driver_conf_t *drv_conf)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;
        AD_SPI_ERROR conf_result;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        SPI_MUTEX_GET(spi->busy);

        /* Reconfigure must follow a previous call to ad_spi_open */
        OS_ASSERT(spi->conf->drv);

        /* Validate */
        conf_result = ad_spi_validate_driver_config(handle, drv_conf);
        if (conf_result != AD_SPI_ERROR_NONE) {
                SPI_MUTEX_PUT(spi->busy);
                return conf_result;
        }

        /* Check if SPI is occupied */
        if (hw_spi_get_clock_en(spi->conf->id)) {
                if (hw_spi_is_occupied(spi->conf->id)) {
                        SPI_MUTEX_PUT(spi->busy);
                        return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
                }
        }

        /* Update driver configuration in static adapter data... */
        memcpy(&(spi->drv_current.spi), &(drv_conf->spi), sizeof(hw_spi_config_t));
        /* ...with the first CS info from the adapter I/O list */
        ad_io_conf_t *ad_cs = &(spi->conf->io->spi_cs[0]);
        spi->drv_current.spi.cs_pad.port = ad_cs->port;
        spi->drv_current.spi.cs_pad.pin = ad_cs->pin;
        spi->drv_current.spi.spi_cs = gpio_func_to_cs_mode(ad_cs->on.function);

        /* Initialize the interface */
        hw_spi_init(spi->conf->id, &(spi->drv_current.spi));

        SPI_MUTEX_PUT(spi->busy);

        return AD_SPI_ERROR_NONE;
}

int ad_spi_close(ad_spi_handle_t handle, bool force)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        HW_SPI_ID id = spi->conf->id;
#if (CONFIG_AD_SPI_LOCKING == 1)
        RES_ID res_id = get_res_id();
#endif /* CONFIG_AD_SPI_LOCKING == 1*/

#if (HW_SPI_DMA_SUPPORT == 1)
        HW_DMA_CHANNEL dma_channel = HW_DMA_CHANNEL_INVALID;
        if (spi->drv_current.spi.use_dma) {
                dma_channel = spi->drv_current.spi.rx_dma_channel;
                OS_ASSERT(dma_channel < HW_DMA_CHANNEL_INVALID - 1);
        }
#endif

        /* check for ongoing transactions */

        if (!force && hw_spi_is_occupied(id)) {
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_deinit(id);

        if (!config_io(spi->conf->io, AD_IO_CONF_OFF)) {
                return AD_SPI_ERROR_IO_CFG_INVALID;
        }
        set_pad_latches(spi->conf->io, AD_IO_PAD_LATCHES_OP_DISABLE);
        hw_sys_pd_com_disable();
#if (CONFIG_AD_SPI_LOCKING == 1)
        spi->owner = NULL;
#endif
        spi->conf = NULL;
        memset(&(spi->drv_current.spi), 0, sizeof(hw_spi_config_t));
        SPI_RES_RELEASE(res_id, dma_channel);

        pm_sleep_mode_release(pm_mode_idle);
        return AD_SPI_ERROR_NONE;
}

static void switch_to_cs(ad_spi_data_t *spi, uint8_t csid)
{
        OS_ASSERT(csid < spi->conf->io->cs_cnt);

        ad_io_conf_t *new_cs = &(spi->conf->io->spi_cs[csid]);
        HW_SPI_CS_MODE new_mode = gpio_func_to_cs_mode(new_cs->on.function);

        SPI_Pad current_pad = spi->drv_current.spi.cs_pad;
        /* Re-configure the CS pad, if something has changed */
        if (current_pad.port != new_cs->port || current_pad.pin != new_cs->pin || spi->drv_current.spi.spi_cs != new_mode ) {

                /* Update the driver section of the adapter configuration */
                spi->drv_current.spi.cs_pad.port = new_cs->port;
                spi->drv_current.spi.cs_pad.pin = new_cs->pin;
                spi->drv_current.spi.spi_cs = new_mode;

                /* Update the driver CS pad */
                hw_spi_set_cs_pad(spi->conf->id, &(spi->drv_current.spi.cs_pad), new_mode, true);
        }
}

void ad_spi_activate_cs(ad_spi_handle_t handle, uint8_t csid)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;
        const HW_SPI_ID id = spi->conf->id;

        /* The task must own the controller */
        OS_ASSERT(id);

        if (!hw_spi_is_slave(id)) {
                switch_to_cs(spi, csid);
                hw_spi_set_cs_low(id);
        }
}

void ad_spi_deactivate_cs(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;
        const HW_SPI_ID id = spi->conf->id;

        /* The task must own the controller */
        OS_ASSERT(id);

        if (!hw_spi_is_slave(id)) {
                hw_spi_set_cs_high(id);
        }
}

void ad_spi_deactivate_cs_when_spi_done(ad_spi_handle_t handle)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;
        const HW_SPI_ID id = spi->conf->id;

        /* The task must own the controller */
        OS_ASSERT(id);

        hw_spi_wait_while_busy(id);

        ad_spi_deactivate_cs(handle);
}

#if (CONFIG_SPI_USE_SYNC_TRANSACTIONS == 1)

static void ad_spi_wait_event(void *p, uint16_t transferred)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) p;
        OS_EVENT_SIGNAL_FROM_ISR(spi->event);
}

int ad_spi_write(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen)
{

        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_write_buf(id, wbuf, wlen, ad_spi_wait_event, spi);

        OS_EVENT_WAIT(spi->event, OS_EVENT_FOREVER);

        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_read(ad_spi_handle_t handle, uint8_t *rbuf, size_t rlen)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_read_buf(id, rbuf, rlen, ad_spi_wait_event, spi);

        OS_EVENT_WAIT(spi->event, OS_EVENT_FOREVER);

        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_write_read(ad_spi_handle_t handle, const uint8_t *wbuf, uint8_t *rbuf, size_t len)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_writeread_buf(id, wbuf, rbuf, len, ad_spi_wait_event, spi);

        OS_EVENT_WAIT(spi->event, OS_EVENT_FOREVER);

        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

#endif /* CONFIG_SPI_USE_SYNC_TRANSACTIONS */

#if (CONFIG_SPI_USE_ASYNC_TRANSACTIONS == 1)

int ad_spi_write_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen,
                                                ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_write_buf(id, wbuf, wlen, cb, user_data);
        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_read_async(ad_spi_handle_t handle, const uint8_t *rbuf, size_t rlen,
                                                ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_read_buf(id, (uint8_t *)rbuf, (uint16_t)rlen, cb, user_data);
        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}

int ad_spi_write_read_async(ad_spi_handle_t handle, const uint8_t *wbuf, size_t wlen,
                         uint8_t *rbuf, size_t rlen, ad_spi_user_cb cb, void *user_data)
{
        ad_spi_data_t *spi = (ad_spi_data_t *) handle;

        if (!AD_SPI_HANDLE_IS_VALID(handle)) {
                OS_ASSERT(0);
                return AD_SPI_ERROR_HANDLE_INVALID;
        }

        const HW_SPI_ID id = spi->conf->id;

        SPI_MUTEX_GET(spi->busy);

        if (hw_spi_is_occupied(id)) {
                SPI_MUTEX_PUT(spi->busy);
                return AD_SPI_ERROR_TRANSF_IN_PROGRESS;
        }

        hw_spi_writeread_buf(id, wbuf, rbuf, wlen, cb, user_data);
        SPI_MUTEX_PUT(spi->busy);
        return AD_SPI_ERROR_NONE;
}
#endif /* CONFIG_SPI_USE_ASYNC_TRANSACTIONS */

int ad_spi_io_config(HW_SPI_ID id, const ad_spi_io_conf_t *io, AD_IO_CONF_STATE state)
{
        /* SPI clk should be at least configured*/
        if (!AD_IO_PIN_PORT_VALID(io->spi_clk.port, io->spi_clk.pin)) {
                return AD_SPI_ERROR_NO_SPI_CLK_PIN;
        }

        hw_sys_pd_com_enable();

        if (!config_io(io, state)) {
                return AD_SPI_ERROR_IO_CFG_INVALID;
        }

        set_pad_latches(io, AD_IO_PAD_LATCHES_OP_TOGGLE);
        hw_sys_pd_com_disable();
        return AD_SPI_ERROR_NONE;
}

ADAPTER_INIT(ad_spi_adapter, ad_spi_init);

#endif /* dg_configSPI_ADAPTER */
