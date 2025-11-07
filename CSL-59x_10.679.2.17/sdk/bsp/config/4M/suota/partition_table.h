/**
 ****************************************************************************************
 *
 * @file 4M/suota/partition_table.h
 *
 * Copyright (C) 2018-2023 Renesas Electronics Corporation and/or its affiliates.
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


#define NVMS_PRODUCT_HEADER_PART_START  (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00001000)
#define NVMS_PRODUCT_HEADER_PART_SIZE   (0x00002000)

/* The size of the firmware image should be contained within the memory range that can be accessed
 * by M33, which is determined by the default flash region size (512KB) in relation to the base
 * address. If the FW image exceeds this size, the M33 will be unable to access the extra code. */
#define NVMS_FW_EXEC_PART_START         (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00003000)
#define NVMS_FW_EXEC_PART_SIZE          (0x0007D000)

/* +----------------512KB---------------------+ */

#define NVMS_GENERIC_PART_START         (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00080000)
#define NVMS_GENERIC_PART_SIZE          (0x00080000)
#define NVMS_PLATFORM_PARAMS_PART_START (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00100000)
#define NVMS_PLATFORM_PARAMS_PART_SIZE  (0x000FF000)
/* Recommended location, 4KB before the end of the 1st flash section. */
#define NVMS_PARAM_PART_START           (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x001FF000)
#define NVMS_PARAM_PART_SIZE            (0x00001000)

/* +------------------2MB---------------------+ */
/* The size of the update firmware image should be contained within the memory range that can be
 * accessed by M33, which is determined by the default flash region size (512KB) in relation to the
 * base address. If the FW image exceeds this size, the M33 will be unable to access the extra code. */
#define NVMS_FW_UPDATE_PART_START       (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x00200000)
#define NVMS_FW_UPDATE_PART_SIZE        (0x0007D000)
#define NVMS_LOG_PART_START             (QSPI_MEM1_VIRTUAL_BASE_ADDR + 0x0027D000)
#define NVMS_LOG_PART_SIZE              (0x00183000)
/* To maintain consistency across various partition tables and facilitate reading without requiring
 * the rebuilding of flashing tools, it is strongly recommended to position the NVMS_PARTITION_TABLE
 * section at the end of the Embedded Flash. */
#define NVMS_PARTITION_TABLE_START      (EFLASH_MEM1_VIRTUAL_BASE_ADDR + 0x0003F800)
#define NVMS_PARTITION_TABLE_SIZE       (0x00000800)

PARTITION2( NVMS_PRODUCT_HEADER_PART  , 0 )
PARTITION2( NVMS_FW_EXEC_PART         , 0 )
PARTITION2( NVMS_GENERIC_PART         , PARTITION_FLAG_VES )
PARTITION2( NVMS_PLATFORM_PARAMS_PART , PARTITION_FLAG_READ_ONLY )
PARTITION2( NVMS_PARAM_PART           , 0 )
PARTITION2( NVMS_FW_UPDATE_PART       , 0 )
PARTITION2( NVMS_LOG_PART             , 0 )
PARTITION2( NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )



