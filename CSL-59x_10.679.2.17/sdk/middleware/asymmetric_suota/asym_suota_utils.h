/**
 ****************************************************************************************
 *
 * @file asym_suota_utils.h
 *
 * @brief Asymmetric SUOTA utilities functions header
 *
 * Copyright (C) 2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef ASYM_SUOTA_UTILS_H_
#define ASYM_SUOTA_UTILS_H_

#include <stdbool.h>

/*
 * MACRO DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Controls if image CRC is verified
 */
#ifndef ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN
#define ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN       ( 1 )
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */
/**
 * \brief Asymmetric utilities operation status
 */
typedef enum {
        ASUOTA_UTILS_ERR_NO_ERROR,           /**< Operation completed successfully */

        ASUOTA_UTILS_ERR_ALLOC,              /**< Required buffer(s) allocation failed */

        ASUOTA_UTILS_ERR_PART_OPEN,          /**< Opening required partition(s) failed */

        ASUOTA_UTILS_ERR_PROD_HDR_READ,      /**< Product header read failed */
        ASUOTA_UTILS_ERR_PROD_HDR_ERASE,     /**< Product header erase failed */
        ASUOTA_UTILS_ERR_PROD_HDR_WRITE_PRIM,/**< Primary product header write failed */
        ASUOTA_UTILS_ERR_PROD_HDR_WRITE_SEC, /**< Secondary product header write failed */
        ASUOTA_UTILS_ERR_PROD_HDR_SIGNATURE, /**< Product header identifier not found */
        ASUOTA_UTILS_ERR_PROD_HDR_CRC,       /**< Product header CRC mismatch */

        ASUOTA_UTILS_ERR_IMG_HDR_READ,       /**< Image header read failed */
        ASUOTA_UTILS_ERR_IMG_HDR_ERASE,      /**< Image header erase failed */
        ASUOTA_UTILS_ERR_IMG_HDR_WRITE,      /**< Image header write failed */
        ASUOTA_UTILS_ERR_IMG_HDR_SIGNATURE,  /**< Image header identifier not found */
        ASUOTA_UTILS_ERR_IMG_HDR_VERIFY,     /**< Image header verification failed */

        ASUOTA_UTILS_ERR_IMG_READ,           /**< Image contents read failed */
        ASUOTA_UTILS_ERR_IMG_WRITE,          /**< Image contents write failed */
        ASUOTA_UTILS_ERR_IMG_VERIFY,         /**< Image contents verification failed */
        ASUOTA_UTILS_ERR_IMG_CRC,            /**< Image contents CRC mismatch */

        ASUOTA_UTILS_ERR_FW_IMG_MISSING,     /**< FW image is missing / overwritten */
} ASUOTA_UTILS_ERR;

/*
 * API FUNCTION DECLARATIONS
 *****************************************************************************************
 */
/**
 * \brief Handles boot of application from FW location
 *
 * Detects if booted from application partition (FW_EXEC_PART or FW_UPDATE_PART). In such case it
 * copies the application to the FW partition (FIRMWARE_PART) and performs a system reboot.
 *
 * \return 0 if booted from FW partition, error type if booted from application partition and error occurred
 */
ASUOTA_UTILS_ERR asym_suota_utils_handle_boot_from_fw_location(void);

/**
 * \brief Checks if FW image is valid
 *
 * Checks if image identifier is valid and if \ref ASYM_SUOTA_UTILS_IMG_CRC_CHECK_EN is set to 1,
 * also checks if image CRC is correct
 *
 * \return Status of operation
 */
ASUOTA_UTILS_ERR asym_suota_utils_is_fw_valid(void);

/**
 * \brief Set device in application FW mode
 *
 * Changes the update firmware image address in product header to point to the application partition
 * (FW_EXEC_PART )and performs a system reboot.
 *
 * \param [in] reboot           Performs a system reboot when set to true
 *
 * \return Status of operation
 */
ASUOTA_UTILS_ERR asym_suota_utils_boot_fw_mode(bool reboot);

/**
 * \brief Set device in asymmetric SUOTA mode
 *
 * Changes the update firmware image address in product header to point to the FW partition
 * (NVMS_FIRMWARE_PART) and performs a system reboot.
 *
 * \param [in] reboot           Performs a system reboot when set to true
 *
 * \return Status of operation
 */
ASUOTA_UTILS_ERR asym_suota_utils_boot_suota_mode(bool reboot);

/**
 * \brief Gets the current active image's FW version string as stored in image header
 *
 * \return FW version string
 */
const char *asym_suota_utils_get_fw_ver_str(void);

/**
 * \brief Gets the current active image's FW address string
 *
 * \return Address string of current active image
 */
const char *asym_suota_utils_get_fw_addr_str(void);

#endif /* ASYM_SUOTA_UTILS_H_ */
