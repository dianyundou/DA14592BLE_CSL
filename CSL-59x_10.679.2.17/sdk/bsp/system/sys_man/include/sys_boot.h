/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_BOOT system Boot Service
 *
 * \brief System Boot Service
 *
 * \{
 */

/**
****************************************************************************************
*
* @file sys_boot.h
*
* @brief System Boot Handler header file
*
* Copyright (C) 2022-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SYS_BOOT_H_
#define SYS_BOOT_H_

#if dg_configUSE_SYS_BOOT

/**
 * \brief  Check and repair the Primary and the Backup Product Headers
 *
 * Check whether the Primary Product Header is valid or not. If not, copy the Backup Product Header
 * on it and validate the CRC of the repaired Primary Product Header. If yes, check the Backup
 * Product Header. If corrupted, copy the Primary Product Header on it and check the CRC of the
 * repaired Backup Product Header. In case that both product headers are corrupted, the system will
 * never boot, hence it is meaningless to check both of them at every boot.
 *
 */
void sys_boot_restore_product_headers(void);

#endif /* dg_configSYS_BOOT */

#endif /* SYS_BOOT_H_ */
/**
 * \}
 * \}
 */
