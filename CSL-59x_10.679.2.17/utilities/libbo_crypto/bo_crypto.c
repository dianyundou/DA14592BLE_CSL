/**
 ****************************************************************************************
 *
 * @file bo_crypto.c
 *
 * @brief Cryptographic library implementation.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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
#include <sys/time.h>
#include "aes.h"
#include "ecdsa.h"
#include "ecp.h"
#include "md.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "sha512.h"
#include "bo_crypto.h"
#include "crypto_sign_ed25519.h"
#include "sodium.h"
#include "error.h"


#define INIT_VEC_SIZE           16
#define RNG_INIT_ONCE           1
#define GENERATION_TRY_NUM      30
#define MAX_SHA_LEN             64

/* Elliptic curve basic info */
typedef struct {
        /* Curves's id */
        elliptic_curve_t id;
        /* Curve's id in mbedTLS library */
        mbedtls_ecp_group_id mbedtls_id;
        /* Private key length */
        unsigned int priv_key_len;
        /* Public key length (and ECDSA signature) */
        unsigned int pub_key_len;
        /* Curve's name */
        const char *name;
} elliptic_curve_info_t;

/* Supported elliptic curves */
static const elliptic_curve_info_t supported_elliptic_curves[] = {
        { ELLIPTIC_CURVE_NONE,          MBEDTLS_ECP_DP_NONE,            0,      0,      "" },
        { ELLIPTIC_CURVE_SECP192R1,     MBEDTLS_ECP_DP_SECP192R1,       24,     48,     "SECP192R1" },
        { ELLIPTIC_CURVE_SECP224R1,     MBEDTLS_ECP_DP_SECP224R1,       28,     56,     "SECP224R1" },
        { ELLIPTIC_CURVE_SECP256R1,     MBEDTLS_ECP_DP_SECP256R1,       32,     64,     "SECP256R1" },
        { ELLIPTIC_CURVE_SECP384R1,     MBEDTLS_ECP_DP_SECP384R1,       48,     96,     "SECP384R1" },
        { ELLIPTIC_CURVE_BP256R1,       MBEDTLS_ECP_DP_BP256R1,         32,     64,     "BP256R1" },
        { ELLIPTIC_CURVE_BP384R1,       MBEDTLS_ECP_DP_BP384R1,         48,     96,     "BP384R1" },
        { ELLIPTIC_CURVE_BP512R1,       MBEDTLS_ECP_DP_BP512R1,         64,     128,    "BP512R1" },
        { ELLIPTIC_CURVE_CURVE25519,    MBEDTLS_ECP_DP_CURVE25519,      32,     32,     "CURVE25519" },
        { ELLIPTIC_CURVE_SECP192K1,     MBEDTLS_ECP_DP_SECP192K1,       24,     48,     "SECP192K1" },
        { ELLIPTIC_CURVE_SECP224K1,     MBEDTLS_ECP_DP_SECP224K1,       28,     56,     "SECP224K1" },
        { ELLIPTIC_CURVE_SECP256K1,     MBEDTLS_ECP_DP_SECP256K1,       32,     64,     "SECP256K1" },
        { ELLIPTIC_CURVE_EDWARDS25519,  MBEDTLS_ECP_DP_NONE,            32,     32,     "EDWARDS25519" },
};

/* Initialize random number generator, seed could be passed by 'user_data' parameter */
static void rng_init(void *user_data)
{
        unsigned int seed;
#if RNG_INIT_ONCE
        static bool init_done = false;

        if (init_done) {
                return;
        }

        init_done = true;
#endif

        if (user_data) {
                seed = *((unsigned int *) user_data);
        } else {
                struct timeval tv;

                gettimeofday(&tv, NULL);
                /*
                 * Create seed from seconds and microseconds. Using seconds from 'time' function
                 * only is unsafe. Fast execution of a program, which uses this library few times
                 * in a row will cause the same random seed in multiple calls.
                 */
                seed = tv.tv_sec + tv.tv_usec;
        }

        srand(seed);
}

