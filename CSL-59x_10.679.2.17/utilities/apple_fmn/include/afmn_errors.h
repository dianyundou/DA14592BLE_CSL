/**
 ****************************************************************************************
 *
 * @file afmn_errors.h
 *
 * @brief Apple FMN errors header file
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AFMN_ERRORS_H_
#define AFMN_ERRORS_H_

/**
 * \brief Apple FMN framework error levels (criticality)
 */
typedef enum {
        AFMN_ERROR_NORMAL,                 /**< Normal error */
        AFMN_ERROR_CRITICAL,               /**< Critical error */
        AFMN_ERROR_INITIALIZATION,         /**< Error during module initialization */
} AFMN_ERROR_LEVEL;

/**
 * \brief Apple FMN framework error category that combines error location and type
 */
typedef enum {
        AFMN_ERROR_OS_TIMER_CREATE = 1,    /**< Timer created failed */
        AFMN_ERROR_OS_TIMER_START,         /**< Timer start failed */
        AFMN_ERROR_OS_TIMER_STOP,          /**< Timer stop failed */
        AFMN_ERROR_OS_QUEUE_CREATE,        /**< Queue create failed */
        AFMN_ERROR_OS_QUEUE,               /**< Queue operation failed */
        AFMN_ERROR_OS_MALLOC,              /**< Malloc failed */
        AFMN_ERROR_CRYPTO,                 /**< General crypto library failure */
        AFMN_ERROR_CRYPTO_WRONG_LENGTH,    /**< Incorrect length of crypto data */
        AFMN_ERROR_CRYPTO_PAIRING,         /**< Crypto operation failed during pairing */
        AFMN_ERROR_CRYPTO_KEY_ROTATION,    /**< Crypto operation failed during key rotation */
        AFMN_ERROR_BLE_DISCONNECT,         /**< BLE disconnect failed */
        AFMN_ERROR_BLE_CONNECTION_SETUP,   /**< BLE connection setup issue */
        AFMN_ERROR_BLE_INDICATION,         /**< BLE indication failed */
        AFMN_ERROR_BLE_INVALID_CONN_HANDLE,/**< BLE invalid connection handle */
        AFMN_ERROR_BLE_SET_ADV_PARAMS,     /**< BLE setting advertise parameters failed */
        AFMN_ERROR_BLE_SET_ADV_STRUCT,     /**< BLE setting advertise struct failed */
        AFMN_ERROR_BLE_ADV_START,          /**< BLE advertising failed */
        AFMN_ERROR_BLE_GATTS_CFM,          /**< BLE GATTS response error */
        AFMN_ERROR_UNSUPPORTED_OPCODE,     /**< Unsupported opcode received */
        AFMN_ERROR_SM_INVALID_STATE,       /**< Invalid state of AFMN state machine */
        AFMN_ERROR_ALREADY_PAIRED,         /**< Accessory is already paired */
        AFMN_ERROR_BUFFER_OVERFLOW,        /**< Receive buffer overflow */
        AFMN_ERROR_REQUEST_CHECK,          /**< Received request check failed */
        AFMN_ERROR_INVALID_SERIAL_QUERY,   /**< Unsupported serial number query */
        AFMN_ERROR_CALLBACK_NULL,          /**< AFMN callback is null */
        AFMN_ERROR_UNINITIALIZED,          /**< AFMN framework is not initialized */
        AFMN_ERROR_CANNOT_INIT             /**< AFMN framework cannot be initialized, reboot the device */
} AFMN_ERROR_CATEGORY;

#endif /* AFMN_ERRORS_H_ */
