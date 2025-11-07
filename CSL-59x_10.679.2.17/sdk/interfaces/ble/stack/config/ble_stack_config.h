/**
 * \addtogroup MID_INT_BLE
 * \{
 * \addtogroup BLE_STACK_CONFIG Configuration Options for BLE Stack
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_stack_config.h
 *
 * @brief BLE stack configuration options
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
#ifndef _BLE_STACK_CONFIG_H_
#define _BLE_STACK_CONFIG_H_


#define RAM_BUILD

/*
 * Stack definitions
 */
#include "da14590_config.h"


/* macro used to silence warnings about unused parameters/variables/function */
#define __UNUSED__      __attribute__((unused))

// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                        ASSERT_ERROR(cond)



/**
 * \brief Ble pti values
 */
#define BLE_PTI_CONNECT_REQ_RESPONSE      0
#define BLE_PTI_LLCP_PACKETS              1
#define BLE_PTI_DATA_CHANNEL_TX           2
#define BLE_PTI_INITIATING_SCAN           3
#define BLE_PTI_ACTIVE_SCAN_MODE          4
#define BLE_PTI_CONNECTABLE_ADV_MODE      5
#define BLE_PTI_NON_CONNECTABLE_ADV_MODE  6

#include "bsp_debug.h"

#endif /* _BLE_STACK_CONFIG_H_ */

/**
 * \}
 * \}
 */