/* Wrapper for rand function used in mbedTLS library */
static int rng_bytes(void *ud, unsigned char *buff, size_t len)
{
        int i;

        for (i = 0; i < len; i++) {
                /*
                 * Use only the lowest byte, because in Windows 'rand' function returns only 16-bits
                 * values and a sign bit is always 0.
                 */
                buff[i] = (unsigned char) rand();
        }

        /* Error cannot occur in this function - return 0 always */
        return 0;
}

static const elliptic_curve_info_t get_elliptic_curve_info(elliptic_curve_t elliptic_curve)
{
        const size_t supported_elliptic_curves_num = sizeof(supported_elliptic_curves) /
                                                        sizeof(supported_elliptic_curves[0]);
        int i;

        for (i = 0; i < supported_elliptic_curves_num; i++) {
                if (supported_elliptic_curves[i].id == elliptic_curve) {
                        return supported_elliptic_curves[i];
                }
        }

        return supported_elliptic_curves[0];
}


/* Function returns 0 when invalid hash method is selected, hash length otherwise */
static unsigned int compute_hash(hash_method_t hash_method, const uint8_t *input,
                                                        unsigned int input_len, uint8_t *sha)
{
        unsigned int sha_len;

        switch (hash_method) {
        case HASH_METHOD_MD5:
                mbedtls_md5(input, input_len, sha);
                sha_len = 16;
                break;
        case HASH_METHOD_SHA1:
                mbedtls_sha1(input, input_len, sha);
                sha_len = 20;
                break;
        case HASH_METHOD_SHA224:
                mbedtls_sha256(input, input_len, sha, true);
                sha_len = 28;
                break;
        case HASH_METHOD_SHA256:
                mbedtls_sha256(input, input_len, sha, false);
                sha_len = 32;
                break;
        case HASH_METHOD_SHA384:
                mbedtls_sha512(input, input_len, sha, true);
                sha_len = 48;
                break;
        case HASH_METHOD_SHA512:
                mbedtls_sha512(input, input_len, sha, false);
                sha_len = 64;
                break;
        default:
                return 0;
        }

        return sha_len;
}

crypto_buffer_t *crypto_buffer_alloc(size_t buffer_size, const uint8_t *buffer)
{
        crypto_buffer_t *crypto_buffer;

        if (buffer_size < 1) {
                return NULL;
        }

        if (!buffer) {
                crypto_buffer = calloc(1, sizeof(crypto_buffer_t) + buffer_size);
        } else {
                /* Only allocate - this will be set later */
                crypto_buffer = malloc(sizeof(crypto_buffer_t) + buffer_size);
        }

        if (!crypto_buffer) {
                return NULL;
        }

        /* Set size and value pointer */
        crypto_buffer->value = ((uint8_t *) crypto_buffer) + sizeof(crypto_buffer_t);
        crypto_buffer->size = buffer_size;

        if (buffer) {
                memcpy(crypto_buffer->value, buffer, buffer_size);
        }

        return crypto_buffer;
}

void crypto_buffer_init(crypto_buffer_t *crypto_buffer, size_t buffer_size, uint8_t *buffer)
{
        if (!crypto_buffer) {
                return;
        }

        crypto_buffer->size = buffer_size;
        crypto_buffer->value = buffer;
}

void crypto_buffer_free(crypto_buffer_t *buffer)
{
        free(buffer);
}

bool crypto_symmetric_key_gen(crypto_buffer_t *key)
{
        /* Check arguments */
        if (!key || key->size < 1) {
                return false;
        }

        rng_init(NULL);
        rng_bytes(NULL, key->value, key->size);

        return true;
}

static bool crypto_aes_cbc(const uint8_t *key, unsigned int key_len, const uint8_t *init_vec,
                                                const uint8_t *input, unsigned int input_len,
                                                                uint8_t *output, bool encrypt)
{
        int mode = encrypt ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT;
        bool ret = false;
        uint8_t init_vec_cp[INIT_VEC_SIZE];
        mbedtls_aes_context aes;

        /* Check pointers */
        if (!key || !init_vec || !input || !output) {
                return false;
        }

        /* Check lengths */
        if ((key_len != 16 && key_len != 24 && key_len != 32) || (input_len % 16)) {
                return false;
        }

        /*
         * Initialization vector is not constant in mbedtls_aes_crypt_cbc - create copy from
         * arguments.
         */
        memcpy(init_vec_cp, init_vec, INIT_VEC_SIZE);

        mbedtls_aes_init(&aes);

        if (encrypt) {
                if (mbedtls_aes_setkey_enc(&aes, key, key_len * 8)) {
                        goto cleanup;
                }
        } else {
                if (mbedtls_aes_setkey_dec(&aes, key, key_len * 8)) {
                        goto cleanup;
                }
        }

        if (mbedtls_aes_crypt_cbc(&aes, mode, input_len, init_vec_cp, input, output)) {
                goto cleanup;
        }

        ret = true;

cleanup:
        mbedtls_aes_free(&aes);

        return ret;
}

