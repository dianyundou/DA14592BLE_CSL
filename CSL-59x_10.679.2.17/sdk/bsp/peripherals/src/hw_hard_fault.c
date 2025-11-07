/**
 ****************************************************************************************
 *
 * @file hw_hard_fault.c
 *
 * @brief HardFault handler.
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

#include <stdint.h>
#include <stdio.h>
#include "sdk_defs.h"
#include "hw_hard_fault.h"
#include "hw_watchdog.h"
#include "hw_cpm.h"
#include "hw_sys.h"



/*
 * Global variables
 */
volatile uint32_t hardfault_event_data[9] __attribute__((section("hard_fault_info")));

/*
 * Local variables
 */


/*
 * This is the base address in Retention RAM where the stacked information will be copied.
 */
#define STATUS_BASE (0x20005600)

/*
 * Compilation switch to enable verbose output on HardFault
 */
#ifndef VERBOSE_HARDFAULT
#       define VERBOSE_HARDFAULT        0
#endif

/*
 * Function definitions
 */

/**
* \brief HardFault handler implementation. During development it will copy the system's status
*        to a predefined location in memory. In release mode, it will cause a system reset.
*
* \param [in] hardfault_args The system's status when the HardFault event occurred.
*
* \return void
*
*/
__ALWAYS_RETAINED_CODE void HardFault_HandlerC(unsigned long *hardfault_args)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif /* dg_configENABLE_MTB */

        // Stack frame contains:
        // r0, r1, r2, r3, r12, r14, the return address and xPSR
        // - Stacked R0 = hf_args[0]
        // - Stacked R1 = hf_args[1]
        // - Stacked R2 = hf_args[2]
        // - Stacked R3 = hf_args[3]
        // - Stacked R12 = hf_args[4]
        // - Stacked LR = hf_args[5]
        // - Stacked PC = hf_args[6]
        // - Stacked xPSR= hf_args[7]
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                hw_watchdog_freeze();                           // Stop WDOG

                ENABLE_DEBUGGER;

                *(volatile unsigned long *)(STATUS_BASE       ) = hardfault_args[0];    // R0
                *(volatile unsigned long *)(STATUS_BASE + 0x04) = hardfault_args[1];    // R1
                *(volatile unsigned long *)(STATUS_BASE + 0x08) = hardfault_args[2];    // R2
                *(volatile unsigned long *)(STATUS_BASE + 0x0C) = hardfault_args[3];    // R3
                *(volatile unsigned long *)(STATUS_BASE + 0x10) = hardfault_args[4];    // R12
                *(volatile unsigned long *)(STATUS_BASE + 0x14) = hardfault_args[5];    // LR
                *(volatile unsigned long *)(STATUS_BASE + 0x18) = hardfault_args[6];    // PC
                *(volatile unsigned long *)(STATUS_BASE + 0x1C) = hardfault_args[7];    // PSR
                *(volatile unsigned long *)(STATUS_BASE + 0x20) = (unsigned long)hardfault_args;    // Stack Pointer

                *(volatile unsigned long *)(STATUS_BASE + 0x24) = (*((volatile unsigned long *)(0xE000ED28)));    // CFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x28) = (*((volatile unsigned long *)(0xE000ED2C)));    // HFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x2C) = (*((volatile unsigned long *)(0xE000ED30)));    // DFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x30) = (*((volatile unsigned long *)(0xE000ED3C)));    // AFSR
                *(volatile unsigned long *)(STATUS_BASE + 0x34) = (*((volatile unsigned long *)(0xE000ED34)));    // MMAR
                *(volatile unsigned long *)(STATUS_BASE + 0x38) = (*((volatile unsigned long *)(0xE000ED38)));    // BFAR

                if (VERBOSE_HARDFAULT) {
                        printf("HardFault Handler:\r\n");
                        printf("- R0  = 0x%08lx\r\n", hardfault_args[0]);
                        printf("- R1  = 0x%08lx\r\n", hardfault_args[1]);
                        printf("- R2  = 0x%08lx\r\n", hardfault_args[2]);
                        printf("- R3  = 0x%08lx\r\n", hardfault_args[3]);
                        printf("- R12 = 0x%08lx\r\n", hardfault_args[4]);
                        printf("- LR  = 0x%08lx\r\n", hardfault_args[5]);
                        printf("- PC  = 0x%08lx\r\n", hardfault_args[6]);
                        printf("- xPSR= 0x%08lx\r\n", hardfault_args[7]);
                }

                if (EXCEPTION_DEBUG == 1) {
                        hw_sys_assert_trigger_gpio();
                }

                while (1);
        }
        else {
# ifdef PRODUCTION_DEBUG_OUTPUT
# if (USE_WDOG)
                WDOG->WATCHDOG_REG = 0xC8;                      // Reset WDOG! 200 * 10.24ms active time for UART to finish printing!
# endif

                dbg_prod_output(1, hardfault_args);
# endif // PRODUCTION_DEBUG_OUTPUT

                hardfault_event_data[0] = HARDFAULT_MAGIC_NUMBER;
                hardfault_event_data[1] = hardfault_args[0];    // R0
                hardfault_event_data[2] = hardfault_args[1];    // R1
                hardfault_event_data[3] = hardfault_args[2];    // R2
                hardfault_event_data[4] = hardfault_args[3];    // R3
                hardfault_event_data[5] = hardfault_args[4];    // R12
                hardfault_event_data[6] = hardfault_args[5];    // LR
                hardfault_event_data[7] = hardfault_args[6];    // PC
                hardfault_event_data[8] = hardfault_args[7];    // PSR

                hw_cpm_reboot_system();                         // Force reset

                while (1);
        }
}

__ALWAYS_RETAINED_CODE void MemManage_Handler(void)
{
        volatile uint8_t mem_fault_status_reg;
        volatile uint32_t mem_fault_addr __UNUSED;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif
        mem_fault_status_reg = (SCB->CFSR & SCB_CFSR_MEMFAULTSR_Msk) >> SCB_CFSR_MEMFAULTSR_Pos;
        if (mem_fault_status_reg & 0x80) {
                mem_fault_addr = SCB->MMFAR;
        }
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }
        while (1) {}
}

__RETAINED_CODE void BusFault_Handler(void)
{
        volatile uint8_t bus_fault_status_reg;
        volatile uint32_t bus_fault_addr __UNUSED;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif

        bus_fault_status_reg = (SCB->CFSR & SCB_CFSR_BUSFAULTSR_Msk) >> SCB_CFSR_BUSFAULTSR_Pos;
        if (bus_fault_status_reg & 0x80) {
                bus_fault_addr = SCB->BFAR;
        } else {
                bus_fault_addr = 0x0;
        }
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }
        while (1) {}
}

__ALWAYS_RETAINED_CODE void UsageFault_Handler(void)
{
        volatile uint16_t usage_fault_status_reg __UNUSED;

        usage_fault_status_reg = (SCB->CFSR & SCB_CFSR_USGFAULTSR_Msk) >> SCB_CFSR_USGFAULTSR_Pos;

#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif

        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }

        while (1) {}
}

__ALWAYS_RETAINED_CODE void DebugMon_Handler(void)
{
#if dg_configENABLE_MTB
        /* Disable MTB */
        *MTB_MASTER_REG = MTB_MASTER_REG_DISABLE_VAL;
#endif

        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                        hw_watchdog_freeze();                           // Stop WDOG
        }

        while (1) {}
}


