/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_BSR Busy Status Register Driver Service
 * \{
 * \brief Busy Status Register (BSR) driver
 */

/**
 ****************************************************************************************
 *
 * @file sys_bsr.h
 *
 * @brief Busy Status Register (BSR) driver file.
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
 ****************************************************************************************
 */

#ifndef SYS_BSR_H_
#define SYS_BSR_H_


#include "hw_sys.h"

/**
 * \brief Initialize the BSR module.
 */
void sys_sw_bsr_init(void);

/**
 * \brief Acquire exclusive access to a specific peripheral when
 *        it could also be used by another master. This function
 *        will block until access is granted.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be granted.
 *                       Valid range is (0 - 15). Check HW_SYS_BSR_PERIPH_ID.
 */
void sys_sw_bsr_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Checks if exclusive access to a specific peripheral has been acquired
 *        from a given master.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access will be checked.
 *                       Valid range is (0 - BSR_PERIPH_ID_MAX). Check HW_SYS_BSR_PERIPH_ID.
 * \return true if peripheral exclusive access has been acquired from the specific master, else false.
 */
bool sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Releases the exclusive access from a specific peripheral so it
 *        can be used by another master.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be released.
 *                       Valid range is (0 - 15). Check HW_SYS_BSR_PERIPH_ID.
 */
void sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);


#endif /* SYS_BSR_H_ */

/**
 \}
 \}
 */
