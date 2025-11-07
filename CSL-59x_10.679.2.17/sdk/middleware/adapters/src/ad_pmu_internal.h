/**
 ****************************************************************************************
 *
 * @file ad_pmu_internal.h
 *
 * @brief PMU internal adapter API - Should be excluded from documentation
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef AD_PMU_INTERNAL_H_
#define AD_PMU_INTERNAL_H_

#include "sdk_defs.h"
#if dg_configPMU_ADAPTER

/**
 * Let PMU adapter configure rails before sleep and upon wakeup.
 */
#define PMU_CONFIG_RAILS_SLP_WKUP       1
#if PMU_CONFIG_RAILS_SLP_WKUP
#define AD_PMU_PREPARE_FOR_SLEEP() ad_pmu_prepare_for_sleep()
#define AD_PMU_RESTORE_FOR_WAKEUP() ad_pmu_restore_for_wake_up()
#else
#define AD_PMU_PREPARE_FOR_SLEEP()
#define AD_PMU_RESTORE_FOR_WAKEUP()
#endif

/**
 * \brief Prepare for sleep.
 *
 * Hook for PMU actions before going to sleep.
 */
void ad_pmu_prepare_for_sleep(void);

/**
 * \brief Restore for system wake-up.
 *
 * Hook for PMU actions after waking up.
 */
void ad_pmu_restore_for_wake_up(void);

/**
 * \brief Requests the system to force the voltage level of 1V2 rail to 1.2V
 *
 * The voltage level of 1V2 rail raises to 1.2V. The last configuration of 1V2 is not lost.
 * It will be restored when ad_pmu_1v2_force_max_voltage_release() is called equal times
 * the number of ad_pmu_1v2_force_max_voltage_request() calls.
 *
 * \warning The PM Adapter API user MUST ensure that any request is matched by the respective release.
 *          Otherwise the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_request(void);

/**
 * \brief Restore the 1V2 rail configuration. It terminates a matching request.
 *
 * The 1V2 rail is restored to the configuration that was applied before calling
 * ad_pmu_1v2_force_max_voltage_request().
 *
 * \warning This function MUST be called always to terminate a matching ad_pmu_1v2_force_max_voltage_request().
 *          If called alone the system will reach an error-state!
 */
void ad_pmu_1v2_force_max_voltage_release(void);

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_INTERNAL_H_ */
