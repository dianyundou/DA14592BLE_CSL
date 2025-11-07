/**
 ****************************************************************************************
 *
 * @file flash_partitions.h
 *
 * @brief Default partition table
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef FLASH_PARTITIONS_H_
#define FLASH_PARTITIONS_H_

#ifndef PARTITION_TABLE_BEGIN
#define PARTITION_TABLE_BEGIN
#endif

#ifndef PARTITION_TABLE_ENTRY
/*
 * This to create partition entry in partition table.
 */
#define PARTITION_TABLE_ENTRY(start, size, id)
#endif

#ifndef PARTITION
/*
 * This macro can be used to define partition without specifying size. Size of partition created
 * with this macro will be computed using next partition starting address.
 * When this macro is used partition entries must be put in partition table in ascending starting
 * offset order.
 * Last entry in partition table can't be created with this macro.
 */
#define PARTITION(start, id, flags)
#endif

#ifndef PARTITION2
/*
 * This macro creates partition entry in similar way as PARTITION_TABLE_ENTRY but adds flags field.
 * In this case id_START and id_SIZE macros must be defined.
 */
#define PARTITION2(id, flags)
#endif

#ifndef PARTITION_TABLE_END
#define PARTITION_TABLE_END
#endif

#define PARTITION_TABLE_ADDR NVMS_PARTITION_TABLE_START

PARTITION_TABLE_BEGIN
#include <partition_table.h>
PARTITION_TABLE_END

#endif /* FLASH_PARTITIONS_H_ */

