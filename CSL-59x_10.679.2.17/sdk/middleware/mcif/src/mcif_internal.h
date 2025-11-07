/**
 ****************************************************************************************
 *
 * @file mcif_internal.h
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

#ifndef MCIF_INTERNAL_H
#define MCIF_INTERNAL_H

#include "osal.h"

#include "mcif.h"

#define MCIF_ASCII_UNKNOWN_HEADER "\r\nERROR: Unknown command.\r\n  "
#define MCIF_ASCII_HELP "\r\nAvailable commands:\r\n\r\n  "
#define MCIF_ASCII_EINVAL "\r\nERROR: Invalid arguments. Usage:\r\n\r\n  "
#define MCIF_ASCII_DONE_MESSAGE "\r\nOK\r\n"
#define MCIF_ASCII_FLAGS_ARG1_MASK 0x3
#define MCIF_ASCII_FLAGS_ARG2_MASK 0xC

#define MCIF_HALF_DUPLEX_PROTO

struct mcif_client
{
        uint8_t msgid;
        OS_QUEUE txq;
        OS_QUEUE rxq;

};

int mcif_parse_frame(uint8_t rxbyte[], int len, struct mcif_message_s **rxmsg);

void mcif_framing_init(void);
#endif /* MCIF_INTERNAL_H */
