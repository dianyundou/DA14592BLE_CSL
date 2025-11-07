/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup BSP_SUOTA
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file suota.h
 *
 * @brief SUOTA structure definitions
 *
 * Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SUOTA_H_
#define SUOTA_H_

#include <stdint.h>

#define SUOTA_VERSION_1_1       11
#define SUOTA_VERSION_1_2       12
#define SUOTA_VERSION_1_3       13
#define SUOTA_VERSION_1_4       14      // Security extension

#ifndef SUOTA_VERSION
#define SUOTA_VERSION           SUOTA_VERSION_1_3
#endif

#if SUOTA_PSM && (SUOTA_VERSION < SUOTA_VERSION_1_2)
#       error "SUOTA_PSM is only applicable to SUOTA_VERSION >= 1.2"
#endif

/**
 * \struct suota_1_1_product_header_t;
 *
 * \brief SUOTA 1.1 product header as defined by Dialog SUOTA specification.
 *
 * \note the same header is used for any SUOTA version newer than 1.1
 *
 */
typedef struct {
        uint8_t signature[2];
        uint16_t flags;
        uint32_t current_image_location;
        uint32_t update_image_location;
        uint8_t reserved[8];
} __attribute__((packed)) suota_1_1_product_header_t;

#define SUOTA_1_1_PRODUCT_HEADER_SIGNATURE_B1   0x70
#define SUOTA_1_1_PRODUCT_HEADER_SIGNATURE_B2   0x62

/**
 * \struct suota_1_1_image_header_t
 *
 * \brief SUOTA 1.1 image header as defined by Dialog SUOTA specification.
 *
 * \note the same header is used for any SUOTA version newer than 1.1
 *
 */
typedef struct {
        uint8_t signature[2];
        uint16_t flags;
        uint32_t code_size;
        uint32_t crc;
        uint8_t version[16];
        uint32_t timestamp;
        uint32_t exec_location;
} __attribute__((packed)) suota_1_1_image_header_t;

#define SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B1     0x70
#define SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B2     0x61

#define SUOTA_1_1_IMAGE_FLAG_FORCE_CRC          0x01
#define SUOTA_1_1_IMAGE_FLAG_VALID              0x02
#define SUOTA_1_1_IMAGE_FLAG_RETRY1             0x04
#define SUOTA_1_1_IMAGE_FLAG_RETRY2             0x08

typedef suota_1_1_image_header_t suota_image_header_t;

/**
 * \struct suota_1_1_image_header_da1469x_t
 *
 * \brief SUOTA 1.1 image header for DA1469x devices.
 *
 */
typedef struct {
        /** Image identifier */
        uint8_t image_identifier[2];
        /** Code size */
        uint32_t size;
        /** Code CRC */
        uint32_t crc;
        /** Version string (ends with '\0') */
        uint8_t version_string[16];
        /** Timestamp - seconds elapsed since epoch 1/1/1970 */
        uint32_t timestamp;
        /** Address of interrupt vector table */
        uint32_t pointer_to_ivt;
} __attribute__((packed)) suota_1_1_image_header_da1469x_t;

#define SUOTA_1_1_IMAGE_DA1469x_HEADER_SIGNATURE_B1       0x51
#define SUOTA_1_1_IMAGE_DA1469x_HEADER_SIGNATURE_B2       0x71

#define SUOTA_1_1_PRODUCT_DA1469x_HEADER_SIGNATURE_B1     0x50
#define SUOTA_1_1_PRODUCT_DA1469x_HEADER_SIGNATURE_B2     0x70

#endif /* SUOTA_H_ */

/**
 * \}
 * \}
 */
