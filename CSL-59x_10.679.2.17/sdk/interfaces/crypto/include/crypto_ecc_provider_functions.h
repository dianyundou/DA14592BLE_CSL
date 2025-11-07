/**
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \brief Elliptic curves primitives and basic datatypes.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_functions.h
 *
 * @brief Elliptic curves primitives and basic datatypes. Depending on ecc library and
 *        curve settings, the primitives provided from this header will be mapped to the
 *        respective functions.
 *
 * Copyright (C) 2018 Renesas Electronics Corporation and/or its affiliates.
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


#ifndef SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_
#define SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_

#include <sys_trng.h>
#include <ad_crypto.h>
#include <hw_ecc.h>
#include "sodium.h"
#include "uECC.h"
#include <crypto_ecc_provider_functions.h>
#include <crypto_ec.h>

/* Compute private key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_private_key_uecc(crypto_ec_params_t *curve, uint8_t *d);
CRYPTO_ECC_PROVIDER_RET compute_private_key_25519(crypto_ec_params_t *curve, uint8_t *d);
CRYPTO_ECC_PROVIDER_RET compute_private_key_hw(crypto_ec_params_t *curve, uint8_t *d);

/* Compute public key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);

/* Compute shared key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);

/* Curve init functions .*/
CRYPTO_ECC_PROVIDER_RET curve_init_secp160r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp192r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp224r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp256r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp256k1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_25519(crypto_ec_params_t *curve);

/* This is the error handling function of ecc library providing mechanism.
 * In case a call to a curve or a library that is not provided depending
 * on settings in crypto_ecc_provider_params.h this function will be called.
 */
CRYPTO_ECC_PROVIDER_RET ecc_lib_provider_error_handler();

#endif /* SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_ */
/**
 * \}
 * \}
 */
