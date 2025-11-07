/**
 * \addtogroup BSP_CONFIG_DEFINITIONS
 * \{
 * \addtogroup BSP_CFG_DEF_DEVICE_MAP Device information attributes definitions.
 *
 * \brief Device information attributes definitions for all supported devices.
 *
 *\{
 */
/**
 ****************************************************************************************
 *
 * @file bsp_device_definitions.h
 *
 * @brief Board Support Package. Device information attributes definitions.
 *
 * Copyright (C) 2019-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef BSP_DEVICE_DEFINITIONS_H_
#define BSP_DEVICE_DEFINITIONS_H_


#include "bsp_device_definitions_internal.h"

/*
 * Available public macros characterizing each product supported by SDK10.
 * The variable dg_configDEVICE *MUST* take one of the following values
 * i.e. dg_configDEVICE=DA14680_01
 * The variable *MUST* be visible to both the compiler and the assembler.
 */

#define DA14680_01                      (DA14680 | _DEVICE_MK_CHIP_ID(680)  | _DEVICE_MK_VER(A, DONT_CARE, E))
#define DA14681_01                      (DA14681 | _DEVICE_MK_CHIP_ID(680)  | _DEVICE_MK_VER(A, DONT_CARE, E))
#define DA14682_00                      (DA14682 | _DEVICE_MK_CHIP_ID(680)  | _DEVICE_MK_VER(B, DONT_CARE, B))
#define DA14683_00                      (DA14683 | _DEVICE_MK_CHIP_ID(680)  | _DEVICE_MK_VER(B, DONT_CARE, B))

#define DA14691_00                      DA14691_2522_00
#define DA14693_00                      DA14693_2522_00
#define DA14695_00                      DA14695_2522_00
#define DA14697_00                      DA14697_2522_00
#define DA14699_00                      DA14699_2522_00

#define DA14592_00                      DA14592_2634_00

#define DA14701_00                      DA14701_2798_00
#define DA14705_00                      DA14705_2798_00
#define DA14706_00                      DA14706_2798_00
#define DA14708_00                      DA14708_2798_00


#endif /* BSP_DEVICE_DEFINITIONS_H_ */
/**
\}
\}
*/
