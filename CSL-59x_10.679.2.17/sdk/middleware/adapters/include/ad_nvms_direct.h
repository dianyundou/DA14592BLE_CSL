/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup NVMS_ADAPTER
 * \{
 * \addtogroup NVMS_DIRECT NVMS Direct Driver
 *
 * \brief NVMS direct driver
 *
 * NVMS direct driver allows to write to flash without explicit erase.
 * To achieve this, driver has to have RAM memory buffer to hold flash sector in case erase
 * is needed.
 * For flexibility of driver, it's possible to configure how memory needed for sector is handled.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_nvms_direct.h
 *
 * @brief NVMS direct access driver API
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AD_NVMS_DIRECT_H_
#define AD_NVMS_DIRECT_H_

#if dg_configNVMS_ADAPTER

#include <ad_nvms.h>

/*
 * 1. Dynamic sector buffer allocates memory when write changes data in a way that erase
 * is required. After writing data RAM is freed. In low memory condition this can lead to write
 * failure.
 * 2. Static sector buffer, in this case driver keeps sector size memory buffer in no retention
 * RAM all the time. Memory is always available.
 * 3. No sector buffer, write will fail if sector is not manually erased before.
 */
#define DIRECT_DRIVER_DYNAMIC_SECTOR_BUF 1
#define DIRECT_DRIVER_STATIC_SECTOR_BUF  2
#define DIRECT_DRIVER_NO_SECTOR_BUF      3

extern const partition_driver_t ad_nvms_direct_driver;

/**
 * \brief Initialize NVMS direct access driver
 *
 */
void ad_nvms_direct_init(void);

#endif /* dg_configNVMS_ADAPTER */

#endif /* AD_NVMS_DIRECT_H_ */

/**
 \}
 \}
 \}
 */
