/**
 ****************************************************************************************
 *
 * @file assertions.c
 *
 * @brief Assertion functions implementation.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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
#include "hw_watchdog.h"
#ifdef OS_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif
#include "hw_sys.h"

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)

#define NUM_OF_REGS_TO_SAVE                                     (4)

/* Store r0-r3 registers during assert function calls to improve debugging.
 * __saved_scratch_regs_array[0] = r0
 * __saved_scratch_regs_array[1] = r1
 * __saved_scratch_regs_array[2] = r2
 * __saved_scratch_regs_array[3] = r3
*/
/* Array where the R0-R3 scratch registers are saved when triggered an ASSERTION */
__RETAINED uint32_t __saved_scratch_regs_array[NUM_OF_REGS_TO_SAVE] __USED;

/**
 * When MTB is enabled, stop the tracing to prevent polluting the MTB
 * buffer with the while(1) in the assert_warning_uninit(), config_assert()
 * and assert_warning()
 */
__STATIC_FORCEINLINE void disable_tracing(void)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif /* dg_configENABLE_MTB */
}

__STATIC_FORCEINLINE void __SAVE_SCRATCH_REGS(void)
{
        __asm volatile (
                        "  .syntax unified  \n"                            /* force unified syntax */
                        "stmia  %[arr] , {r0-r3} \n"                       /* store the scratch registers */
                        :                                                  /* output */
                        : [arr] "r" ((uint32_t)__saved_scratch_regs_array) /* input */
                        : "r0","r1","r2","r3","memory"                     /* clobbers */
                       );
}

/**
 * The assert_warning() is used from anywhere in the code and is placed in
 * retention RAM to be safely called in all cases
 */
__NO_RETURN __ALWAYS_RETAINED_CODE static void assert_warning(void)
{
        __disable_irq();
        __SAVE_SCRATCH_REGS();
        disable_tracing();
        hw_watchdog_freeze();
        if (EXCEPTION_DEBUG == 1) {
                hw_sys_assert_trigger_gpio();
        }
        do {} while (1);
}

/**
 * The assert_warning_uninit() is used only during boot in SystemInitPre()
 * while the RAM is not initialized yet, thus is selected to run from FLASH.
 */
__NO_RETURN static void assert_warning_uninit(void)
{
        __disable_irq();
        __SAVE_SCRATCH_REGS();
        disable_tracing();
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SYS_WDOG_Msk;
        do {} while (1);
}

#ifdef OS_FREERTOS
__NO_RETURN __ALWAYS_RETAINED_CODE void config_assert(void)
{
        __disable_irq();
        __SAVE_SCRATCH_REGS();
        disable_tracing();
        hw_watchdog_freeze();
        do {} while (1);
}
#endif /* OS_FREERTOS */

#endif /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */


/* Pointers to assertion functions to use
 * In the SystemInitPre these will be initialized to assert_warning_uninit()
 * Then the RAM will be initialized and they will take the normal assignment
 * below pointing to assert_warning()
 */
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
typedef __NO_RETURN void (*assertion_func_t)(void);
__RETAINED_RW assertion_func_t assert_warning_func = assert_warning;


void assertion_functions_set_to_init(void)
{
        assert_warning_func = assert_warning;
}

void assertion_functions_set_to_uninit(void)
{
        assert_warning_func = assert_warning_uninit;
}
#endif /* (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) */
