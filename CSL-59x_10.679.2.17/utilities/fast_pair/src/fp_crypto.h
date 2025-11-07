/**
 ****************************************************************************************
 *
 * @file fp_crypto.h
 *
 * @brief Google Fast Pair crypto helper module header file
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

#ifndef FP_CRYPTO_H_
#define FP_CRYPTO_H_

#include <stdint.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"

/* The length of SHA256 in bytes */
#define FP_CRYPTO_SHA256_BYTES_LEN      32
/* A single AES block length */
#define FP_CRYPTO_AES_BLOCK_LENGTH      16

/**
 * \brief Crypto operation: encryption or decryption
 */
typedef enum {
        FP_CRYPTO_OP_ENCRYPT,
        FP_CRYPTO_OP_DECRYPT
} FP_CRYPTO_OP;

/**
 * \brief Calculate SHA-256 of the input buffer
 *
 * \param [out] output calculated hash (SHA256)
 * \param [in] input input buffer for hash calculation
 * \param [in] n the length of the input buffer
 */
void fp_crypto_sha256(uint8_t *output, const uint8_t *input, uint32_t n);

/**
 * \brief Get random 2-byte salt
 *
 * \return 2-byte salt
 */
uint16_t fp_crypto_get_salt(void);

/**
 * \brief Calculate HMAC-SHA-256 of the input buffer
 *
 * \param [in] input input buffer for the calculation
 * \param [in] length the length of the input buffer
 * \param [in] key key used for HMAC calculation
 * \param [in] key_length the length of the key
 * \param [out] output calculated hash (HMAC-SHA256)
 */
void fp_crypto_hmac_sha256(const uint8_t *input, uint8_t length, const uint8_t *key,
        uint8_t key_length, uint8_t *output);

/**
 * \brief Encrypt or decrypt the input buffer with AES in CTR mode
 *
 * \param [in] input input buffer for the calculation
 * \param [in] length the length of the input buffer
 * \param [in] nonce 16-byte nonce buffer
 * \param [in] key AES 16-byte key
 * \param [in] op crypto operation (FP_CRYPTO_OP)
 * \param [out] output AES encrypted input buffer
 */
void fp_crypto_aes_ctr(const uint8_t *input, uint8_t length, uint8_t *nonce, const uint8_t *key,
        FP_CRYPTO_OP op, uint8_t *output);

/**
 * \brief Encrypt or decrypt buffer with AES in ECB mode
 *
 * \param [in] key AES key
 * \param [in] key_size AES key size
 * \param [in] op crypto operation (FP_CRYPTO_OP)
 * \param [in] block_num number of AES blocks
 * \param [in] input input buffer with block_num AES blocks to encrypt
 * \param [out] output output buffer with block_num AES blocks encrypted
 */
void fp_crypto_aes_ecb(const uint8_t *key, uint8_t key_size, FP_CRYPTO_OP op, uint8_t block_num,
        const uint8_t *input, uint8_t *output);

#endif /* FP_CRYPTO_H_ */
