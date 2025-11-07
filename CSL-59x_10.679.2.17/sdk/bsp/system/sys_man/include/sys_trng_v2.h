/**
 ****************************************************************************************
 *
 * @file sys_trng_v2.h
 *
 * @brief sys_trng_v2 header file.
 *
 *
 * Copyright (C) 2020-2023 Renesas Electronics Corporation and/or its affiliates.
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

/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TRNG True Random Number Generator
 *
 * \brief Random number generation
 *
 * \{
 *
 */

#ifndef SYS_TRNG_V2_H_
#define SYS_TRNG_V2_H_


#if dg_configUSE_SYS_TRNG

#include <stdint.h>
#include "sdk_defs.h"
#include "iid_platform.h"
#include "iid_irng.h"
#include "iid_aes_types.h"
#include "iid_return_codes.h"

/*
 * MACROS
 *****************************************************************************************
 */

 /**
 * \brief The size of TRNG seed in bytes.
 */
#define SYS_TRNG_SEED_SIZE                              (IRNG_RANDOM_SEED_SIZE_BYTES)

 /**
 * \brief The number of memory blocks passed as entropy source. The size of the block is 16 bytes,
 *        as defined by IRNG_BLOCK_SIZE_BYTES.
 */
#define SYS_TRNG_MEMORY_BLOCKS                          (IRNG_MINIMUM_SRAM_PUF_BLOCKS + 4)

#if (SYS_TRNG_MEMORY_BLOCKS < IRNG_MINIMUM_SRAM_PUF_BLOCKS)
#error "The number of SYS_TRNG_MEMORY_BLOCKS must be equal or greater than IRNG_MINIMUM_SRAM_PUF_BLOCKS"
#endif

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SYS_TRNG error codes
 */
typedef enum {
        SYS_TRNG_ERROR_NONE                     = IID_SUCCESS,
        SYS_TRNG_ERROR_NOT_ALLOWED              = IID_NOT_ALLOWED,
        SYS_TRNG_ERROR_INVALID_PARAMETERS       = IID_INVALID_PARAMETERS,
        SYS_TRNG_ERROR_INVALID_SRAM_PUF_DATA    = IID_ERROR_SRAM_PUF_DATA,
        SYS_TRNG_ERROR_INSUFFICIENT_SRAM_BLOCKS = IID_ERROR_INSUFFICIENT_SRAM_BLOCKS,
        SYS_TRNG_ERROR_AES_TIMEOUT              = IID_ERROR_AES_TIMEOUT,
        SYS_TRNG_ERROR_AES_FAILED               = IID_ERROR_AES_FAILED,
} SYS_TRNG_ERROR;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Checks whether the TRNG module can generate a seed or not.
 *
 * \return if true the sys_trng_init() can be called, otherwise not.
 */
bool sys_trng_can_run(void);

/**
 * \brief Runs a software algorithm which generates a random seed. Random memory data are used
 *        to feed the software algorithm.
 *
 * \note  It must be ensured that random memory data (it is assumed that a RAM cell contains
 *        random values when it powers-up) are fed to the software algorithm, otherwise the
 *        sys_trng_init() will return an error code. The sys_trng_can_run() assures that the
 *        software algorithm will be fed with random data.
 *
 * \return error code. SYS_TRNG_ERROR_NONE if the seed was successfully generated, otherwise an error code.
 */
SYS_TRNG_ERROR sys_trng_init(void);


#endif /* dg_configUSE_SYS_TRNG */


#endif /* SYS_TRNG_V2_H_ */

/**
\}
\}
*/

