/**
 ****************************************************************************************
 *
 * @file partition_def.h
 *
 * @brief Partition table entry definition
 *
 * Copyright (C) 2016-2020 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef PARTITION_DEF_H_
#define PARTITION_DEF_H_

/**
 * \brief NVMS Partition IDs
 */
typedef enum {
        NVMS_FIRMWARE_PART              = 1,
        NVMS_PARAM_PART                 = 2,
        NVMS_BIN_PART                   = 3,
        NVMS_LOG_PART                   = 4,
        NVMS_GENERIC_PART               = 5,
        NVMS_PLATFORM_PARAMS_PART       = 15,
        NVMS_PARTITION_TABLE            = 16,
        NVMS_FW_EXEC_PART               = 17,
        NVMS_FW_UPDATE_PART             = 18,
        NVMS_PRODUCT_HEADER_PART        = 19,
        NVMS_IMAGE_HEADER_PART          = 20,
} nvms_partition_id_t;

/**
 * \brief Partition entry.
 */
typedef struct partition_entry_t {
        uint8_t magic;          /**< Partition magic number 0xEA */
        uint8_t type;           /**< Partition ID */
        uint8_t valid;          /**< Valid marker 0xFF */
        uint8_t flags;          /**< */
        uint32_t start_address; /**< Partition start address */
        uint32_t size;          /**< Size of partition */
        uint8_t reserved2[4];   /**< Reserved for future use */
} partition_entry_t;

#define PARTITION_FLAG_READ_ONLY        (1 << 0)
#define PARTITION_FLAG_VES              (1 << 1)

#endif /* PARTITION_DEF_H_ */
