/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup PMU_ADAPTER PMU Adapter
 *
 * \brief Power Management Unit adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_pmu.h
 *
 * @brief PMU adapter API
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

#ifndef AD_PMU_H_
#define AD_PMU_H_

#if dg_configPMU_ADAPTER

#include <hw_pmu.h>
#include <hw_sys.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Rail selection
 *
 */
typedef enum {
        PMU_RAIL_VDD,           //!< VDD   rail
        PMU_RAIL_VDCDC,         //!< VDCDC rail
        PMU_RAIL_1V8,           //!< 1V8   rail
} AD_PMU_RAIL;

/**
 * \brief Rail configuration
 *
 */
typedef struct {
        bool enabled_onwakeup;                                  //!< true if rail is enabled in wakeup / active state
        bool enabled_onsleep;                                   //!< true if rail is enabled in sleep state
        union {
                struct {
                        HW_PMU_VDD_VOLTAGE voltage_onwakeup;    //!< VDD rail voltage configuration in wakeup / active state
                        HW_PMU_VDD_VOLTAGE voltage_onsleep;     //!< VDD rail voltage configuration in sleep state
                        HW_PMU_VDD_MAX_LOAD current_onwakeup;   //!< VDD rail current configuration in wakeup / active state
                        HW_PMU_VDD_MAX_LOAD current_onsleep;    //!< VDD rail current configuration in sleep state
                } rail_vdd;                                     //!< VDD rail voltage and current configuration

                struct {
                        HW_PMU_VDCDC_VOLTAGE voltage_onwakeup;  //!< VDCDC rail voltage configuration in wakeup / active state
                        HW_PMU_VDCDC_MAX_LOAD current_onwakeup; //!< VDCDC rail current configuration in wakeup / active state
                        HW_PMU_VDCDC_MAX_LOAD current_onsleep;  //!< VDCDC rail current configuration in sleep state
                } rail_vdcdc;                                   //!< VDCDC rail voltage and current configuration

                struct {
                        HW_PMU_1V8_MAX_LOAD current_onwakeup;   //!< 1V8 rail current configuration in wakeup / active state
                        HW_PMU_1V8_MAX_LOAD current_onsleep;    //!< 1V8 rail current configuration in sleep state
                } rail_1v8;                                     //!< 1V8 rail current configuration

        };                                                      //!< rail voltage and current configuration
} ad_pmu_rail_config_t;


/**
 * \brief Configure a power rail
 *
 * \return 0 if the rail is configured, >0 otherwise
 *
 * \param[in] rail the rail to configure
 * \param[in] config a pointer to the structure holding the rail configuration
 */
int ad_pmu_configure_rail(AD_PMU_RAIL rail, const ad_pmu_rail_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* dg_configPMU_ADAPTER */


#endif /* AD_PMU_H_ */

/**
 * \}
 * \}
 */
