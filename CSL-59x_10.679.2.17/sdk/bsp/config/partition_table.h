/**
 ****************************************************************************************
 *
 * @file bsp/config/partition_table.h
 *
 * @brief Partition table selection. Image partition's size definition.
 *
 * Copyright (C) 2016-2024 Renesas Electronics Corporation and/or its affiliates.
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

/*
 * When partition_table is not overridden by adding a custom partition_table.h file to a project
 * then this file is used to select partition table by macro definition.
 *
 * To use layout other than SDK one, add include path into the project settings that will point
 * to a folder with the custom partition_table file.
 */


#  if defined(USE_PARTITION_TABLE_EFLASH_WITH_SUOTA)
#   include <EFLASH/suota/partition_table.h>
#  elif defined(USE_PARTITION_TABLE_1MB_WITH_SUOTA)
#   include <1M/suota/partition_table.h>
#  elif defined(USE_PARTITION_TABLE_4MB_WITH_SUOTA)
#   include <4M/suota/partition_table.h>
#  elif defined(USE_PARTITION_TABLE_1MB)
#   include <1M/partition_table.h>
#  else
#   if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || \
       (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#    include <EFLASH/partition_table.h>
#   elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#    include <4M/partition_table.h>
#   endif
#  endif


/*
 * Define a maximal size of the image which could be written to QSPI - based on the partition sizes.
 */
#if defined(NVMS_FW_EXEC_PART_SIZE) && defined(NVMS_FW_UPDATE_PART_SIZE)
#if NVMS_FW_EXEC_PART_SIZE < NVMS_FW_UPDATE_PART_SIZE
#define IMAGE_PARTITION_SIZE    NVMS_FW_EXEC_PART_SIZE
#else
#define IMAGE_PARTITION_SIZE    NVMS_FW_UPDATE_PART_SIZE
#endif /* NVMS_FW_EXEC_PART_SIZE < NVMS_FW_UPDATE_PART_SIZE */
#elif defined(NVMS_FW_EXEC_PART_SIZE)
#define IMAGE_PARTITION_SIZE    NVMS_FW_EXEC_PART_SIZE
#elif defined(NVMS_FW_UPDATE_PART_SIZE)
#define IMAGE_PARTITION_SIZE    NVMS_FW_UPDATE_PART_SIZE
#elif defined(NVMS_FIRMWARE_PART_SIZE)
#define IMAGE_PARTITION_SIZE    NVMS_FIRMWARE_PART_SIZE
#else
#error "At least one partition where application could be placed should be defined!"
#endif /* defined(NVMS_FW_EXEC_PART_SIZE) && defined(NVMS_FW_UPDATE_PART_SIZE) */
