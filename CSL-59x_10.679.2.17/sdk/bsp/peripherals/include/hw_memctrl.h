/**
 * \addtogroup PLA_DRI_MEMORY
 * \{
 * \addtogroup HW_MEMCTRL Memory Controller
 * \{
 * \brief Memory Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_memctrl.h
 *
 * @brief Definition of API for the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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
 *****************************************************************************************
 */
#ifndef HW_MEMCTRL_H_
#define HW_MEMCTRL_H_

#include <sdk_defs.h>

/**
 * \brief Priority values for SYSCPU, DMA and CMAC.
 */
typedef enum {
        MEMCTRL_PRIO_LOWEST = 0,
        MEMCTRL_PRIO_LOWER = 1,
        MEMCTRL_PRIO_LOW = 2,
        MEMCTRL_PRIO_HIGH = 3,
        MEMCTRL_PRIO_HIGHER = 4,
        MEMCTRL_PRIO_HIGHEST = 5
} MEMCTRL_PRIO;

#define MEMCTRL_PRIO_DEFAULT MEMCTRL_PRIO_LOWEST

/**
 * \brief Resets memory controller's configuration.
 *
 * Must be used only when CMAC master is disabled.
 */

void hw_memctrl_reset(void);

/**
 * \brief Configures CMAC code, data and shared regions.
 *
 * \param [in] code_base_addr   CMAC code address. CMAC 0x00000000 address is remaped to this value.
 *                              Must be multiple of 1024. The region [code_base_addr, shared_base_addr]
 *                              is not accessible by DMA.
 * \param [in] data_base_addr   CMAC data address. CMAC 0x20000000 address is remaped to this value.
 *                              Must be multiple of 4.
 * \param [in] shared_base_addr CMAC code address. Must be multiple of 1024.
 * \param [in] end_addr         The upper bound of RAM region that CMAC can access. Must end at 1024 byte boundary (10 last bits 0x3FF).
 *                              DMA can only access the RAM region between shared_base_addr and end_addr addresses.
 */
void hw_memctrl_config_cmac_region(uint32_t code_base_addr, uint32_t data_base_addr, uint32_t shared_base_addr, uint32_t end_addr);


/**
 * \brief Configures RAM access priority for SYSCPU and DMA.
 *
 * CMAC and MTB have always priority over SYSCPU and DMA and they cannot
 * operate on the same RAM cell (Since MTB is located at the last RAM cell
 * CMAC should not operate there).
 *
 * When SYSCPU or DMA request access on the same RAM cell (and CMAC or MTB do not),
 * the PRIO fields determine which master will gain access. For the masters that did not get priority
 * there is an internal counter (with initial value equal to the STALL cycles fields) that
 * decreases by one. When the counter reaches zero, the specific master will gain access
 * regardless of its PRIO for a single cycle and the internal counter will be reset again to the
 * initial STALL value. This is done to avoid starvation of low priority masters.
 *
 * A possible mapping of priorities to priority/stall cycle pair values could be the following:
 * - HIGHEST: PRIO 2, STALL 3
 * - HIGH: PRIO 2, STALL 6
 * - MEDIUM: PRIO 1, STALL 9
 * - LOW: PRIO 0, STALL 12
 * - LOWEST: PRIO 0, STALL 15
 *
 * Configuring two masters with the same stall cycle values should be avoided, since the field
 * was added to differentiate between masters.
 *
 * \param [in] syscpu_prio SYCPU priority
 * \param [in] syscpu_max_stall_cycles SYCPU max stall cycles (1 - 15)
 * \param [in] dma_prio DMA priority
 * \param [in] dma_max_stall_cycles DMA max stall cycles (1 - 15)
 */
void hw_memctrl_config_master_priorities(MEMCTRL_PRIO syscpu_prio, uint8_t syscpu_max_stall_cycles,
                                         MEMCTRL_PRIO dma_prio, uint8_t dma_max_stall_cycles);

#endif /* HW_MEMCTRL_H_ */

/**
\}
\}
*/
