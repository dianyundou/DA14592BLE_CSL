/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup BSP_INTERRUPTS Interrupts
 *
 * \brief Interrupt priority configuration
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file interrupts.h
 *
 * @brief Interrupt priority configuration
 *
 * Copyright (C) 2015-2020 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Setup interrupt priorities
 *
 * When CPU is reset all interrupts have some priority setup.
 * Reset     -3
 * NMI       -2
 * HardFault -1
 * All other interrupts have configurable priority that is set to 0.
 * If some interrupts should have priority other then default, this function should be called.
 * Argument \p prios can specify only those interrupts that need to have value other than default.
 * For memory efficiency table with priorities for each interrupt consist of interrupt priority
 * tag PRIORITY_x followed by interrupts that should have this priority, interrupts names are from
 * enum IRQn_Type.
 *
 * \note If interrupt priorities do not need to be changed dynamically at runtime, best way to
 * specify static configuration is to create table named __dialog_interrupt_priorities that will
 * be used automatically at startup.
 *
 * Most convenient way to prepare such table is to use macros like in example below:
 *
 * \code{.c}
 * INTERRUPT_PRIORITY_CONFIG_START(__dialog_interrupt_priorities)
 *      PRIORITY_0, // Start interrupts with priority 0 (highest)
 *               SVCall_IRQn,
 *               PendSV_IRQn,
 *               SysTick_IRQn,
 *      PRIORITY_1, // Start interrupts with priority 1
 *               BLE_WAKEUP_LP_IRQn,
 *               BLE_GEN_IRQn,
 *               FTDF_WAKEUP_IRQn,
 *               FTDF_GEN_IRQn,
 *      PRIORITY_2,
 *               SRC_IN_IRQn,
 *               SRC_OUT_IRQn,
 *      PRIORITY_3,
 *               UART_IRQn,
 *               UART2_IRQn,
 * INTERRUPT_PRIORITY_CONFIG_END
 * \endcode
 *
 * Table __dialog_interrupt_priorities can now be used to call this function.
 * Table can specify all interrupts or only those that need to be changed.
 *
 * \param [in] prios table with interrupts and priorities to setup
 *
 */
void set_interrupt_priorities(const int8_t prios[]);

/**
 * \brief Check whether running in interrupt context.
 *
 * \return true if the CPU is serving an interrupt.
 */
__STATIC_FORCEINLINE bool in_interrupt(void)
{
        return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}


/**
 * \brief Default interrupt priorities table.
 *
 */
extern const int8_t __dialog_interrupt_priorities[];

# define LAST_IRQn GPIO_P1_IRQn

/*
 * Following macros allow easy way to build table with interrupt priorities.
 * See example in set_interrupt_priorities function description.
 */
#define INTERRUPT_PRIORITY_CONFIG_START(name) const int8_t name[] = {
#define PRIORITY_0      (LAST_IRQn + 1)
#define PRIORITY_1      (LAST_IRQn + 2)
#define PRIORITY_2      (LAST_IRQn + 3)
#define PRIORITY_3      (LAST_IRQn + 4)
#define PRIORITY_4      (LAST_IRQn + 5)
#define PRIORITY_5      (LAST_IRQn + 6)
#define PRIORITY_6      (LAST_IRQn + 7)
#define PRIORITY_7      (LAST_IRQn + 8)
#define PRIORITY_8      (LAST_IRQn + 9)
#define PRIORITY_9      (LAST_IRQn + 10)
#define PRIORITY_10     (LAST_IRQn + 11)
#define PRIORITY_11     (LAST_IRQn + 12)
#define PRIORITY_12     (LAST_IRQn + 13)
#define PRIORITY_13     (LAST_IRQn + 14)
#define PRIORITY_14     (LAST_IRQn + 15)
#define PRIORITY_15     (LAST_IRQn + 16)
#define PRIORITY_TABLE_END (LAST_IRQn + 17)
#define INTERRUPT_PRIORITY_CONFIG_END PRIORITY_TABLE_END };

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPTS_H_ */

/**
 * \}
 * \}
 */
