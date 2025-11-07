/**
 ****************************************************************************************
 *
 * @file qspi_automode_internal_v2.h
 *
 * @brief Access QSPI device when running in auto mode
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

#ifndef QSPI_AUTOMODE_INTERNAL_V2_H_
#define QSPI_AUTOMODE_INTERNAL_V2_H_

#if dg_configQSPI_AUTOMODE_ENABLE

#include "qspi_common.h"

/**
 * \brief Switch the QSPIC to auto access mode.
 *
 * \param [in] id       QSPI controller ID
 */
__RETAINED_CODE void qspi_automode_int_enter_auto_access_mode(HW_QSPIC_ID id);

/**
 * \brief Write data to flash memory
 *
 * \param [in] id       QSPI controller ID
 * \param [in] addr     Zero based address of the flash memory, where the content of the buf is to be written.
 * \param [in] buf      Pointer to the source data.
 * \param [in] size     Number of bytes to write.
 *
 * \return Number of written bytes by the function call.
 *
 * \warning This function switches and leaves the QSPIC to manual access mode. Therefore, it must
 *          be called with disabled interrupts. It's up to the caller to switch the QSPIC back to
 *          auto access mode, in order to re-enable XiP.
 *
 * \warning The write operation will not exceed the page boundary. Thus, It's up to the caller to
 *          issue another call of the function, in order to write the remaining data to the next page.
 */
__RETAINED_CODE uint32_t qspi_automode_int_flash_write_page(HW_QSPIC_ID id, uint32_t addr, const uint8_t *buf, uint32_t size);

#endif /* dg_configQSPI_AUTOMODE_ENABLE */

#endif /* QSPI_AUTOMODE_INTERNAL_V2_H_ */
