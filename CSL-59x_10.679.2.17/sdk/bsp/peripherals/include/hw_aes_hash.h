/**
 * \addtogroup PLA_DRI_CRYPTO
 * \{
 * \addtogroup AES_HASH AES / HASH
 * \{
 * \brief AES/Hash Engine
 */

/**
 ****************************************************************************************
 *
 * @file hw_aes_hash.h
 *
 * @brief Definition of API for the AES/HASH Engine Low Level Driver.
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

#ifndef HW_AES_HASH_H_
#define HW_AES_HASH_H_

#include <stdbool.h>
#include "sdk_defs.h"


#if dg_configUSE_HW_AES || dg_configUSE_HW_HASH

/**
 * \brief       AES/HASH engine status
 */
typedef enum {
        HW_AES_HASH_STATUS_UNLOCKED = 0,
        HW_AES_HASH_STATUS_LOCKED_BY_AES  = 1,
        HW_AES_HASH_STATUS_LOCKED_BY_HASH = 2,
} HW_AES_HASH_STATUS;

/**
 * \brief       Masks of AES/HASH Engine Interrupt sources
 *
 * Use these enumerator values to detect which interrupt source triggered the Crypto_Handler by
 * masking the status variable of the IRQ callback with them, as indicated by the next example:
 *
 * \code
 *
 * void aes_hash_callback(uint32_t status)
 * {
 *      bool active;
 *      bool waiting_for_input;
 *
 *      active = !(status & HW_AES_HASH_IRQ_MASK_INACTIVE);
 *      waiting_for_input = !(status & HW_AES_HASH_IRQ_MASK_WAITING_FOR_INPUT);
 *      ...
 * \endcode
 */

typedef enum {
        HW_AES_HASH_IRQ_MASK_INACTIVE = REG_MSK(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_INACTIVE),
        HW_AES_HASH_IRQ_MASK_WAITING_FOR_INPUT = REG_MSK(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_WAIT_FOR_IN),
} HW_AES_HASH_IRQ_MASK;

/**
 * \brief       Set AES/HASH engine input data mode
 *
 * \param [in]  wait_more_input  If true, the AES/HASH engine expects more input data to be received,
 *                               thus when the current input data has been processed, it waits for
 *                               incoming data by setting the corresponding flag (CRYPTO_WAIT_FOR_IN).
 *                               If false, the current input data is considered as the last one and
 *                               the output data is written to the memory.
 */
__STATIC_INLINE void hw_aes_hash_set_input_data_mode(bool wait_more_input)
{
        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN, wait_more_input);
}

/**
 * \brief       Get AES/HASH engine input data mode
 *
 * \return      true if the AES/HASH engine expects more input data to be received, otherwise false.
 */
__STATIC_INLINE bool hw_aes_hash_get_input_data_mode(void)
{
        return REG_GETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_MORE_IN);
}

/**
 * \brief       Set the input data length
 *
 * \param [in]  len Input data length
 */
__STATIC_INLINE void hw_aes_hash_set_input_data_len(uint32_t len)
{
        AES_HASH->CRYPTO_LEN_REG = len;
}

/**
 * \brief       Get the input data length
 *
 * \return      Input data length
 */
__STATIC_INLINE uint32_t hw_aes_hash_get_input_data_len(void)
{
        return AES_HASH->CRYPTO_LEN_REG;
}

/**
 * \brief Check whether the AES/Hash Engine is waiting for more input data or not.
 *
 * \return true if the AES/Hash engine is waiting more data, otherwise false.
 *
 */
__STATIC_INLINE bool hw_aes_hash_waiting_for_input_data(void)
{
        return REG_GETF(AES_HASH, CRYPTO_STATUS_REG, CRYPTO_WAIT_FOR_IN) == 0;
}

/**
 * \brief       Set the address of the Input Data
 *
 * \param [in]  inp_data_addr Address of the Input Data
 */
void hw_aes_hash_set_input_data_addr(uint32_t inp_data_addr);

/**
 * \brief       Set the address of the Output Data
 *
 * \param [in]  out_data_addr The output data address.
 */
void hw_aes_hash_set_output_data_addr(uint32_t out_data_addr);

/**
 * \brief       Get the status of the AES/HASH engine.
 *
 * \return      The status of the AES/HASH engine
 *
 * \sa          HW_AES_HASH_STATUS
 */
HW_AES_HASH_STATUS hw_aes_hash_get_status(void);

/**
 * \brief AES/Hash callback
 *
 * This function is called by the AES/Hash driver when the interrupt is fired.
 *
 */
typedef void (*hw_aes_hash_cb)(uint32_t status);

/**
 * \brief       Enable AES/HASH engine clock
 *
 */
__STATIC_INLINE void hw_aes_hash_enable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief       Disable AES/HASH engine clock
 *
 */
__STATIC_INLINE void hw_aes_hash_disable_clock(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief       Check whether the AES/HASH engine clock is enabled or not.
 *
 * \return      true if enabled, otherwise false.
 */
__STATIC_INLINE bool hw_aes_hash_clock_is_enabled(void)
{
        return REG_GETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE);
}

/**
 * \brief AES/Hash is active.
 *
 * Check whether the AES/Hash engine is active or not.
 *
 * \warning The AES crypto engine employs its own dedicated DMA (distinct from the system DMA) to
 *          fetch data from the memory, which utilizes the AHB-DMA bus to accomplish the data
 *          transaction. At the same time the Cortex-M33 processor utilizes the AHB-CPUC bus to
 *          fetch data in order to execute code in place, which has higher priority from the AHB-DMA.
 *
 *          Usually this function is called inside a loop in order to poll whether an AES operation
 *          has been completed or not. Consequently, if it resides in the XiP flash memory and the
 *          data to be processed by the AES crypto engine reside there as well, the continuous call
 *          of the function through the AHB-CPUC bus might block the engine from fetching the required
 *          data via AHB-DMA bus, actually if the instruction cache is disabled, leading the AES
 *          operation to get blocked forever. To prevent this situation, this function is always
 *          retained in SysRAM.
 *
 * \return true if the AES/Hash engine is active, otherwise false.
 *
 */
__ALWAYS_RETAINED_CODE bool hw_aes_hash_is_active(void);

/**
 * \brief Start AES/HASH engine operation
 *
 * Start an AES/HASH operation depending on the configuration of the AES/HASH Engine.
 *
 */
__STATIC_INLINE void hw_aes_hash_start(void)
{
        AES_HASH->CRYPTO_START_REG = 1;
}

/**
 * \brief De-initialize AES/HASH crypto engine
 *
 * This function disables the AES/HASH engine interrupt, clears any pending interrupt request and
 * disables the AES/HASH engine clock.
 */
void hw_aes_hash_deinit(void);

/**
 * \brief Enable interrupt for AES/HASH crypto engine.
 *
 * This function enables the crypto engine interrupt in the NVIC controller, enables the AES/HASH
 * engine interrupt source, and registers a callback function to serve the pending interrupt requests.
 *
 * \param [in] cb Callback to serve the crypto engine's interrupt requests.
 *
 */
void hw_aes_hash_interrupt_enable(hw_aes_hash_cb cb);

/**
 * \brief Disable interrupt for AES/HASH crypto engine.
 *
 * This function disables the crypto engine interrupt in the NVIC controller, disables the AES/HASH
 * engine interrupt source and clears any registered callback function.
 */
void hw_aes_hash_interrupt_disable(void);

#endif /* dg_configUSE_HW_AES || dg_configUSE_HW_HASH */


#endif /* HW_AES_HASH_H_ */
/**
 * \}
 * \}
 */
