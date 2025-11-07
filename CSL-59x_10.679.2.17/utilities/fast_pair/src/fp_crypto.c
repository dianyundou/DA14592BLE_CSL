/**
 ****************************************************************************************
 *
 * @file fp_crypto.c
 *
 * @brief Google Fast Pair crypto helper module implementation
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hw_aes_hash.h"
#include "hw_aes.h"
#include "hw_hash.h"
#include "osal.h"
#include "ad_crypto.h"

#include "fp_crypto.h"

#if (FP_AES_HW == 0)
#include "aes.h"
#include "sha256.h"
#include "md.h"
#endif

#if (FP_AES_HW == 0)
/* MbedTLS AES context structure */
static mbedtls_aes_context mbedtls_aes_ctx;
#endif

#if (FP_AES_HW == 1)
static HW_AES_KEY_SIZE get_key_size(uint8_t key_size)
{
        HW_AES_KEY_SIZE hw_aes_key_size = HW_AES_KEY_SIZE_128;

        switch (key_size) {
        case 32:
                hw_aes_key_size = HW_AES_KEY_SIZE_256;
                break;
        case 16:
                hw_aes_key_size = HW_AES_KEY_SIZE_128;
                break;
        default:
                /* Unsupported key size */
                OS_ASSERT(0);
        }
        return hw_aes_key_size;
}
#endif

/* AES initialization */
static void aes_init(const uint8_t *key, uint8_t key_size, FP_CRYPTO_OP op)
{
#if (FP_AES_HW == 1)
        ad_crypto_handle_t crypto_hdl;
        ad_crypto_config_t cfg = {
                .algo = AD_CRYPTO_ALGO_AES,
                .engine.aes = {
                        .mode = HW_AES_MODE_ECB,
                        .operation = op == FP_CRYPTO_OP_DECRYPT ? HW_AES_OPERATION_DECRYPT :
                                                                  HW_AES_OPERATION_ENCRYPT,
                        .key_size = get_key_size(key_size),
                        .key_expand = HW_AES_KEY_EXPAND_BY_HW,
                        .output_data_mode = HW_AES_OUTPUT_DATA_MODE_ALL,
                        .wait_more_input = false,
                        .keys_addr = (uint32_t) key
                }
        };

        crypto_hdl = ad_crypto_open(&cfg, OS_MUTEX_FOREVER);
        OS_ASSERT(crypto_hdl);
#else
        mbedtls_aes_init(&mbedtls_aes_ctx);
        if (op == FP_CRYPTO_OP_ENCRYPT) {
                mbedtls_aes_setkey_enc(&mbedtls_aes_ctx, key, key_size == 32 ? 256 : 128);
        } else {
                mbedtls_aes_setkey_dec(&mbedtls_aes_ctx, key, key_size == 32 ? 256 : 128);
        }
#endif /* FP_AES_HW */
}

/* AES crypto operation execution */
static void aes_crypt(const uint8_t *input, uint8_t *output, FP_CRYPTO_OP op)
{
#if (FP_AES_HW == 1)
        hw_aes_hash_set_input_data_addr((uint32_t) input);
        hw_aes_hash_set_output_data_addr((uint32_t) output);
        hw_aes_hash_set_input_data_len(FP_CRYPTO_AES_BLOCK_LENGTH);
        hw_aes_start_operation(op == FP_CRYPTO_OP_DECRYPT ? HW_AES_OPERATION_DECRYPT :
                                                            HW_AES_OPERATION_ENCRYPT);
        while (hw_aes_hash_is_active());
#else
        mbedtls_aes_crypt_ecb(&mbedtls_aes_ctx,
                op == FP_CRYPTO_OP_DECRYPT ? MBEDTLS_AES_DECRYPT : MBEDTLS_AES_ENCRYPT,
                input, output);
#endif /* FP_AES_HW */
}

#if (FP_AES_HW == 1)
/* Calculate HMAC-SHA-256 */
static void aes_hmac_hash(ad_crypto_config_t *cfg, const uint8_t *pad, const uint8_t *input,
        uint8_t input_length, uint8_t *output)
{
        hw_aes_hash_set_output_data_addr((uint32_t) output);

        cfg->engine.hash.wait_more_input = true;
        cfg->engine.hash.input_data_len = 64;
        cfg->engine.hash.input_data_addr = (uint32_t) pad;
        ad_crypto_configure_for_next_fragment(cfg);
        hw_aes_hash_start();
        while (hw_aes_hash_waiting_for_input_data());

        cfg->engine.hash.wait_more_input = false;
        cfg->engine.hash.input_data_len = input_length;
        cfg->engine.hash.input_data_addr = (uint32_t) input;
        ad_crypto_configure_for_next_fragment(cfg);
        hw_aes_hash_start();
        while (hw_aes_hash_is_active());
}
#endif /* FP_AES_HW */

