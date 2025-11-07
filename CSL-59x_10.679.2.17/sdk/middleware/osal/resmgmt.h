/**
 * \addtogroup MID_RTO_OSAL
 * \{
 * \addtogroup MID_RTO_OSAL_RES_MGMT
 *
 * \brief OSAL resource management
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file resmgmt.h
 *
 * @brief Resource management API
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

#ifndef RESMGMT_H_
#define RESMGMT_H_

#include <stdint.h>
#include "osal.h"

/*
 * Use large resource id (optional, disabled by default) for applications with many devices
 * used (and thus many extra resource ids needed).
 */
#ifndef CONFIG_LARGE_RESOURCE_ID
#define CONFIG_LARGE_RESOURCE_ID (0)
#endif

/**
 * Data type used for managing devices
 */
#if !CONFIG_LARGE_RESOURCE_ID
typedef uint32_t resource_mask_t;
#else
typedef uint64_t resource_mask_t;
#endif

/**
 * \brief Shared resource ids
 *
 * \sa RES_MASK
 *
 */
typedef enum {
        RES_ID_UART1,
        RES_ID_UART2,
        RES_ID_UART1_CONFIG,
        RES_ID_UART2_CONFIG,
        RES_ID_UART1_READ,
        RES_ID_UART2_READ,
        RES_ID_UART1_WRITE,
        RES_ID_UART2_WRITE,
        RES_ID_SPI1,
        RES_ID_I2C1,
        RES_ID_QSPI,
        RES_ID_TIMER0,
        RES_ID_TIMER1,
        RES_ID_TIMER2,
        RES_ID_DMA_CH0,
        RES_ID_DMA_CH1,
        RES_ID_DMA_CH2,
        RES_ID_DMA_CH3,
        RES_ID_DMA_CH4,
        RES_ID_DMA_CH5,
        RES_ID_DMA_CH_SECURE = RES_ID_DMA_CH5,
        RES_ID_GPADC,
        RES_ID_SDADC,
        RES_ID_SRC1,
        RES_ID_SRC2,
        RES_ID_COUNT
} RES_ID;

/**
 * \brief Make resource mask from ID
 *
 * Use this macro to prepare argument for resource_acquire().
 *
 * \param [in] id value from RES_IS enum
 *
 * \sa resource_acquire
 *
 */
#define RES_MASK(id) (((resource_mask_t) 1) << (id))

/**
 * \brief Constant to use when resource_acquire() should wait till resource is available
 *
 */
#define RES_WAIT_FOREVER OS_EVENT_FOREVER

/**
 * \brief Initialize resource management structures
 *
 * Function allocates internal structure so resource allocation can be done
 * on OS level.
 * This function must be called before any calls to resource_acquire(), resource_release().
 *
 * \sa resource_acquire
 * \sa resource_release
 *
 */
void resource_init(void);

/**
 * \brief Acquire resource(s)
 *
 * Function acquires resource(s) so they can be accessed exclusively.
 *
 * \param [in] resource_mask bit mask of requested resource
 *             it can have single resource:
 *                RES_MASK(RES_ID_UART1)
 *             or group of them:
 *                RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2) | RES_MASK(RES_ID_I2C1)
 * \param [in] timeout how long to wait for resources to be available
 *             0 - no wait take resource if is available
 *             RES_WAIT_FOREVER - wait till all resources are available
 *             other value specifies how many ticks to wait for resources
 *
 * \return \p  acquired_resources mask  on success where the requested resource_mask is included,
 *             0                        on timeout or failure to acquire the resources
 *
 * \sa resource_release
 *
 */
resource_mask_t resource_acquire(resource_mask_t resource_mask, OS_TICK_TIME timeout);

/**
 * \brief Release resource(s)
 *
 * Function releases resources so they can be used by other tasks.
 * If there is task waiting for resources just released it will be
 * scheduled to run (provided that all requested resources are free).
 *
 * It's possible to acquire resources in one resource_acquire() call,
 * and then release them separately.
 *   resource_acquire(RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2);
 * ...
 *   resource_release(RES_MASK(RES_ID_UART1);
 *   resource_release(RES_MASK(RES_ID_SPI2);
 *
 * \param [in] resource_mask bit mask of released resources
 *             it can have single resource:
 *                RES_MASK(RES_ID_UART1)
 *             or group of them:
 *                RES_MASK(RES_ID_UART1) | RES_MASK(RES_ID_SPI2) | RES_MASK(RES_ID_I2C1)
 *
 * \sa resource_acquire
 *
 */
void resource_release(resource_mask_t resource_mask);

#ifndef CONFIG_NO_DYNAMIC_RESOURCE_ID

/**
 * \brief Add resource at run time
 *
 * Resources that can be acquired are defined in RES_ID enum. This creates pool of resources at
 * compile time.
 * It allows to add resources to extend this list at run time. This does not
 * affects resource management functions, it just allows to create resource ids in a safe way
 * without changing enum in this file.
 * When all resources needed by the user are identified this feature can be switch off and all
 * dynamic resource ids can be put in enum.
 *
 */
int resource_add(void);

#endif

#endif /* RESMGMT_H_ */

/**
 * \}
 * \}
 */
