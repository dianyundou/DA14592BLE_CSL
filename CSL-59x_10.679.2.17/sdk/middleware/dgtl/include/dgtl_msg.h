/**
 * \addtogroup MID_INT_DGTL
 * \{
 * \addtogroup DGTL_MSG Messages
 *
 * \brief DGTL messages API
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dgtl_msg.h
 *
 * @brief Declarations for DGTL Messages API
 *
 * Copyright (C) 2016-2019 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef DGTL_MSG_H_
#define DGTL_MSG_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief DGTL message
 *
 */
typedef __PACKED_UNION {
        const uint8_t pkt_type;
        uint8_t data[0];
} dgtl_msg_t;

/**
 * \brief Allocate a DGTL message
 *
 * This function allocates an empty DGTL message.
 *
 * The allocated message contents has following structure:
 *
 *   ,-- packet type indicator
 *   |       ,-- packet header (length depends on packet type indicator)
 *   |       |            ,-- packet parameters
 *   |       |            |
 *   v       v            v
 * ,---.----------.---------------.
 * | 1 |  2 .. X  | X+1 .. length |
 * '---'----------'---------------'
 *  ^              ^
 *  |              |
 *  |              |
 *  |              `-- message parameters pointer
 *  `-- message pointer
 *
 * The message contents parameters pointer can be obtained using dgtl_msg_get_param_ptr() helper.
 *
 * The allocated message has to be freed using dgtl_msg_free() after use.
 *
 * \warning
 * The application shall not change packet type indicator for already allocated message, this may
 * result in an undefined behavior.
 *
 * \param [in] pkt_type  packet type indicator
 * \param [in] length    packet length
 *
 * \return DGTL message
 *
 */
dgtl_msg_t *dgtl_msg_alloc(uint8_t pkt_type, size_t length);

/**
 * \brief Free DGTL message
 *
 * This function should be used to free a DGTL message previously allocated using other DGTL APIs.
 *
 * \param [in] msg  DGTL message
 *
 */
void dgtl_msg_free(dgtl_msg_t *msg);

/**
 * \brief Get pointer to message contents parameters
 *
 * This function returns a pointer to message contents parameters. \p length parameter is optional
 * and, if supplied, will be filled with message contents parameters length.
 *
 * \warning
 * This function should only be called on an already initialized message which has proper packet
 * header set. Calling it on a message which is not properly initialized may result in undefined
 * behavior.
 *
 * \param [in]  msg     DGTL message
 * \param [out] length  message contents parameters length
 *
 * \return message contents pointer
 *
 */
void *dgtl_msg_get_param_ptr(dgtl_msg_t *msg, size_t *length);

/*
 * \brief Prepare DGTL message as HCI command packet
 *
 * This function can be used to fill HCI command packet header for a previously allocated DGTL
 * message. The message has to have packet type set to HCI command.
 *
 * When called with \p msg set to \p NULL, this function will allocated new message of proper size.
 *
 * \param [in] msg        existing DGTL message
 * \param [in] opcode     HCI command opcode
 * \param [in] param_len  length of parameters buffer
 * \param [in] param      command parameters
 *
 * \return DGTL message
 *
 */
dgtl_msg_t *dgtl_msg_prepare_hci_cmd(dgtl_msg_t *msg, uint16_t opcode, uint8_t param_len, void *param);

/*
 * \brief Prepare DGTL message as HCI event packet
 *
 * This function can be used to fill HCI event packet header for a previously allocated DGTL
 * message. The message has to have packet type set to HCI event.
 *
 * When called with \p msg set to \p NULL, this function will allocated new message of proper size.
 *
 * \param [in] msg        existing DGTL message
 * \param [in] code       HCI event code
 * \param [in] param_len  length of parameters buffer
 * \param [in] param      command parameters
 *
 * \return DGTL message
 *
 */
dgtl_msg_t *dgtl_msg_prepare_hci_evt(dgtl_msg_t *msg, uint8_t code, uint8_t param_len, void *param);

/**
 * \brief Get pointer for raw buffer of DGTL message
 *
 * \warning
 * This function is intended for internal usage only and shall not be used by regular applications.
 *
 * \param [in] msg  DGTL message
 *
 * \return buffer pointer
 *
 */
void *dgtl_msg_to_raw_ptr(dgtl_msg_t *msg);

/**
 * \brief Get DGTL message pointer from raw buffer
 *
 * \warning
 * This function is intended for internal usage only and shall not be used by regular applications.
 *
 * \param [in] ptr       buffer pointer
 * \param [in] pkt_type  packet type, as stored in buffer
 *
 * \return DGTL message
 *
 */
dgtl_msg_t *dgtl_msg_from_raw_ptr(void *ptr, uint8_t pkt_type);

#ifdef __cplusplus
}
#endif

#endif /* DGTL_MSG_H_ */

/**
 * \}
 * \}
 */
