/**
****************************************************************************************
*
* @file sys_bsr.c
*
* @brief Busy Status Register (BSR) driver
*
* Copyright (C) 2018-2020 Renesas Electronics Corporation and/or its affiliates.
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

#include "sdk_defs.h"
#include "hw_sys.h"
#include "sys_bsr.h"
#ifndef OS_BAREMETAL
#include "osal.h"
#endif /* OS_BAREMETAL */

#ifndef OS_BAREMETAL
__RETAINED static OS_MUTEX sys_sw_bsr_mutex;
#endif

void sys_sw_bsr_init(void)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_CREATE(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

void sys_sw_bsr_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        while (!hw_sys_sw_bsr_try_acquire(sw_bsr_master_id, periph_id)) {
        }
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

bool sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        bool acquired;
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        acquired = hw_sys_sw_bsr_acquired(sw_bsr_master_id, periph_id);
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
        return acquired;
}

void sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        hw_sys_sw_bsr_release(sw_bsr_master_id, periph_id);
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

