/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_PMU Power Manager Driver
\{
\brief Power Manager
*/

/**
****************************************************************************************
*
* @file hw_pmu_da1459x.h
*
* @brief Power Manager header file for DA1459x.
*
* Copyright (C) 2019-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef HW_PMU_DA1459x_H_
#define HW_PMU_DA1459x_H_


#if dg_configUSE_HW_PMU


#include "sdk_defs.h"

/**
 * \brief PMU API Error Codes
 *
 */
typedef enum {
        HW_PMU_ERROR_NOERROR                    = 0,        //!< No Error
        HW_PMU_ERROR_INVALID_ARGS               = 1,        //!< Invalid arguments
        HW_PMU_ERROR_NOT_ENOUGH_POWER           = 2,        //!< Current LDO config cannot supply enough power for this config
        HW_PMU_ERROR_XTAL32M_DBLR_ON            = 3,        //!< Doubler is on
        HW_PMU_ERROR_XTAL32M_ON                 = 4,        //!< XTAL32M is on
        HW_PMU_ERROR_RC32M_ON                   = 5,        //!< RC32M is on
        HW_PMU_ERROR_RCX_ON                     = 6,        //!< RCX is on
        HW_PMU_ERROR_RCX_LP                     = 7,        //!< RCX set as LP clock
        HW_PMU_ERROR_XTAL32K_ON                 = 8,        //!< XTAL32K is on
        HW_PMU_ERROR_XTAL32K_LP                 = 9,        //!< XTAL32K set as LP clock
        HW_PMU_ERROR_FAST_WAKEUP_ON             = 10,       //!< Fast wakeup is on
        HW_PMU_ERROR_ACTION_NOT_POSSIBLE        = 11,       //!< Action not possible to execute
        HW_PMU_ERROR_OTHER_LOADS_DEPENDENCY     = 12,       //!< Other loads dependency
        HW_PMU_ERROR_BOD_IS_ACTIVE              = 13,       //!< BOD is active
        HW_PMU_ERROR_BOD_THRESHOLD              = 14,       //!< BOD threshold level
        HW_PMU_ERROR_SLEEP_LDO                  = 15,       //!< Sleep LDO configured with voltage below 0.9V
        HW_PMU_ERROR_EFLASH_OPS                 = 16        //!< eFLASH write or erase operation is ongoing
} HW_PMU_ERROR_CODE;

/**
 * \brief PMU API Source type
 *
 * This allows the user to select a high-efficiency and high-ripple source (DCDC)
 * or low-efficiency and low-ripple source (LDO) for a Power Rail.
 */
typedef enum {
        HW_PMU_SRC_TYPE_LDO_LOW_RIPPLE       = 0,   //!< Low ripple source (LDO)
        HW_PMU_SRC_TYPE_DCDC_HIGH_EFFICIENCY = 1,   //!< High efficiency (and ripple) source (DCDC)
        HW_PMU_SRC_TYPE_VBAT                 = 2    //!< Bypass mode for LDO_IO/LDO_IO_RET
} HW_PMU_SRC_TYPE;

/**
 * \brief Power rail state (enabled or disabled)
 *
 *  Depending on the context it either imply disabled / enabled in sleep state or
 *  active / wakeup state.
 */
typedef enum {
        POWER_RAIL_DISABLED = 0,        //!< The rail is disabled
        POWER_RAIL_ENABLED  = 1         //!< The rail is enabled
} HW_PMU_POWER_RAIL_STATE;

/**
 * \brief Voltage level options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_VOLTAGE_1V8 = 0      //!< 1.8V
} HW_PMU_1V8_VOLTAGE;

/**
 * \brief Maximum load current options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_1V8_MAX_LDO_LOAD_2       = 0,    //!< 2mA supplied by LDO_IO_RET
        HW_PMU_1V8_MAX_BYPASS_LOAD_2    = 1,    //!< 2mA supplied by VBAT
        HW_PMU_1V8_MAX_LDO_LOAD_20      = 2,    //!< 20mA supplied by LDO_IO
        HW_PMU_1V8_MAX_BYPASS_LOAD_20   = 3,    //!< 20mA supplied by VBAT
} HW_PMU_1V8_MAX_LOAD;

/**
 * \brief 1V8 power rail configuration
 *
 */
typedef struct {
        HW_PMU_1V8_VOLTAGE voltage;
        HW_PMU_1V8_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_1V8_RAIL_CONFIG;


/**
 * \brief Voltage level options for VDCDC rail applicable in active state
 *
 */
typedef enum {
        HW_PMU_VDCDC_VOLTAGE_1V10       = 0,    //!< 1.10V
        HW_PMU_VDCDC_VOLTAGE_1V15       = 1,    //!< 1.15V
        HW_PMU_VDCDC_VOLTAGE_1V20       = 2,    //!< 1.20V
        HW_PMU_VDCDC_VOLTAGE_1V25       = 3,    //!< 1.25V
        HW_PMU_VDCDC_VOLTAGE_1V30       = 4,    //!< 1.30V
        HW_PMU_VDCDC_VOLTAGE_1V35       = 5,    //!< 1.35V
        HW_PMU_VDCDC_VOLTAGE_1V40       = 6,    //!< 1.40V
        HW_PMU_VDCDC_VOLTAGE_1V45       = 7     //!< 1.45V
} HW_PMU_VDCDC_VOLTAGE;

/**
 * \brief Maximum load current options for the VDCDC power rail
 *
 */
typedef enum {
        HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300        = 0,    //!< 300uA
        HW_PMU_VDCDC_MAX_LDO_LOAD_1             = 1,    //!< 1mA
        HW_PMU_VDCDC_MAX_DCDC_LOAD_40           = 2,    //!< 40mA
        HW_PMU_VDCDC_MAX_LDO_LOAD_40            = 3     //!< 40mA
} HW_PMU_VDCDC_MAX_LOAD;

/**
 * \brief VDCDC power rail configuration
 *
 */
typedef struct {
        HW_PMU_VDCDC_VOLTAGE voltage;
        HW_PMU_VDCDC_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_VDCDC_RAIL_CONFIG;


/**
 * \brief Voltage level options for VDD rail
 *
 */
typedef enum {
        /* active state */
        HW_PMU_VDD_VOLTAGE_0V90         = 0,    //!< 0.90V during active state
        HW_PMU_VDD_VOLTAGE_0V95         = 1,    //!< 0.95V during active state
        HW_PMU_VDD_VOLTAGE_1V00         = 2,    //!< 1.00V during active state
        HW_PMU_VDD_VOLTAGE_1V05         = 3,    //!< 1.05V during active state
        HW_PMU_VDD_VOLTAGE_1V10         = 4,    //!< 1.10V during active state
        HW_PMU_VDD_VOLTAGE_1V15         = 5,    //!< 1.15V during active state
        HW_PMU_VDD_VOLTAGE_1V20         = 6,    //!< 1.20V during active state
        HW_PMU_VDD_VOLTAGE_1V25         = 7,    //!< 1.25V during active state

        /* sleep state
         * Only the last bit of the enumerated value is used
         * for programming the rail
         */

        HW_PMU_VDD_VOLTAGE_SLEEP_0V75   = 8,    //!< 0.75V during sleep state
        HW_PMU_VDD_VOLTAGE_SLEEP_0V90   = 9,    //!< 0.90V during sleep state

        HW_PMU_VDD_VOLTAGE_INVALID              //!< Invalid voltage
} HW_PMU_VDD_VOLTAGE;

/**
 * \brief Maximum load current options for the 1V8 power rail
 *
 */
typedef enum {
        HW_PMU_VDD_MAX_LOAD_20          = 0,    //!< 20mA
        HW_PMU_VDD_MAX_LOAD_0_400       = 1     //!< 400uA
} HW_PMU_VDD_MAX_LOAD;

/**
 * \brief VDD power rail configuration
 *
 */
typedef struct {
        HW_PMU_VDD_VOLTAGE voltage;
        HW_PMU_VDD_MAX_LOAD current;
        HW_PMU_SRC_TYPE src_type;
} HW_PMU_VDD_RAIL_CONFIG;


/**
 * \brief Set 1V8 rail wakeup / active configuration
 *
 * This function sets the 1V8 rail configuration during the wakeup / active state.
 * This is effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_IO or bypass mode in active mode, disable other power sources.
 * - Low current -  Enable LDO_IO_RET, disable other power sources.
 * The rail cannot be enabled if the power sources that supply it are off. In this case
 * HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                         Source
 *  HW_PMU_1V8_MAX_LDO_LOAD_20          LDO_IO
 *  HW_PMU_1V8_MAX_LDO_LOAD_2           LDO_IO_RET
 *  HW_PMU_1V8_MAX_BYPASS_LOAD_20       VBAT (Bypass mode)
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8 rail in wakeup / active state
 *
 * This function disables all the 1V8 power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onwakeup_disable(void);

/**
 * \brief Enable 1V8 rail in sleep state
 *
 * This function enables the 1V8 power sources used during the sleep state.
 * Depending on the input parameter the power source could be either LDO_IO_RET or VBAT
 * (bypass mode in sleep mode).
 *
 * If none of the power sources is available HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                         Source
 *  HW_PMU_1V8_MAX_LDO_LOAD_2           LDO_IO_RET
 *  HW_PMU_1V8_MAX_BYPASS_LOAD_2        VBAT (Bypass mode)
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_enable(HW_PMU_1V8_MAX_LOAD max_load);

/**
 * \brief Disable 1V8 rail in sleep state
 *
 * This function disables all the 1V8 power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_1v8_onsleep_disable(void);

/**
 * \brief Get the 1V8 rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_active_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8 rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onwakeup_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Get the 1V8 rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail 1V8 has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_1v8_onsleep_config(HW_PMU_1V8_RAIL_CONFIG *rail_config);

/**
 * \brief Set the voltage level of VDCDC rail
 *
 * This function sets the voltage level of the VDCDC rail during active / wakeup state.
 * The voltage level setting is common no matter the power source (DCDC or LDO_LOW / LDO_LOW_RET).
 *
 * The input voltage should be set at least 200mV higher than that of VDD. Otherwise an error
 * code is returned.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_VDCDC_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_vdcdc_set_voltage(HW_PMU_VDCDC_VOLTAGE voltage);

/**
 * \brief Set VDCDC rail wakeup / active configuration
 *
 * This function sets the VDCDC rail configuration during the wakeup / active state.
 * This is effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_LOW or DCDC
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                         Source
 *  HW_PMU_VDCDC_MAX_DCDC_LOAD_40       DCDC
 *  HW_PMU_VDCDC_MAX_LDO_LOAD_40        LDO_LOW
 */
HW_PMU_ERROR_CODE hw_pmu_vdcdc_onwakeup_enable(HW_PMU_VDCDC_MAX_LOAD max_load);

/**
 * \brief Disable VDCDC rail in wakeup / active state
 *
 * This function disables all the VDCDC power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vdcdc_onwakeup_disable(void);

/**
 * \brief Enable the VDCDC rail in sleep state
 *
 * This function enables the VDCDC rail during the sleep state.
 * Depending on the input parameter the power source could be either LDO_IO_LOW_RET or DCDC
 *
 * If none of the power sources is available HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                         Source
 *  HW_PMU_VDCDC_MAX_LDO_LOAD_1         LDO_LOW_RET
 *  HW_PMU_VDCDC_MAX_DCDC_LOAD_0_300    DCDC
 */
HW_PMU_ERROR_CODE hw_pmu_vdcdc_onsleep_enable(HW_PMU_VDCDC_MAX_LOAD max_load);

/**
 * \brief Disable VDCDC rail in sleep state
 *
 * This function disables all the VDCDC power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vdcdc_onsleep_disable(void);

/**
 * \brief Get the VDCDC rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDCDC has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_active_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VDCDC rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDCDC has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_onwakeup_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VDCDC rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDCDC has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdcdc_onsleep_config(HW_PMU_VDCDC_RAIL_CONFIG *rail_config);

/**
 * \brief Set the voltage level of VDD rail
 *
 * This function sets the voltage level of the VDD rail during active / wakeup or sleep state.
 *
 * The input voltage should be set at least 200mV lower than that of VDCDC. Otherwise an error
 * code is returned.
 *
 * \param[in] voltage The voltage of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been configured properly, or an
 *                 error code otherwise.
 *
 *  Valid input parameters \sa HW_PMU_VDCDC_VOLTAGE
 */
HW_PMU_ERROR_CODE hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE voltage);

/**
 * \brief Set VDD rail wakeup / active configuration
 *
 * This function sets the VDD rail configuration during the wakeup / active state.
 * This is effective immediately.
 * Depending on the input parameter, the appropriate power source will be selected:
 * - High current - Enable LDO_CORE, disable other power sources.
 * - Low current  - LDO_CORE_RET, dsiable other power sources.
 *
 * The rail cannot be enabled if the power sources that supply it are off.
 * In this case HW_PMU_ERROR_NOT_ENOUGH_POWER error code will
 * be returned.
 *
 * \param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an
 *         error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                 Source
 *  HW_PMU_VDD_MAX_LOAD_20      LDO_CORE
 *  HW_PMU_VDD_MAX_LOAD_0_400   LDO_CORE_RET
 */
HW_PMU_ERROR_CODE hw_pmu_vdd_onwakeup_enable(HW_PMU_VDD_MAX_LOAD max_load);

/**
 * \brief Disable VDD rail in wakeup / active state
 *
 * This function disables all the VDD power sources used during the wakeup / active state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vdd_onwakeup_disable(void);

/**
 * \brief Enable VDD rail in sleep state
 *
 * This function enables the VDD power sources used during the sleep state.
 * The only power source is LDO_IO_RET.
 *
 * If it is not available HW_PMU_ERROR_NOT_ENOUGH_POWER error code will be returned.
 *
 * param[in] max_load The maximum current that can be supplied to the loads of the rail
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been enabled, or an error code otherwise.
 *
 *  Valid input parameters:
 *  max load mA                         Source
 *  HW_PMU_VDD_MAX_LOAD_0_400           LDO_CORE_RET
 */
HW_PMU_ERROR_CODE hw_pmu_vdd_onsleep_enable(HW_PMU_VDD_MAX_LOAD max_load);

/**
 * \brief Disable VDD rail in sleep state
 *
 * This function disables all the VDD power sources used during the sleep state.
 *
 * \return Returns HW_PMU_ERROR_NOERROR if the rail has been disabled, or an error code otherwise.
 */
HW_PMU_ERROR_CODE hw_pmu_vdd_onsleep_disable(void);

/**
 * \brief Get the VDD rail active state configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDD has been configured properly
 *         to work in active state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_active_config(HW_PMU_VDD_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VDD rail wakeup configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDD has been configured properly
 *         to work in wakeup state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_onwakeup_config(HW_PMU_VDD_RAIL_CONFIG *rail_config);

/**
 * \brief Get the VDD rail sleep configuration
 *
 * \param[out] rail_config The rail configuration
 *
 * \return POWER_RAIL_ENABLED if the rail VDD has been configured properly
 *         to work in sleep state, or POWER_RAIL_DISABLED otherwise.
 */
HW_PMU_POWER_RAIL_STATE hw_pmu_get_vdd_onsleep_config(HW_PMU_VDD_RAIL_CONFIG *rail_config);


#endif /* dg_configUSE_HW_PMU */


#endif /* HW_PMU_DA1459x_H_ */

/**
\}
\}
*/
