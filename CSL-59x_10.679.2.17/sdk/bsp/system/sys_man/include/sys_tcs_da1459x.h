/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TCS_HANDLER TCS Handler
 * \brief TCS Handler
 * \{
 */

/**
****************************************************************************************
*
* @file sys_tcs_da1459x.h
*
* @brief TCS Handler header file.
*
* Copyright (C) 2020-2023 Renesas Electronics Corporation and/or its affiliates.
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
#if (dg_configUSE_SYS_TCS == 1)

#ifndef SYS_TCS_DA1459X_H_
#define SYS_TCS_DA1459X_H_

#include "sdk_defs.h"

#define TCS_DATA_SIZE           512   /**< max number of entries in words (4 bytes). */
#define MAX_SUPPORTED_TCS_GID   128   /**< max supported GID. */
#define GID_EMPTY 0x1FF

/**
 * \enum SYS_TCS_TYPE
 * \brief custom TCS value type.
 *
 */
typedef enum {
        SYS_TCS_TYPE_TRIM_VAL = 0,      /**< trimmed value */
        SYS_TCS_TYPE_REG_PAIR = 1,      /**< register pair value */
} SYS_TCS_TYPE;

/**
 * \brief the configured group ids.
 *
 */
typedef enum {
        SYS_TCS_GROUP_PD_SYS = 0x01,                    /**< PD_SYS group id */
        SYS_TCS_GROUP_PD_COMM = 0x02,                   /**< PD_COMM group id */
        SYS_TCS_GROUP_PD_MEM = 0x03,                    /**< PD_MEM group id */
        SYS_TCS_GROUP_PD_TMR = 0x04,                    /**< PD_TMR group id */
        SYS_TCS_GROUP_PD_PER = 0x05,                    /**< PD_PER group id */
        SYS_TCS_GROUP_PD_RAD = 0x06,                    /**< PD_RAD group id */
        SYS_TCS_GROUP_PD_SYNTH = 0x07,                  /**< PD_SYNTH group id */
        SYS_TCS_GROUP_PD_AUD = 0x08,                    /**< PD_ADU group id */
        SYS_TCS_GROUP_BD_ADDR = 0x0B,                   /**< BD_ADDR group id */
        SYS_TCS_GROUP_PROD_INFO = 0x0C,                 /**< PROD_INFO group id */
        SYS_TCS_GROUP_CHIP_ID = 0x0D,                   /**< CHIP_ID group id */
        SYS_TCS_GROUP_PROD_WAFER = 0x0E,                /**< PROD_WAFER group id */
        SYS_TCS_GROUP_TESTPROGRAM_VERSION = 0x0F,       /**< TESTPROGRAM_VERSION  group id */
        SYS_TCS_GROUP_SD_ADC_SINGLE_MODE = 0x10,        /**< SD_ADC_SINGLE group id */
        SYS_TCS_GROUP_SD_ADC_DIFF_MODE = 0x11,          /**< SD_ADC_DIFF group id */
        SYS_TCS_GROUP_GP_ADC_SINGLE_MODE = 0x12,        /**< GP_ADC_SINGLE group id */
        SYS_TCS_GROUP_GP_ADC_DIFF_MODE = 0x13,          /**< GP_ADC_DIFF group id */
        SYS_TCS_GROUP_TEMP_SENS_25C = 0x14,             /**< TEMP_SENS_25C group id */
        SYS_TCS_GROUP_PD_RAD_CCOEFF_LP = 0x15,          /**< PD_RAD_CCOEFF_LP group id */
        SYS_TCS_GROUP_PD_RAD_CCOEFF_HP = 0x16,          /**< PD_RAD_CCOEFF_HP group id */
        SYS_TCS_GROUP_PD_RAD_MODE_LP = 0x17,            /**< PD_RAD_MODE_LP group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_LP = 0x18,          /**< PD_SYNTH_MODE_LP group id */
        SYS_TCS_GROUP_PD_RAD_MODE_HP = 0x19,            /**< PD_RAD_MODE_HP group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_HP = 0x1A,          /**< PD_SYNTH_MODE_HP group id */
        SYS_TCS_GROUP_PD_RAD_MODE_LP_0DBM = 0x1B,       /**< PD_RAD_MODE_LP_0DBM group id */
        SYS_TCS_GROUP_PD_RAD_MODE_LP_6DBM = 0x1C,       /**< PD_RAD_MODE_LP_6DBM group id */
        SYS_TCS_GROUP_PD_RAD_MODE_HP_0DBM = 0x1D,       /**< PD_RAD_MODE_HP_0DBM group id */
        SYS_TCS_GROUP_PD_RAD_MODE_HP_6DBM = 0x1E,       /**< PD_RAD_MODE_HP_6DBM group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_LP_105C = 0x1F,     /**< PD_SYNTH_MODE_LP_105C group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_LP_N40C = 0x20,     /**< PD_SYNTH_MODE_LP_N40C group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_HP_105C = 0x21,     /**< PD_SYNTH_MODE_HP_105C group id */
        SYS_TCS_GROUP_PD_SYNTH_MODE_HP_N40C = 0x22,     /**< PD_SYNTH_MODE_HP_N40C group id */
        SYS_TCS_GROUP_TEMP_SENS_105C = 0x23,            /**< TEMP_SENS_105C group id */
        SYS_TCS_GROUP_TEMP_SENS_N40C = 0x24,            /**< TEMP_SENS_N40C group id */
        SYS_TCS_GROUP_MAX = MAX_SUPPORTED_TCS_GID       /**< Maximum supported group id */
} SYS_TCS_GID;

/**
 * \struct sys_tcs_attr_t
 * \brief attributes per custom value group id
 *
 */
typedef struct {
        uint16_t value_type : 1;        /**< TCS entry type */
        uint16_t start : 9;             /**< TCS entry start position  */
        uint16_t size : 6;              /**< TCS entry type size in words */
} sys_tcs_attr_t;

/**
 * \brief TCS custom trim values callback
 *
 * \param [in] values_group the TCS group id custom trim values belong to
 * \param [in] user_data user specific data
 * \param [in] values custom trim values
 * \param [in] size the number of the custom trim values
 *
 */
typedef void (*sys_tcs_custom_values_cb)( SYS_TCS_GID values_group , void *user_data, const uint32_t *values, uint8_t size);

/**
 * \brief retrieve the TCS values from CS located in OTP or flash and then store
 * TCS register pair address, value  and/or custom value custom_trim_value in the Global TCS array
 *
 */
void sys_tcs_get_trim_values_from_cs(void);

/**
 * \brief get the number of entries stored in tcs_data
 *
 * \return the number of stored TCS entries. Each entry is 32bit
 */
uint32_t sys_tcs_get_tcs_data_size(void);

/**
 * \brief check if the register addresses included in reg_address are configured in CS
 * \param [in] reg_address pointer to array containing the register addresses
 * \param [in] num number of register addresses
 * \param [out] trimmed_reg pointer to array containing information whether the corresponding
 * register is included in CS or not
 *
 * \return true if all register addresses are included in CS
 *
 * \warning register values are first searched in INFO block, then in MAIN eflash block ,if CS exists
 * there, otherwise in CS block located in QSPI. trimmed_reg should be initialized to false otherwise
 * will report false positive that register address found.
 */
bool sys_tcs_reg_pairs_in_cs(const uint32_t* reg_address, uint8_t num, bool *trimmed_reg);

#endif /* SYS_TCS_DA1459X_H_ */
#endif /* (dg_configUSE_SYS_TCS == 1) */
/**
\}
\}
*/
