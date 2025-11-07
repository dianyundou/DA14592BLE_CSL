/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_BOD BOD driver
\{
\brief DA1459x BOD LLD
*/

/**
****************************************************************************************
*
* @file hw_bod_da1459x.h
*
* @brief BOD LLD header file for DA1459x.
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

#ifndef HW_BOD_DA1459x_H_
#define HW_BOD_DA1459x_H_


#include "sdk_defs.h"

/**
 * \brief The BOD channel name
 */
typedef enum {
        HW_BOD_CHANNEL_1V8         = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDDIO_COMP),   /**< VDDIO channel */
        HW_BOD_CHANNEL_VDCDC       = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDCDC_COMP),   /**< VDCDC channel */
        HW_BOD_CHANNEL_VDD         = REG_POS(CRG_TOP, BOD_CTRL_REG, BOD_DIS_VDD_COMP)      /**< VDD channel */
} HW_BOD_CHANNEL;

#if dg_configUSE_BOD

typedef enum {
        HW_BOD_VDD_LEVEL_SLEEP_0V70        = 700,
        HW_BOD_VDD_LEVEL_ACTIVE_0V78       = 780,
        HW_BOD_VDD_LEVEL_ACTIVE_1V05       = 1050,
        HW_BOD_VDD_LEVEL_UNDEF             = UINT16_MAX,
} HW_BOD_VDD_LVL;

/**
 * \brief Activate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_activate_channel(HW_BOD_CHANNEL channel)
{
        switch (channel) {
        case HW_BOD_CHANNEL_1V8:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDDIO_MASK);
                while (REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDIO_MASK_SYNC_RD));
                break;
        case HW_BOD_CHANNEL_VDCDC:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDCDC_MASK);
                while (REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDCDC_MASK_SYNC_RD));
                break;
        case HW_BOD_CHANNEL_VDD:
                REG_CLR_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDD_MASK);
                while (REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_MASK_SYNC_RD));
                break;
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Deactivate BOD for a channel.
 *
 * \param[in] channel BOD channel
 *
 */
__STATIC_FORCEINLINE void hw_bod_deactivate_channel(HW_BOD_CHANNEL channel)
{
        switch (channel) {
        case HW_BOD_CHANNEL_1V8:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDDIO_MASK);
                while (!REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDIO_MASK_SYNC_RD));
                break;
        case HW_BOD_CHANNEL_VDCDC:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDCDC_MASK);
                while (!REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDCDC_MASK_SYNC_RD));
                break;
        case HW_BOD_CHANNEL_VDD:
                REG_SET_BIT(CRG_TOP, BOD_CTRL_REG, BOD_VDD_MASK);
                while (!REG_GETF(CRG_TOP, STARTUP_STATUS_REG, BOD_VDDD_MASK_SYNC_RD));
                break;
        default:
                /* Invalid channel, we should not reach here. */
                ASSERT_WARNING(0);
        }
}

/**
 * \brief Set BOD channel voltage level.
 *
 *  Only VDD BOD channel is programmable with the following levels:
 *  -  700 Applicable in sleep mode
 *  -  780 When VDD = 900mV
 *  - 1050 When VDD = 1200mV
 *
 * \param[in] channel BOD channel
 * \param[in] level voltage level in mV
 *
 */
void hw_bod_set_channel_voltage_level(HW_BOD_CHANNEL channel, HW_BOD_VDD_LVL level);

/**
 * \brief Get BOD channel voltage level.
 *
 * \param[in] channel BOD channel
 *
 * \return voltage level in mV
 *
 */
HW_BOD_VDD_LVL hw_bod_get_channel_voltage_level(HW_BOD_CHANNEL channel);

/**
 * \brief Configure BOD.
 *
 */
void hw_bod_configure(void);

#endif /* dg_configUSE_BOD */

/**
 * \brief Read BOD status for a channel.
 *
 * \param[in] channel BOD channel
 *
 * \return BOD status of the specific channel. If true, BOD channel is disabled.
 *
 */
bool hw_bod_get_status(HW_BOD_CHANNEL channel);

/**
 * \brief Deactivate BOD for all channels.
 *
 */
void hw_bod_deactivate(void);


#endif /* HW_BOD_DA1459x_H_ */

/**
\}
\}
*/