bool crypto_aes_cbc_encrypt(const crypto_buffer_t *key, const uint8_t *init_vec,
                                                const crypto_buffer_t *input, uint8_t *output)
{
        return crypto_aes_cbc(key->value, key->size, init_vec, input->value, input->size, output,
                                                                                        true);
}

bool crypto_aes_cbc_decrypt(const crypto_buffer_t *key, const uint8_t *init_vec,
                                                const crypto_buffer_t *input,  uint8_t *output)
{
        return crypto_aes_cbc(key->value, key->size, init_vec, input->value, input->size, output,
                                                                                        false);
}

bool crypto_aes_ctr_encrypt(const crypto_buffer_t *key, const uint8_t *nonce,
                                                const crypto_buffer_t *input,  uint8_t *output)
{
        bool ret = false;
        uint8_t nonce_counter[INIT_VEC_SIZE];
        uint8_t stream_block[INIT_VEC_SIZE];
        mbedtls_aes_context aes;
        /* mod 16 counter of already encrypted bytes */
        size_t nc_off = 0;

        /* Check pointers */
        if (!key || !nonce || !input || !output) {
                return false;
        }

        /* Check lengths */
        if ((key->size != 16 && key->size != 24 && key->size != 32) || (input->size % 16)) {
                return false;
        }

        /* Initialization nonce_counter with user provided nonce. */
        memcpy(nonce_counter, nonce, INIT_VEC_SIZE);

        mbedtls_aes_init(&aes);

        if (mbedtls_aes_setkey_enc(&aes, key->value, key->size * 8)) {
                goto cleanup;
        }

        if (mbedtls_aes_crypt_ctr(&aes, input->size, &nc_off, nonce_counter, stream_block,
                                                                        input->value, output)) {
                goto cleanup;
        }

        ret = true;

cleanup:
        mbedtls_aes_free(&aes);

        return ret;
}

bool crypto_asymmetric_key_pair_gen(elliptic_curve_t elliptic_curve, crypto_buffer_t *priv_key,
                                                                        crypto_buffer_t *pub_key)
{
        bool ret = false;
        size_t d_len;
        elliptic_curve_info_t ec_info;
        mbedtls_ecp_group grp;
        mbedtls_ecp_point Q;
        mbedtls_mpi d;
        size_t len;
        int i;
        int res = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;


        /* Check pointers */
        if (!priv_key || !pub_key) {
                return false;
        }

        /* Check arguments */
        if (elliptic_curve <= ELLIPTIC_CURVE_NONE || (priv_key->size == 0) || (pub_key->size == 0)) {
                return false;
        }

        ec_info = get_elliptic_curve_info(elliptic_curve);
        rng_init(NULL);


        /* Edwards 25519 curve handled by libsodium library */

        if (elliptic_curve == ELLIPTIC_CURVE_EDWARDS25519) {
                /* sk is concatenation of public and private key */
                uint8_t sk[crypto_sign_ed25519_secretkeybytes()];
                /* Private key is 32 bytes long integer */
                rng_bytes(NULL, priv_key->value, priv_key->size);
                crypto_sign_ed25519_seed_keypair(pub_key->value, sk, priv_key->value);
                memset(sk, 0, crypto_sign_ed25519_secretkeybytes());
                priv_key->size = ec_info.priv_key_len;
                pub_key->size = ec_info.pub_key_len;
                return true;
        }



        mbedtls_ecp_group_init( &grp );
        mbedtls_mpi_init( &d );
        mbedtls_ecp_point_init( &Q );

        d_len = ec_info.priv_key_len;

        /* Private key will be longer than buffer */
        if (d_len > priv_key->size) {
                goto cleanup;
        }

        if (ec_info.pub_key_len > pub_key->size) {
                       goto cleanup;
        }


        for (i = 0; i < GENERATION_TRY_NUM; i++) {

                if ( ( res = mbedtls_ecp_group_load( &grp, ec_info.mbedtls_id ) ) == 0 ) {
                        res =  mbedtls_ecp_gen_keypair( &grp, &d, &Q, rng_bytes, NULL );
                }

                if ( res != 0 ) {
                         goto cleanup;
                }

                /*
                 * Very rarely using e.g. secp224k1 elliptic curve the private key has one bit more
                 * than expected. In that case try generate it again.
                 */
                if (mbedtls_mpi_bitlen(&d) <= d_len * 8) {
                        goto proper_size;
                }
        }

        goto cleanup;

proper_size:

        /* Set new keys sizes */
        priv_key->size = d_len;

        if (mbedtls_mpi_write_binary(&d, priv_key->value, d_len)) {
                goto cleanup;
        }

        mbedtls_ecp_point_write_binary(&grp, &Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &len, pub_key->value, pub_key->size);

        pub_key->size = len;
        ret = true;

cleanup:
        mbedtls_ecp_group_free( &grp );
        mbedtls_mpi_free( &d );
        mbedtls_ecp_point_free( &Q );

        return ret;
}



