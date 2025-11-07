/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_params.c
 *
 * @brief ecc parameter arrays initialization.
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

#include "crypto_ecc_provider_params.h"
#include <stdint.h>


ECC_CRYPTO_LIB_PROVIDER curve_lib_provider_index[ECC_CRYPTO_LAST_VALUE] =  { CURVE_SECP160R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP192R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP224R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP256R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP256K1_DEFAULT_LIB_PROVIDER,
        CURVE_25519_DEFAULT_LIB_PROVIDER};

/* This is used in order to check if a library provider is available and if it is
 * implementing a certain curve, it will be marked as true in the lib provider truthtable.
 */
#if dg_USE_HW_ECC
#define HW_ECC_TRUTHTABLE_FLAG true
#else
#define HW_ECC_TRUTHTABLE_FLAG false
#endif
#if dg_USE_UECC_LIB
#define UECC_TRUTHTABLE_FLAG true
#else
#define UECC_TRUTHTABLE_FLAG false
#endif
#if dg_USE_SODIUM_LIB
#define SODIUM_TRUTHTABLE_FLAG true
#else
#define SODIUM_TRUTHTABLE_FLAG false
#endif

#if dg_USE_CURVE_SECP160R1
#define dg_USE_CURVE_SECP160R1_FLAG true
#else
#define dg_USE_CURVE_SECP160R1_FLAG false
#endif
#if dg_USE_CURVE_SECP192R1
#define dg_USE_CURVE_SECP192R1_FLAG true
#else
#define dg_USE_CURVE_SECP192R1_FLAG false
#endif
#if dg_USE_CURVE_SECP224R1
#define dg_USE_CURVE_SECP224R1_FLAG true
#else
#define dg_USE_CURVE_SECP224R1_FLAG false
#endif
#if dg_USE_CURVE_SECP256R1
#define dg_USE_CURVE_SECP256R1_FLAG true
#else
#define dg_USE_CURVE_SECP256R1_FLAG false
#endif
#if dg_USE_CURVE_SECP256K1
#define dg_USE_CURVE_SECP256K1_FLAG true
#else
#define dg_USE_CURVE_SECP256K1_FLAG false
#endif
#if dg_USE_CURVE_25519
#define dg_USE_CURVE_25519_FLAG true
#else
#define dg_USE_CURVE_25519_FLAG false
#endif

/*
 * Truthtable that provides info which library provides which curve, for sanitizing input when
 * setting libprovider during curve initialization.
 * Row order must be the same as CRYPTO_ECC_CURVE
 * Column order must be the same as ECC_CRYPTO_LIB_PROVIDER
 *                                    |hw_ecc | uecc | sodium |
 *                         secp160r1: |   F   |   T  |    F   |
 *                         secp192r1: |   T   |   T  |    F   |
 *                         secp224r1: |   T   |   T  |    F   |
 *                         secp256r1: |   T   |   T  |    F   |
 *                         secp256k1: |   F   |   T  |    F   |
 *                        curve25519: |   T   |   F  |    T   |
 */
const bool lib_provider_truthtable[ECC_CRYPTO_LAST_VALUE][CRYPTO_ECC_PROVIDER_LAST_VALUE] =
{      //|                      hw_ecc                        |                      uecc                          |                     sodium                          |
       { false,                                                 UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP160R1_FLAG,  false                                              },    //secp160r1: |   F   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP192R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP192R1_FLAG,  false                                              },    //secp192r1: |   T   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP224R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP224R1_FLAG,  false                                              },    //secp224r1: |   T   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256R1_FLAG,  false                                              },    //secp256r1: |   T   |   T  |    F   |
       { false,                                                 UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256K1_FLAG,  false                                              },    //secp256k1: |   F   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_25519_FLAG,     false,                                                SODIUM_TRUTHTABLE_FLAG && dg_USE_CURVE_25519_FLAG  }    //curve25519: |   T   |   F  |    T   |

};
/**
 * \}
 * \}
 * \}
 * \}
 */
