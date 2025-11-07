/**
 ****************************************************************************************
 *
 * @file ini_parser.h
 *
 * @brief Parser for .ini files - API.
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
#ifndef INI_PARSER_H
#define INI_PARSER_H

#include "queue.h"

/* Initialize ini queue */
#define INI_QUEUE_INIT(q) queue_init(q)

/**
 * Representation of one parameter
 */
typedef struct {
        /** Section */
        char *section;
        /** Key */
        char *key;
        /** Value */
        char *value;
} ini_conf_elem_t;

/**
 * \brief Load configuration from .ini file .
 *
 * Read file for configuration parameters. Each parameter is added
 * to queue.
 *
 * \param [in] file_path        path to .ini file
 * \param [in] ini_queue        initialized queue instance
 *
 * \return true if file was loaded properly, false otherwise
 *
 */
bool ini_queue_load_file(const char *file_path, queue_t *ini_queue);

/**
 * \brief Save configuration to .ini file.
 *
 * \param [in] file_path        path to .ini file
 * \param [in] ini_queue        queue instance with configuration parameters
 *
 * \return true if file was saved properly, false otherwise
 *
 */
bool ini_queue_save_file(const char *file_path, const char *hdr_comment, queue_t *ini_queue);

/**
 * \brief Add configuration parameter to parameter queue
 *
 * \param [in] ini_queue        queue instance with configuration parameters
 * \param [in] section          section name
 * \param [in] key              parameter key
 * \param [in] value            parameter value
 */
void ini_queue_add(queue_t *ini_queue, const char *section, const char *key, const char *value);

/**
 *  \brief Get parameter value for specified section and key
 *
 * \param [in] ini_queue        queue instance with configuration parameters
 * \param [in] section_name     section name
 * \param [in] key              parameter key
 *
 * \return value for specified section and key if exist, NULL otherwise
 */
const char *ini_queue_get_value(const queue_t *ini_queue, const char *section_name, const char *key);

/**
 *  \brief Take first parameter from parameters queue
 *
 * Returned parameter's key, value and section fields were dynamic allocated - they should be freed
 * after use.
 *
 * \param [in] ini_queue        queue instance with configuration parameters
 *
 * \return parameter instance with key, section and value fields set if queue not empty, parameter
 * instance with set fields to NULL otherwise
 */
ini_conf_elem_t ini_queue_pop(queue_t *ini_queue);

#endif /* INI_PARSER_H */
