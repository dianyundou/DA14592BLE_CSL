/**
 * \addtogroup MID_INT_DGTL
 * \{
 * \addtogroup DGTL_PKT Packets
 *
 * \brief DGTL Packets API
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file dgtl_pkt.h
 *
 * @brief Declarations for DGTL Packets API
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

#ifndef DGTL_PKT_H_
#define DGTL_PKT_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief DGTL message packet type indicator
 */
typedef enum {
        DGTL_PKT_TYPE_HCI_CMD = 0x01,
        DGTL_PKT_TYPE_HCI_ACL = 0x02,
        DGTL_PKT_TYPE_HCI_SCO = 0x03,
        DGTL_PKT_TYPE_HCI_EVT = 0x04,
        DGTL_PKT_TYPE_GTL     = 0x05,
        DGTL_PKT_TYPE_APP_CMD = 0x06,
        DGTL_PKT_TYPE_APP_RSP = 0x07,
        DGTL_PKT_TYPE_LOG     = 0x08,
} dgtl_pkt_type_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint16_t opcode;
        uint8_t length;
        uint8_t parameters[0];
} dgtl_pkt_hci_cmd_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint16_t handle;
        uint16_t length;
        uint8_t parameters[0];
} dgtl_pkt_hci_acl_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint16_t handle;
        uint8_t length;
        uint8_t parameters[0];
} dgtl_pkt_hci_sco_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint8_t code;
        uint8_t length;
        uint8_t parameters[0];
} dgtl_pkt_hci_evt_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint16_t msg_id;
        uint16_t dst_task_id;
        uint16_t src_task_id;
        uint16_t length;
        uint8_t parameters[0];
} dgtl_pkt_gtl_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint16_t opcode;
        uint16_t length;
        uint8_t parameters[0];
} dgtl_pkt_app_cmd_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint8_t code;
        uint16_t length;
        uint8_t parameters[0];
} dgtl_pkt_app_rsp_t;

typedef __PACKED_STRUCT {
        uint8_t pkt_type;
        uint8_t length;
        uint8_t parameters[0];
} dgtl_pkt_log_t;

typedef __PACKED_UNION {
        uint8_t pkt_type;
        dgtl_pkt_hci_cmd_t hci_cmd;
        dgtl_pkt_hci_acl_t hci_acl;
        dgtl_pkt_hci_sco_t hci_sco;
        dgtl_pkt_hci_evt_t hci_evt;
        dgtl_pkt_gtl_t     gtl;
        dgtl_pkt_app_cmd_t app_cmd;
        dgtl_pkt_app_rsp_t app_rsp;
        dgtl_pkt_log_t     log;
} dgtl_pkt_t;

/**
 * \brief Get header length of packet
 *
 * This function returns header length (including packet type indicator) of given packet. The packet
 * has to have at least \p pkt_type field initialized.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return header length
 *
 */
__STATIC_INLINE size_t dgtl_pkt_get_header_length(const dgtl_pkt_t *pkt)
{
        switch (pkt->pkt_type) {
        case DGTL_PKT_TYPE_HCI_CMD:
                return sizeof(dgtl_pkt_hci_cmd_t);
        case DGTL_PKT_TYPE_HCI_ACL:
                return sizeof(dgtl_pkt_hci_acl_t);
        case DGTL_PKT_TYPE_HCI_SCO:
                return sizeof(dgtl_pkt_hci_sco_t);
        case DGTL_PKT_TYPE_HCI_EVT:
                return sizeof(dgtl_pkt_hci_evt_t);
        case DGTL_PKT_TYPE_GTL:
                return sizeof(dgtl_pkt_gtl_t);
        case DGTL_PKT_TYPE_APP_CMD:
                return sizeof(dgtl_pkt_app_cmd_t);
        case DGTL_PKT_TYPE_APP_RSP:
                return sizeof(dgtl_pkt_app_rsp_t);
        case DGTL_PKT_TYPE_LOG:
                return sizeof(dgtl_pkt_log_t);
        }

        /* Unknown packet type, header length is not known */
        return 0;
}

/**
 * \brief Get parameters length of packet
 *
 * This function returns parameters length of given packet. The packet has to have header initialized
 * properly as otherwise return value in undefined.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return parameters length
 *
 */
__STATIC_INLINE size_t dgtl_pkt_get_param_length(const dgtl_pkt_t *pkt)
{
        switch (pkt->pkt_type) {
        case DGTL_PKT_TYPE_HCI_CMD:
                return pkt->hci_cmd.length;
        case DGTL_PKT_TYPE_HCI_ACL:
                return pkt->hci_acl.length;
        case DGTL_PKT_TYPE_HCI_SCO:
                return pkt->hci_sco.length;
        case DGTL_PKT_TYPE_HCI_EVT:
                return pkt->hci_evt.length;
        case DGTL_PKT_TYPE_GTL:
                return pkt->gtl.length;
        case DGTL_PKT_TYPE_APP_CMD:
                return pkt->app_cmd.length;
        case DGTL_PKT_TYPE_APP_RSP:
                return pkt->app_rsp.length;
        case DGTL_PKT_TYPE_LOG:
                return pkt->log.length;
        }

        /* Unknown packet type, parameters length is not known */
        return 0;
}

/**
 * \brief Get length of packet
 *
 * This function returns length of given packet. The packet has to have header initialized properly
 * as otherwise return value in undefined.
 *
 * \param [in] pkt  pointer to a packet buffer
 *
 * \return packet length
 *
 */
__STATIC_INLINE size_t dgtl_pkt_get_length(const dgtl_pkt_t *pkt)
{
        return dgtl_pkt_get_header_length(pkt) + dgtl_pkt_get_param_length(pkt);
}

#ifdef __cplusplus
}
#endif

#endif /* DGTL_PKT_H_ */

/**
 * \}
 * \}
 */
