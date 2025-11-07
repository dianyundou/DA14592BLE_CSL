/**
 ****************************************************************************************
 *
 * @file wc_da1459x.c
 *
 * @brief wolfCrypt port for Renesas DA1459x device
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

#include <stdlib.h>
#include <string.h>
#include "hw_aes_hash.h"
#include "hw_aes.h"
#include "hw_hash.h"
#include "osal.h"
#include "ad_crypto.h"

#include "wc_da1459x.h"

/* The length of SHA256 in bytes */
#define CRYPTO_SHA256_BYTES_LEN         32
/* A single AES block length */
#define CRYPTO_AES_BLOCK_LENGTH         16

typedef enum {
        ENCRYPT,
        DECRYPT
} operation_t;

/* Calculate SHA-256 of the input buffer of n length */
static int da1459x_sha256(uint8_t *output, const uint8_t *input, uint32_t n)
{
        ad_crypto_handle_t crypto_hdl;
        ad_crypto_config_t cfg = {
                .algo = AD_CRYPTO_ALGO_HASH,
                .engine.hash = {
                        .type = HW_HASH_TYPE_SHA_256,
                        .wait_more_input = false,
                        .input_data_len = n,
                        .output_data_len = CRYPTO_SHA256_BYTES_LEN,
                        .input_data_addr = (uint32_t) input,
                        .output_data_addr = (uint32_t) output
                }
        };

        crypto_hdl = ad_crypto_open(&cfg, OS_MUTEX_FOREVER);
        OS_ASSERT(crypto_hdl);

        if (crypto_hdl != NULL) {
                hw_aes_hash_start();
                while (hw_aes_hash_is_active());

                ad_crypto_close();
        }

        return (crypto_hdl != NULL) ? 0 : -1;
}

int wc_da1459x_sha256_final(wc_da1459x_hash_context *ctx, void *digest)
{
        int ret = da1459x_sha256(digest, ctx->data, ctx->len);

        if (ctx->data != NULL) {
                OS_FREE(ctx->data);
                ctx->data = NULL;
                ctx->len = 0;
        }

        return ret;
}

void wc_da1459x_sha256_init(wc_da1459x_hash_context *ctx)
{
        ctx->data = NULL;
        ctx->len = 0;
}

int wc_da1459x_sha256_update(wc_da1459x_hash_context *ctx, const void *data, uint32_t len)
{
        uint32_t newlen = ctx->len + len;
        uint8_t *newdata = (uint8_t *) OS_MALLOC(newlen);
        OS_ASSERT(newdata);

        if (newdata) {
                if (ctx->data != NULL) {
                        memcpy(newdata, ctx->data, ctx->len);
                        OS_FREE(ctx->data);
                }
                memcpy(newdata + ctx->len, data, len);
                ctx->data = newdata;
                ctx->len = newlen;
        }

        return (newdata != NULL) ? 0 : -1;
}

int wc_da1459x_rand_generate_block(uint8_t *output, uint32_t sz)
{
        uint32_t i = 0;
        int random_number;
        uint8_t bytes_to_copy;

        while (sz) {
                random_number = rand();
                if (sz > sizeof(int)) {
                        bytes_to_copy = sizeof(int);
                        sz -= sizeof(int);
                } else {
                        bytes_to_copy = sz;
                        sz = 0;
                }
                memcpy(&output[i], &random_number, bytes_to_copy);
                i += bytes_to_copy;
        }
        return 0;
}

/* AES crypto operation execution */
static int da1459x_aes_crypt(const uint8_t *key, uint8_t key_size, const uint8_t *input,
        uint8_t *output, operation_t op)
{
        HW_AES_KEY_SIZE key_sz;

        if (key_size == 32) {
                key_sz = HW_AES_KEY_SIZE_256;
        } else if (key_size == 16) {
                key_sz = HW_AES_KEY_SIZE_128;
        } else {
                return -1;
        }

        ad_crypto_handle_t crypto_hdl;
        ad_crypto_config_t cfg = {
                .algo = AD_CRYPTO_ALGO_AES,
                .engine.aes = {
                        .mode = HW_AES_MODE_ECB,
                        .operation = op == DECRYPT ? HW_AES_OPERATION_DECRYPT : HW_AES_OPERATION_ENCRYPT,
                        .key_size = key_sz,
                        .key_expand = HW_AES_KEY_EXPAND_BY_HW,
                        .output_data_mode = HW_AES_OUTPUT_DATA_MODE_ALL,
                        .wait_more_input = false,
                        .keys_addr = (uint32_t) key
                }
        };

        crypto_hdl = ad_crypto_open(&cfg, OS_MUTEX_FOREVER);
        OS_ASSERT(crypto_hdl);

        if (crypto_hdl != NULL) {
                hw_aes_hash_set_input_data_addr((uint32_t) input);
                hw_aes_hash_set_output_data_addr((uint32_t) output);
                hw_aes_hash_set_input_data_len(CRYPTO_AES_BLOCK_LENGTH);
                hw_aes_start_operation(op == DECRYPT ? HW_AES_OPERATION_DECRYPT : HW_AES_OPERATION_ENCRYPT);
                while (hw_aes_hash_is_active());

                ad_crypto_close();
        }

        return (crypto_hdl != NULL) ? 0 : -1;
}

int wc_da1459x_aes_encrypt(const uint8_t *key, uint8_t key_size, const uint8_t *input,
        uint8_t *output)
{
        return da1459x_aes_crypt(key, key_size, input, output, ENCRYPT);
}

int wc_da1459x_aes_decrypt(const uint8_t *key, uint8_t key_size, const uint8_t *input,
        uint8_t *output)
{
        return da1459x_aes_crypt(key, key_size, input, output, DECRYPT);
}

void *wc_da1459x_malloc(size_t size)
{
        return OS_MALLOC(size);
}

void wc_da1459x_free(void *addr)
{
        OS_FREE(addr);
}
