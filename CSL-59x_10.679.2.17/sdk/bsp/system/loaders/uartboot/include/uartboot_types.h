/**
 ****************************************************************************************
 *
 * @file uartboot_types.h
 *
 * @brief Common type definitions
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

#ifndef UARTBOOT_TYPES_H
#define UARTBOOT_TYPES_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>


/*                 Memory layout
 * +=====================+==========================+
 * +        len          +          table           +
 * +=====================+==========================+
 * +    start_address    +          entry 1         +
 * +    size             +                          +
 * +    sector_size      +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +                     +                          +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +           potential   padding                  +
 * +=====================+==========================+
 * +    start_address    +          entry N         +
 * +    size             +                          +
 * +    sector_size      +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +                                                +
 * +           potential   padding                  +
 * +                                                +
 * +=====================+==========================+
 */

 /**
  * \brief Partition name buffer structure
  *
  */
typedef struct {
        uint16_t len;                   /**< Name length in bytes (including '\0') */
        uint8_t str;                    /**< Partition name (characters array)  */
} cmd_partition_name_t;

/**
 * \brief Partition entry structure
 *
 */
typedef struct {
        uint32_t start_address;         /**< Start address */
        uint32_t size;                  /**< Size */
        uint16_t sector_size;           /**< Sector size - can be aligned with FLASH sector e.g. 4KB */
        uint8_t  type;                  /**< Partition ID \sa nvms_partition_id_t */
        cmd_partition_name_t name;      /**< Partition name buffer */
} cmd_partition_entry_t;

/**
 * \brief Partition table structure
 *
 */
typedef struct {
        uint16_t len;                   /**< Size of the whole structure in bytes */
        cmd_partition_entry_t entry;    /**< Flexible array with all partition entries */
} cmd_partition_table_t;

/**
 * \brief Product information structure
 *
 */
typedef struct {
        uint16_t len;   /**< Size of product information in bytes */
        char     str;   /**< Product information (character array) */
} cmd_product_info_t;

#ifdef __cplusplus
}
#endif

#endif /* UARTBOOT_TYPES_H */
