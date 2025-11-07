/**
 ****************************************************************************************
 *
 * @file afmn_sound_maker.h
 *
 * @brief Apple FMN sound maker header file
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

#ifndef AFMN_SOUND_MAKER_H_
#define AFMN_SOUND_MAKER_H_

/**
 * \brief Initialize Apple FMN sound maker
 *
 * This function is called by Apple FindMy Network ADK to initialize sound maker.
 */
void afmn_sound_maker_init(void);

/**
 * \brief De-initialize Apple FMN sound maker
 *
 * This function is called by Apple FindMy Network ADK to reset and free resources related to
 * sound maker.
 */
void afmn_sound_maker_deinit(void);

/**
 * \brief Enable Apple FMN sound maker
 *
 * This function is called by Apple FindMy Network ADK to enable sound maker.
 */
void afmn_sound_maker_enable(void);

/**
 * \brief Disable Apple FMN sound maker
 *
 * This function is called by Apple FindMy Network ADK to disable sound maker.
 */
void afmn_sound_maker_disable(void);

#endif /* AFMN_SOUND_MAKER_H_ */
