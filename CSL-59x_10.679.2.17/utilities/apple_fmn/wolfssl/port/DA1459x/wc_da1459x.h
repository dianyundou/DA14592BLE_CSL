/**
 ****************************************************************************************
 *
 * @file wc_da1459x.h
 *
 * @brief wolfCrypt port for Renesas DA1459x device header file
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

#ifndef WC_DA1459X_H_
#define WC_DA1459X_H_

#include <stddef.h>
#include <stdint.h>

/**
 * \brief DA1459x HASH context
 */
typedef struct {
        unsigned char *data;    /**< Input data */
        unsigned int len;       /**< Input data length */
} wc_da1459x_hash_context;

/**
 * \brief Calculate SHA-256 HASH
 *
 * This function calculates SHA-256 HASH based on input data that have been set with
 * wc_da1459x_sha256_update(). It shall be called after HASH input data initialization and update,
 * that is, after calling wc_da1459x_sha256_init() and wc_da1459x_sha256_update(), respectively.
 *
 * \param [in] ctx HASH context
 * \param [out] digest HASH output
 *
 * \return 0 on success, <0 otherwise
 */
int wc_da1459x_sha256_final(wc_da1459x_hash_context *ctx, void *digest);

/**
 * \brief Initialize SHA-256 HASH operation
 *
 * This function initializes HASH context.
 *
 * \param [in] ctx HASH context
 */
void wc_da1459x_sha256_init(wc_da1459x_hash_context *ctx);

/**
 * \brief Update SHA-256 HASH
 *
 * This function updates/extends with more input data the HASH context, on which SHA-256 HASH
 * operation will be executed.
 *
 * \param [in] ctx HASH context
 * \param [in] data HASH input data
 * \param [in] len HASH input data length
 *
 * \return 0 on success, <0 otherwise
 */
int wc_da1459x_sha256_update(wc_da1459x_hash_context *ctx, const void *data, uint32_t len);

/**
 * \brief Generate block of random numbers
 *
 * \param [out] output buffer to fill with random numbers
 * \param [in] sz size of buffer
 *
 * \return 0 on success, <0 otherwise
 */
int wc_da1459x_rand_generate_block(uint8_t *output, uint32_t sz);

/**
 * \brief Encrypt with AES in ECB mode
 *
 * \param [in] key AES key
 * \param [in] key_size AES key size
 * \param [in] input input data to encrypt
 * \param [in] output encrypted output data
 *
 * \return 0 on success, <0 otherwise
 */
int wc_da1459x_aes_encrypt(const uint8_t *key, uint8_t key_size, const uint8_t *input,
        uint8_t *output);

/**
 * \brief Decrypt with AES in ECB mode
 *
 * \param [in] key AES key
 * \param [in] key_size AES key size
 * \param [in] input input data to decrypt
 * \param [in] output decrypted output data
 *
 * \return 0 on success, <0 otherwise
 */
int wc_da1459x_aes_decrypt(const uint8_t *key, uint8_t key_size, const uint8_t *input,
        uint8_t *output);

/**
 * \brief Allocate memory
 *
 * \param [in] size size of memory to allocate
 *
 * \return pointer to the allocated memory
 */
void *wc_da1459x_malloc(size_t size);

/**
 * \brief Free allocated memory
 *
 * \param [in] addr address of the allocated memory
 */
void wc_da1459x_free(void *addr);

#endif /* WC_DA1459X_H_ */
