/**
 * \addtogroup UTILITIES
 * \{
 * \addtogroup CLI
 *
 * \brief Command Line Interface
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file cli.h
 *
 * @brief Declarations for CLI service
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef CLI_H_
#define CLI_H_

#if dg_configUSE_CLI

#include <osal.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief CLI task priority
 */
#ifndef CLI_TASK_PRIORITY
#define CLI_TASK_PRIORITY       (OS_TASK_PRIORITY_NORMAL)
#endif

/**
 * CLI instance
 */
typedef void* cli_t;

/**
 * Command handler
 *
 * \p argv[0] is always a command name
 *
 * \param [in] argc       number of arguments
 * \param [in] argv       array of arguments
 * \param [in] user_data  user data passed from command
 */
typedef void (* cli_handler_t) (int argc, const char *argv[], void *user_data);

/**
 * Command definition
 */
typedef struct {
        const char *name;       /**< command name (i.e. \p argv[0]) */
        cli_handler_t handler;  /**< command handler */
        void *user_data;        /**< user data passed to command handler */
} cli_command_t;

#if !dg_configUSE_CLI_STUBS

/**
 * Initialize CLI
 *
 * This function initializes CLI internal structures and shall be called before CLI is used and
 * after \p console_init().
 *
 */
void cli_init(void);

/**
 * Register command handlers for current task
 *
 * This functions registers command handlers to be evaluated for matching CLI input.
 *
 * Once full line of text is entered, CLI task notifies registered task using \p notif_mask and task
 * shall then call cli_handle_notified() to process entry.
 *
 * \warning
 * CLI stores only pointer to \p cmd_handler and application should guarantee that this pointer
 * is valid for entire adapter lifetime.
 *
 * \note
 * Only one task can register handlers in current implementation.
 *
 * \param [in] notif_mask       bit mask for task notification
 * \param [in] cmd_handler      predefined commands handlers
 * \param [in] def_handler      default command handler
 *
 * \return CLI instance
 *
 * \sa cli_handle_notified
 *
 */
cli_t cli_register(uint32_t notif_mask, const cli_command_t cmd_handler[],
                                                                        cli_handler_t def_handler);

/**
 * Handle notification from CLI
 *
 * This function shall be called when application task is notified from CLI task to process pending
 * entry.
 *
 * Entry received from adapter will be split into tokens internally and matched against handlers
 * provided on registration.
 *
 * \param [in] cli              CLI instance
 *
 * \sa cli_register
 *
 */
void cli_handle_notified(cli_t cli);

#else /* !dg_configUSE_CLI_STUBS */

__STATIC_INLINE void cli_init(void)
{

}

__STATIC_INLINE cli_t cli_register(uint32_t notif_mask, const cli_command_t cmd_handler[],
                                                                cli_handler_t def_handler)
{
        return NULL;
}

__STATIC_INLINE void cli_handle_notified(cli_t cli)
{

}

#endif /* !dg_configUSE_CLI_STUBS */

#ifdef __cplusplus
}
#endif

#endif /* dg_configUSE_CLI */

#endif /* CLI_H_ */

/**
 * \}
 * \}
 */
