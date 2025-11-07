/*
 * Copyright (C) 2018-2023 Renesas Electronics Corporation and/or its affiliates.
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
 */

#ifndef FREERTOS_TASKS_C_ADDITIONS_H
#define FREERTOS_TASKS_C_ADDITIONS_H

#include <stdint.h>

#if (configUSE_TRACE_FACILITY == 0)
#error "configUSE_TRACE_FACILITY must be enabled"
#endif

#define FREERTOS_DEBUG_CONFIG_MAJOR_VERSION 1
#define FREERTOS_DEBUG_CONFIG_MINOR_VERSION 2

#define configFRTOS_MEMORY_SCHEME     CONFIG_FREERTOS_HEAP_ALGO

#ifdef __cplusplus
extern "C" {
#endif

#if (tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 2)
// Need the portARCH_NAME define
#ifndef portARCH_NAME
#define portARCH_NAME NULL
#endif
__RETAINED_RW char * portArch_Name = portARCH_NAME;
#endif  // tskKERNEL_VERSION_MAJOR

__RETAINED_RW uint8_t FreeRTOSDebugConfig[] =
{
        FREERTOS_DEBUG_CONFIG_MAJOR_VERSION,
        FREERTOS_DEBUG_CONFIG_MINOR_VERSION,
        tskKERNEL_VERSION_MAJOR,
        tskKERNEL_VERSION_MINOR,
        tskKERNEL_VERSION_BUILD,
        configFRTOS_MEMORY_SCHEME,
        offsetof(struct tskTaskControlBlock, pxTopOfStack),
        offsetof(struct tskTaskControlBlock, xStateListItem),
        offsetof(struct tskTaskControlBlock, xEventListItem),
        offsetof(struct tskTaskControlBlock, pxStack),
        offsetof(struct tskTaskControlBlock, pcTaskName),
        offsetof(struct tskTaskControlBlock, uxTCBNumber),
        offsetof(struct tskTaskControlBlock, uxTaskNumber),
        configMAX_TASK_NAME_LEN,
        configMAX_PRIORITIES,
#if (tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 2)
        configENABLE_MPU,
        configENABLE_FPU,
        configENABLE_TRUSTZONE,
        configRUN_FREERTOS_SECURE_ONLY,
        0,            // 32-bit align
        0, 0, 0, 0    // padding
#else
        0             // pad to 32-bit boundary
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_TASKS_C_ADDITIONS_H */