static int crypto_ecp_check_pub_priv(const mbedtls_ecp_group *grp, const mbedtls_mpi *d, const mbedtls_ecp_point *Q)
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    mbedtls_ecp_point Q2;
    mbedtls_ecp_group grp2;

    if ( grp->id == MBEDTLS_ECP_DP_NONE )
    {
        return( MBEDTLS_ERR_ECP_BAD_INPUT_DATA );
    }

    mbedtls_ecp_point_init( &Q2 );
    mbedtls_ecp_group_init( &grp2 );

    /* mbedtls_ecp_mul() needs a non-const group... */
    mbedtls_ecp_group_copy( &grp2, grp );

    /* Also checks d is valid */
    MBEDTLS_MPI_CHK( mbedtls_ecp_mul( &grp2, &Q2, d, &grp->G, rng_bytes, NULL ) );

    ret = mbedtls_ecp_point_cmp(&Q2, Q);
    if ( ret == MBEDTLS_ERR_ECP_BAD_INPUT_DATA )
    {
        goto cleanup;
    }



cleanup:
    mbedtls_ecp_point_free( &Q2 );
    mbedtls_ecp_group_free( &grp2 );

    return( ret );
}




bool crypto_asymmetric_key_pair_valid(elliptic_curve_t elliptic_curve, const crypto_buffer_t *priv_key,
                                                        const crypto_buffer_t *pub_key, bool *result)
{
        bool ret = false;
        elliptic_curve_info_t ec_info;
        mbedtls_ecp_group grp;
        mbedtls_ecp_point Q;
        mbedtls_mpi d;

        /* Check pointers */
        if (!priv_key || !pub_key) {
                return false;
        }

        /* Check arguments */
        if (elliptic_curve <= ELLIPTIC_CURVE_NONE || (priv_key->size == 0) || (pub_key->size == 0)) {
                return false;
        }

        ec_info = get_elliptic_curve_info(elliptic_curve);

        /* Defined private and public keys lengths are always even */
        if (ec_info.priv_key_len != priv_key->size) {
                return false;
        }

        if ( elliptic_curve != ELLIPTIC_CURVE_CURVE25519 && elliptic_curve != ELLIPTIC_CURVE_EDWARDS25519 ) {
                if ( ec_info.pub_key_len != (pub_key->size -1) ) {
                        return false;
                }
        } else {
                if ( ec_info.pub_key_len != pub_key->size ) {
                        return false;
                }
        }


        /* Edwards 25519 curve handled by libsodium library */
        if (elliptic_curve == ELLIPTIC_CURVE_EDWARDS25519) {
                /* sk is concatenation of public and private key */
                uint8_t sk[crypto_sign_ed25519_secretkeybytes()];
                uint8_t pk[crypto_sign_ed25519_publickeybytes()];

                crypto_sign_ed25519_seed_keypair(pk, sk, priv_key->value);
                memset(sk, 0, crypto_sign_ed25519_secretkeybytes());

                *result = !memcmp(pk, pub_key->value, crypto_sign_ed25519_publickeybytes());
                return true;
        }

        mbedtls_ecp_group_init( &grp );
        mbedtls_mpi_init( &d );
        mbedtls_ecp_point_init( &Q );

        mbedtls_ecp_group_load(&grp, ec_info.mbedtls_id);

        if ( mbedtls_mpi_read_binary(&d, priv_key->value, priv_key->size) ) {
                goto cleanup;
        }


        if ( mbedtls_ecp_point_read_binary( &grp, &Q, pub_key->value, pub_key->size) ) {
                goto cleanup;
        }


        if (mbedtls_ecp_check_privkey(&grp, &d)) {
                goto cleanup;
        }

        if (mbedtls_ecp_check_pubkey(&grp, &Q)) {
                goto cleanup;
        }


        /*
         * Keys don't match or error occurs - assume that the first option. Possible errors should
         * be handled by the previous checks.
         */
        if ( !crypto_ecp_check_pub_priv(&grp, &d, &Q) ) {

                *result = true;
        } else {
                *result = false;
        }

        ret = true;

cleanup:
        mbedtls_ecp_group_free( &grp );
        mbedtls_mpi_free( &d );
        mbedtls_ecp_point_free( &Q );

        return ret;
}

