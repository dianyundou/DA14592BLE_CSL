/**
 ****************************************************************************************
 *
 * @file sound_maker.h
 *
 * @brief Sound maker header file
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef SOUND_MAKER_H_
#define SOUND_MAKER_H_

/**
 * \brief Sound maker volume
 */
typedef enum {
        SOUND_MAKER_VOLUME_DEFAULT,
        SOUND_MAKER_VOLUME_LOW,
        SOUND_MAKER_VOLUME_MEDIUM,
        SOUND_MAKER_VOLUME_HIGH
} SOUND_MAKER_VOLUME;

/**
 * \brief Handle of sound maker instance
 */
typedef void *sound_maker_handle_t;

/**
 * \brief Initialize sound maker
 *
 * This function is called to initialize a sound maker instance.
 *
 * \return handle of the sound maker instance if it is created successfully, otherwise NULL
 */
sound_maker_handle_t sound_maker_init(void);

/**
 * \brief De-initialize sound maker
 *
 * This function is called to reset and free resources related to a sound maker instance.
 *
 * \param [in] sound_maker handle of the sound maker instance
 */
void sound_maker_deinit(sound_maker_handle_t sound_maker);

/**
 * \brief Enable sound maker
 *
 * This function is called to enable sound maker.
 *
 * \param [in] sound_maker handle of the sound maker instance
 * \param [in] vol sound maker volume
 */
void sound_maker_enable(sound_maker_handle_t sound_maker, SOUND_MAKER_VOLUME vol);

/**
 * \brief Disable sound maker
 *
 * This function is called to disable sound maker.
 *
 * \param [in] sound_maker handle of the sound maker instance
 */
void sound_maker_disable(sound_maker_handle_t sound_maker);

#endif /* SOUND_MAKER_H_ */
