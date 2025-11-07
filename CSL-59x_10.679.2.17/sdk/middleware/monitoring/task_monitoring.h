/**
 * \addtogroup UTILITIES
 * \{
 * \addtogroup UTI_TASK_MONITORING
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file task_monitoring.h
 *
 * @brief Monitoring Task functions
 *
 * Copyright (C) 2017-2018 Renesas Electronics Corporation and/or its affiliates.
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
 *****************************************************************************************
 */

#ifndef TASK_MONITORING_H
#define TASK_MONITORING_H

#if (dg_configENABLE_TASK_MONITORING == 1)
/*Maximum number of monitored tasks*/
#define MAX_NUMBER_OF_MONITORED_TASKS 5

/**
 * \brief Register a task to be monitored.
 *
 * \param [in] id the id to be used for monitoring
 *
 */
void tm_register_monitor_task(uint16_t id);

/**
 * \brief Unregister a task from the monitoring function.
 *
 * \param [in] id the id of the monitored task
 *
 */
void tm_unregister_monitor_task(uint16_t id);

/**
 * \brief Print the status of registered a tasks.
 *
 */
void tm_print_registered_tasks(void);

/**
 * \brief Print the status of a all the tasks.
 *
 */
void tm_print_tasks_status(void);

#endif /*dg_configENABLE_TASK_MONITORING*/

#endif /*TASK_MONITORING_H*/

/**
 * \}
 * \}
 */