bool crypto_ecdsa_sig_gen(elliptic_curve_t elliptic_curve, hash_method_t hash_method,
                                const crypto_buffer_t *priv_key, const crypto_buffer_t *input,
                                                                        crypto_buffer_t *sig)
{
        bool ret = false;
        unsigned int sha_len;
        uint8_t sha[MAX_SHA_LEN];
        elliptic_curve_info_t ec_info;
        mbedtls_ecp_group ecp_group;
        mbedtls_mpi r, s, d;
        size_t r_len, s_len;

        /* Check pointers */
        if (!priv_key || !input || !sig) {
                return false;
        }

        /* Check arguments */
        if (elliptic_curve <= ELLIPTIC_CURVE_NONE ||
                elliptic_curve == ELLIPTIC_CURVE_EDWARDS25519 || hash_method <= HASH_METHOD_NONE ||
                                (priv_key->size == 0) || (input->size == 0) || (sig->size == 0)) {
                return false;
        }

        ec_info = get_elliptic_curve_info(elliptic_curve);

        if (ec_info.priv_key_len != priv_key->size || ec_info.pub_key_len > sig->size) {
                return false;
        }

        sha_len = compute_hash(hash_method, input->value, input->size, sha);

        if (sha_len == 0) {
                return false;
        }

        mbedtls_mpi_init(&d);
        mbedtls_mpi_init(&r);
        mbedtls_mpi_init(&s);
        mbedtls_ecp_group_init(&ecp_group);
        mbedtls_ecp_group_load(&ecp_group, ec_info.mbedtls_id);
        rng_init(NULL);


        r_len = ec_info.pub_key_len / 2;
        s_len = ec_info.pub_key_len / 2;

        /* Signature will be longer than buffer */
        if ((r_len + s_len) > sig->size) {
                goto cleanup;
        }

        if (mbedtls_mpi_read_binary(&d, priv_key->value, priv_key->size)) {
                goto cleanup;
        }

        /* Curve25519 could not be used in ECDSA - function will return an error */
        if (mbedtls_ecdsa_sign(&ecp_group, &r, &s, &d, sha, sha_len, rng_bytes, NULL)) {
                goto cleanup;
        }

        sig->size = r_len + s_len;

        if (mbedtls_mpi_write_binary(&r, sig->value, r_len)) {
                goto cleanup;
        }

        if (mbedtls_mpi_write_binary(&s, sig->value + r_len, s_len)) {
                goto cleanup;
        }

        ret = true;

cleanup:
        mbedtls_mpi_free(&d);
        mbedtls_mpi_free(&r);
        mbedtls_mpi_free(&s);
        mbedtls_ecp_group_free(&ecp_group);

        return ret;
}