/* Cleanup after the AES operation execution */
static void aes_free(void)
{
#if (FP_AES_HW == 1)
        ad_crypto_close();
#else
        mbedtls_aes_free(&mbedtls_aes_ctx);
#endif /* FP_AES_HW */
}

void fp_crypto_sha256(uint8_t *output, const uint8_t *input, uint32_t n)
{
#if (FP_AES_HW == 1)
        ad_crypto_handle_t crypto_hdl;
        ad_crypto_config_t cfg = {
                .algo = AD_CRYPTO_ALGO_HASH,
                .engine.hash = {
                        .type = HW_HASH_TYPE_SHA_256,
                        .wait_more_input = false,
                        .input_data_len = n,
                        .output_data_len = FP_CRYPTO_SHA256_BYTES_LEN,
                        .input_data_addr = (uint32_t) input,
                        .output_data_addr = (uint32_t) output
                }
        };

        crypto_hdl = ad_crypto_open(&cfg, OS_MUTEX_FOREVER);
        OS_ASSERT(crypto_hdl);

        hw_aes_hash_start();
        while (hw_aes_hash_is_active());

        ad_crypto_close();
#else
        mbedtls_sha256(input, n, output, 0);
#endif /* FP_AES_HW */
}

uint16_t fp_crypto_get_salt(void)
{
        return (uint16_t)rand();
}

void fp_crypto_hmac_sha256(const uint8_t *input, uint8_t length, const uint8_t *key,
        uint8_t key_length, uint8_t *output)
{
#if (FP_AES_HW == 1)
        uint8_t ipad[64] = { 0 };
        uint8_t opad[64] = { 0 };
        uint8_t digest[FP_CRYPTO_SHA256_BYTES_LEN];

        memcpy(ipad, key, key_length);
        memcpy(opad, key, key_length);
        for (int i = 0; i < 64; ++i) {
                ipad[i] ^= 0x36;
                opad[i] ^= 0x5C;
        }

        ad_crypto_handle_t crypto_hdl;
        ad_crypto_config_t cfg = {
                .algo = AD_CRYPTO_ALGO_HASH,
                .engine.hash = {
                        .type = HW_HASH_TYPE_SHA_256,
                        .wait_more_input = false,
                        .input_data_len = 64,
                        .output_data_len = FP_CRYPTO_SHA256_BYTES_LEN,
                }
        };

        crypto_hdl = ad_crypto_open(&cfg, OS_MUTEX_FOREVER);
        OS_ASSERT(crypto_hdl);

        aes_hmac_hash(&cfg, ipad, input, length, digest);
        aes_hmac_hash(&cfg, opad, digest, FP_CRYPTO_SHA256_BYTES_LEN, output);

        ad_crypto_close();
#else
        mbedtls_md_context_t mbedtls_md_ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

        mbedtls_md_init(&mbedtls_md_ctx);
        mbedtls_md_setup(&mbedtls_md_ctx, mbedtls_md_info_from_type(md_type), 1);
        mbedtls_md_hmac_starts(&mbedtls_md_ctx, key, key_length);
        mbedtls_md_hmac_update(&mbedtls_md_ctx, input, length);
        mbedtls_md_hmac_finish(&mbedtls_md_ctx, output);
        mbedtls_md_free(&mbedtls_md_ctx);
#endif /* FP_AES_HW */
}

void fp_crypto_aes_ctr(const uint8_t *input, uint8_t length, uint8_t *nonce, const uint8_t *key,
        FP_CRYPTO_OP op, uint8_t *output)
{
        uint8_t stream_block[FP_CRYPTO_AES_BLOCK_LENGTH] = { 0 };
        int c;
        size_t n = 0;

        aes_init(key, FP_CRYPTO_AES_BLOCK_LENGTH, op);
        while (length--) {
                if (n == 0) {
                        aes_crypt(nonce, stream_block, op);
                        ++nonce[0];
                }
                c = *input++;
                *output++ = (uint8_t)(c ^ stream_block[n]);
                n = ( n + 1 ) & 0x0F;
        }
        aes_free();
}

void fp_crypto_aes_ecb(const uint8_t *key, uint8_t key_size, FP_CRYPTO_OP op, uint8_t block_num,
        const uint8_t *input, uint8_t *output)
{
        uint8_t i;

        aes_init(key, key_size, op);
        for (i = 0; i < block_num; i++) {
                aes_crypt(&input[FP_CRYPTO_AES_BLOCK_LENGTH * i],
                        &output[FP_CRYPTO_AES_BLOCK_LENGTH * i], op);
        }
        aes_free();
}
