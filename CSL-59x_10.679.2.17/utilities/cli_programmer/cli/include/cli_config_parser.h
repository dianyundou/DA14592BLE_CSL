/**
 ****************************************************************************************
 *
 * @file cli_config_parser.h
 *
 * @brief Parser for CLI programmer configuration - API.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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
#ifndef CLI_CONFIG_PARSER_H
#define CLI_CONFIG_PARSER_H

#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>
#include "cli_common.h"

#define DEFAULT_CLI_CONFIG_FILE_NAME "cli_programmer.ini"

#ifndef _WIN32
#define MAX_CLI_CONFIG_FILE_PATHNAME_LEN PATH_MAX
#else
#include <windef.h>
#define MAX_CLI_CONFIG_FILE_PATHNAME_LEN MAX_PATH
#endif

/**
 * \brief Save CLI programmer configuration to .ini file.
 *
 * \param [in] file_path        path to .ini file
 * \param [in] opts             CLI programmer configuration instance
 *
 * \return true if file was saved properly, false otherwise
 *
 */
bool cli_config_save_to_ini_file(const char *file_path, const struct cli_options *opts);

/**
 * \brief Load CLI programmer configuration from .ini file.
 *
 * \param [in] file_path        path to .ini file
 * \param [in] opts             CLI programmer configuration instance
 *
 * \return true if file was loaded properly, false otherwise
 *
 */
bool cli_config_load_from_ini_file(const char *file_path, struct cli_options *opts);

/**
 * \brief Get default path to CLI programmer configuration file.
 *
 * Default path to configuration file could be: current directory, default user directory or
 * CLI programmer runtime directory - in this order. Function check that file is exist. If file
 * doesnt't exist in any of this three location NULL is returned.
 *
 * \param [in, out] path_buf    allocated buffer for file path
 * \param [in] argv0            CLI programmer argv[0] argument
 *
 * \return default path to CLI programmer configuration file if exist, NULL otherwise
 *
 */
char *get_default_config_file_path(char *path_buf, const char *argv0);

/**
 * \brief Remove ., .., /// & symbolic links from paths to make them canonical
 *
 * Pathnames may become vaulnerabilities to our system, when passed as command-line arguments.
 * The canonicalize functions for linux and win32 provide a minimum resistance to this type of
 * attacks.
 *
 * \param [out] out_name      the canonicalized pathname
 * \param [in]  in_name       the pathname to canonicalize
 *
 * \return true if success, false otherwise
 *
 */
bool cli_config_canonicalize_file_name(char *out_name, const char *in_name);

#endif /* CLI_CONFIG_PARSER_H */
