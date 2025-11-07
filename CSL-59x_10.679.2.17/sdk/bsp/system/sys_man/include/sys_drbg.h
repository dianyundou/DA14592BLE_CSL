/**
 ****************************************************************************************
 *
 * @file sys_drbg.h
 *
 * @brief sys_drbg header file.
 *
 * Copyright (C) 2021-2024 Renesas Electronics Corporation and/or its affiliates.
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
 * \addtogroup SYS_DRBG Deterministic Random Bit Generator
 *
 * \brief Deterministic random bit generator
 *
 * \{
 *
 */

#ifndef SYS_DRBG_H_
#define SYS_DRBG_H_


#if dg_configUSE_SYS_DRBG

#include "sdk_defs.h"

/*
 * TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief sys drbg errors
 */
typedef enum {
        SYS_DRBG_ERROR_NONE             =  0,   /**< no error */
        SYS_DRBG_ERROR_BUFFER_EXHAUSTED = -1,   /**< buffer with random numbers has been exhausted */
} SYS_DRBG_ERROR;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

#if (dg_configUSE_SYS_TRNG == 0)
/**
 * \brief Checks whether the DRBG module can use a random value from RAM as a seed or not.
 *
 * \return if true the sys_drbg_srand() can be called, otherwise not.
 */
bool sys_drbg_can_run(void);
#endif /* dg_configUSE_SYS_TRNG */

/**
 * \brief Set the seed for the random number generator function.
 */
void sys_drbg_srand(void);

#if defined(OS_PRESENT)
/**
 * \brief DRBG mutex and task creation.
 *
 * \note This function shall be called after the scheduler has been started.
 */
void sys_drbg_create_os_objects(void);
#endif /* OS_PRESENT */

/**
 * \brief Initializes the DRBG data structure. \sa sys_drbg_t
 */
void sys_drbg_init(void);

/**
 * \brief Reads a random number from the buffer which holds the random numbers.
 *
 * \note  When a random number is read from the buffer it is assumed to be consumed. The next time
 *        the sys_drbg_read_rand() will be called a new random number will be read from the buffer.
 *
 * \param[out] rand_number      The random number to be returned
 *
 * \return Error code. If SYS_DRBG_ERROR_NONE the random number was successfully read from the buffer.
 *                     If SYS_DRBG_ERROR_BUFFER_EXHAUSTED the random number was read directly from
 *                     random number generator and sys_drbg_task changed to ready state.
 *                     Note: For baremetal project, application should call sys_drbg_update to fill
 *                     the buffer.
 */
SYS_DRBG_ERROR sys_drbg_read_rand(uint32_t *rand_number);

#if !defined(OS_PRESENT)
/**
 * \brief Updates the DRBG data structure. \sa sys_drbg_t
 */
void sys_drbg_update(void);
#endif /* OS_PRESENT */

/**
 * \brief Reads the current index value of the buffer.
 *
 * \return The current index value of the buffer
 */
uint32_t sys_drbg_read_index(void);

/**
 * \brief Reads the threshold level value.
 *
 * \note  If the buffer's current index is equal to the threshold level value or greater
 *        than the threshold level value, then a request for buffer update will be issued.
 *
 * \return The threshold level value
 */
uint32_t sys_drbg_read_threshold(void);

/**
 * \brief Reads the request value.
 *
 * \return Either 0 or 1. If the request value is 1, there is a pending request for buffer update.
 */
uint8_t sys_drbg_read_request(void);

#endif /* dg_configUSE_SYS_DRBG */


#endif /* SYS_DRBG_H_ */

/**
\}
\}
*/