bool crypto_ecdsa_sig_valid(elliptic_curve_t elliptic_curve, hash_method_t hash_method,
                                        const crypto_buffer_t *pub_key, const crypto_buffer_t *input,
                                                        const crypto_buffer_t *sig, bool *result)
{
        bool ret = false;
        unsigned int sha_len;
        uint8_t sha[MAX_SHA_LEN];
        elliptic_curve_info_t ec_info;
        mbedtls_ecp_group ecp_group;
        mbedtls_mpi r, s;
        mbedtls_ecp_point q;

        /* Check pointers */
        if (!pub_key || !input || !sig || !result) {
                return false;
        }

        /* Check arguments */
        if (elliptic_curve <= ELLIPTIC_CURVE_NONE ||
                elliptic_curve == ELLIPTIC_CURVE_EDWARDS25519 || hash_method <= HASH_METHOD_NONE ||
                                (pub_key->size == 0) || (input->size == 0) || (sig->size == 0)) {
                return false;
        }

        ec_info = get_elliptic_curve_info(elliptic_curve);

        if (ec_info.pub_key_len != pub_key->size || ec_info.pub_key_len != sig->size) {
                return false;
        }

        sha_len = compute_hash(hash_method, input->value, input->size, sha);

        if (sha_len == 0) {
                return false;
        }

        mbedtls_ecp_point_init(&q);
        mbedtls_mpi_init(&r);
        mbedtls_mpi_init(&s);
        mbedtls_ecp_group_init(&ecp_group);
        mbedtls_ecp_group_load(&ecp_group, ec_info.mbedtls_id);

        if ( mbedtls_ecp_point_read_binary( &ecp_group, &q, pub_key->value, pub_key->size) ) {
                goto cleanup;
        }

        if (mbedtls_mpi_read_binary(&r, sig->value, sig->size / 2)) {
                goto cleanup;
        }

        if (mbedtls_mpi_read_binary(&s, sig->value + sig->size / 2 , sig->size / 2)) {
                goto cleanup;
        }

        if (!mbedtls_ecdsa_verify(&ecp_group, sha, sha_len, &q, &r, &s)) {
                *result = true;
        } else {
                *result = false;
        }

        ret = true;

cleanup:
        mbedtls_ecp_point_free(&q);
        mbedtls_mpi_free(&r);
        mbedtls_mpi_free(&s);
        mbedtls_ecp_group_free(&ecp_group);

        return ret;
}

bool DLLEXPORT crypto_eddsa_sig_gen(elliptic_curve_t elliptic_curve,
                                const crypto_buffer_t *priv_key, const crypto_buffer_t *input,
                                                                        crypto_buffer_t *sig)
{
        unsigned long long sig_len;
        unsigned char pk[crypto_sign_ed25519_publickeybytes()];
        unsigned char sk[crypto_sign_ed25519_secretkeybytes()];

        /* Only Edwards 25519 curve is supported */
        if (elliptic_curve != ELLIPTIC_CURVE_EDWARDS25519) {
                return false;
        }

        /* Check pointers */
        if (!priv_key || !input || !sig) {
                return false;
        }

        if (priv_key->size != crypto_sign_ed25519_seedbytes()) {
                return false;
        }

        /* EDDSA signature is 64 bytes long */
        if (sig->size < 64) {
                return false;
        }
        crypto_sign_ed25519_seed_keypair(pk, sk, priv_key->value);
        crypto_sign_ed25519_detached(sig->value, &sig_len, input->value, input->size, sk);
        if (sig_len < sig->size) {
                sig->size = sig_len;
        }

        return true;
}

bool DLLEXPORT crypto_eddsa_sig_valid(elliptic_curve_t elliptic_curve,
                                const crypto_buffer_t *pub_key, const crypto_buffer_t *input,
                                                        const crypto_buffer_t *sig, bool *result)
{
        /* Only Edwards 25519 curve is supported */
        if (elliptic_curve != ELLIPTIC_CURVE_EDWARDS25519) {
                return false;
        }

        /* Check pointers */
        if (!pub_key || !input || !sig || !result) {
                return false;
        }

        if (pub_key->size != crypto_sign_ed25519_publickeybytes()) {
                return false;
        }

        *result = !crypto_sign_ed25519_verify_detached(sig->value, input->value, input->size,
                                                                                pub_key->value);

        return true;
}
