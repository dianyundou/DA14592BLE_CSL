/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief Apple Find My Network (FMN) accessory
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#include <string.h>
#include <stdbool.h>
#include "osal.h"
#include "resmgmt.h"
#include "ad_ble.h"
#include "ble_mgr.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_watchdog.h"
#include "platform_devices.h"

/*
 * Perform any application specific hardware configuration. The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware(void);

/*
 * Task functions
 */
extern OS_TASK_FUNCTION(accessory_task, params);

/*
 * System initialization task
 */
static OS_TASK_FUNCTION(system_init, params)
{
        OS_TASK handle;
        OS_BASE_TYPE status __UNUSED;

#ifdef CONFIG_RETARGET
        extern void retarget_init(void);
#endif

        /* Use XTAL32M as system clock, initialize dividers and lp clock */
        cm_sys_clk_init(sysclk_XTAL32M);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /* Prepare the hardware to run this application */
        prvSetupHardware();

        /* Set the desired sleep mode */
        pm_set_wakeup_mode(true);
        pm_sleep_mode_set(pm_mode_extended_sleep);

#ifdef CONFIG_RETARGET
        retarget_init();
#endif

        /* Initialize BLE Manager */
        ble_mgr_init();

        /* Start the Apple FMN accessory application OS task */
        status = OS_TASK_CREATE("accessory",            /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                       accessory_task,                  /* The function that implements the task. */
                       NULL,                            /* The parameter passed to the task. */
                       6 * 1024,                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                       OS_TASK_PRIORITY_NORMAL,         /* The priority assigned to the task. */
                       handle);                         /* The task handle. */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* The work of the SysInit task is done */
        OS_TASK_DELETE(OS_GET_CURRENT_TASK());
}

/*
 * Main function creates a SysInit task which initializes the system and application system tasks
 */
int main(void)
{
        OS_TASK handle;
        OS_BASE_TYPE status __UNUSED;

        /* Start SysInit task. */
        status = OS_TASK_CREATE("SysInit",                /* The text name assigned to the task, for
                                                             debug only; not used by the kernel. */
                                system_init,              /* The System Initialization task. */
                                (void *) 0,               /* The parameter passed to the task. */
                                1200,                     /* The number of bytes to allocate to the
                                                             stack of the task. */
                                OS_TASK_PRIORITY_HIGHEST, /* The priority assigned to the task. */
                                handle);                  /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        OS_TASK_SCHEDULER_RUN();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached. */
        for ( ;; );
}

/*
 * Initialize the peripherals domain after power-up
 */
static void periph_init(void)
{
}

/*
 * Hardware Initialization
 */
static void prvSetupHardware(void)
{
        /* Initialize hardware */
        pm_system_init(periph_init);
}

/*
 * Malloc fail hook
 */
OS_APP_MALLOC_FAILED(void)
{
        /* This function will be called only if a call to OS_MALLOC() fails */
        ASSERT_ERROR(0);
}

#if (dg_configTRACK_OS_HEAP == 1)
/*
 * Constants/Variables used for Tasks Stack and OS Heap tracking
 * Declared global to avoid IDLE stack Overflows
 */
#define mainMAX_NB_OF_TASKS             10
#define mainMIN_STACK_GUARD_SIZE        8 /* words */
#define mainTOTAL_HEAP_SIZE_GUARD       64 /* bytes */

OS_TASK_STATUS pxTaskStatusArray[mainMAX_NB_OF_TASKS];
#endif /* dg_configTRACK_OS_HEAP */

/*
 * Application idle task hook
 */
OS_APP_IDLE(void)
{
        /* This function will be called on each iteration of the idle task */

#if (dg_configTRACK_OS_HEAP == 1)
        OS_BASE_TYPE i = 0;
        OS_BASE_TYPE uxMinimumEverFreeHeapSize;

        /* Generate raw status information about each task */
        OS_UBASE_TYPE uxNbOfTaskEntries = OS_GET_TASKS_STATUS(pxTaskStatusArray, mainMAX_NB_OF_TASKS);

        for (i = 0; i < uxNbOfTaskEntries; i++) {
                /* Check free stack */
                OS_BASE_TYPE uxStackHighWaterMark;

                uxStackHighWaterMark = OS_GET_TASK_STACK_WATERMARK(pxTaskStatusArray[i].xHandle);
                OS_ASSERT(uxStackHighWaterMark >= mainMIN_STACK_GUARD_SIZE);
        }

        /* Check minimum ever free heap against defined guard */
        uxMinimumEverFreeHeapSize = OS_GET_HEAP_WATERMARK();
        OS_ASSERT(uxMinimumEverFreeHeapSize >= mainTOTAL_HEAP_SIZE_GUARD);
#endif /* (dg_configTRACK_OS_HEAP == 1) */

#if (dg_configUSE_WDOG == 1)
        sys_watchdog_idle_task_notify();
#endif
}

/*
 * Application stack overflow hook
 */
OS_APP_STACK_OVERFLOW(OS_TASK pxTask, char *pcTaskName)
{
        (void) pcTaskName;
        (void) pxTask;

        /* This function is called if a stack overflow is detected */
        ASSERT_ERROR(0);
}

/*
 * Application tick hook
 */
OS_APP_TICK(void)
{
}
