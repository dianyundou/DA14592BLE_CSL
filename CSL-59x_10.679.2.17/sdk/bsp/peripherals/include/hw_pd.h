/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup PD Power Domain Driver
\{
\brief Power Domain Driver
*/

/**
****************************************************************************************
*
* @file hw_pd.h
*
* @brief Power Domain Driver header file.
*
* Copyright (C) 2015-2020 Renesas Electronics Corporation and/or its affiliates.
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
#ifndef HW_PD_H_
#define HW_PD_H_


#if dg_configUSE_HW_PD

#include "sdk_defs.h"

/**
 * \enum HW_PD
 * \brief Hardware power domains.
 *
 */
typedef enum {
        HW_PD_AON = 0,      /**< Aon power domain */
        HW_PD_SYS,          /**< System power domain */
        HW_PD_COM,          /**< Communication power domain */
        HW_PD_MEM,          /**< Memory power domain */
        HW_PD_TMR,          /**< Timers power domain */
        HW_PD_PER,          /**< Peripherals power domain */
        HW_PD_RAD,          /**< Radio power domain */
        HW_PD_SLP,          /**< Sleep power domain */
        HW_PD_AUD,          /**< Audio and voice power domain */
        HW_PD_MAX           /**< Power domain max*/
} HW_PD;

/**
 * \brief Power up the Peripherals Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_periph(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_UP)) == 0);
}

__STATIC_FORCEINLINE void hw_pd_power_down_periph(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Peripheral Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_periph(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Peripherals Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_periph_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_UP) == 1;
}

/**
 * \brief Power up the Radio Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_UP)) == 0);
}

/**
 * \brief Power down the Radio Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_INLINE void hw_pd_power_down_rad(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Radio Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_rad(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Radio Power Domain.
 *
 * \return 0, if it is powered down and 1 if it is powered up.
 *
 */
__STATIC_INLINE bool hw_pd_check_rad_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_UP) == 1;
}


/**
 * \brief Power up the Communications Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, COM_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, COM_IS_UP)) == 0);
}

/**
 * \brief Power down the Communications Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_com(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, COM_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Communications Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_com(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, COM_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Communications Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_com_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1;
}

/**
 * \brief Power up the Timers Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_UP)) == 0);
}

/**
 * \brief Power down the Timers Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_tim(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Timers Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_tim(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, TIM_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Timers Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_tim_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 1;
}

/**
 * \brief Power up the Audio Power Domain.
 *
 */
__STATIC_FORCEINLINE void hw_pd_power_up_aud(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP);
        GLOBAL_INT_RESTORE();
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, AUD_IS_UP)) == 0);
}

/**
 * \brief Power down the Audio Power Domain.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * When calling this function, the PD will not be powered down immediately if there is an
 * activated PDC entry requesting this PD. In this case, the PD will be powered down when
 * the system enters sleep state.
 */
__STATIC_FORCEINLINE void hw_pd_power_down_aud(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, AUD_SLEEP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Wait for Audio Power Domain Power down.
 *
 * \note Power Domain Controller (PDC) can also control this power domain (PD).
 * The PD will not be powered down if there is a pending PDC entry for this PD.
 */
__STATIC_FORCEINLINE void hw_pd_wait_power_down_aud(void)
{
        while ((CRG_TOP->SYS_STAT_REG & REG_MSK(CRG_TOP, SYS_STAT_REG, AUD_IS_DOWN)) == 0);
}

/**
 * \brief Check the status of Audio Power Domain.
 *
 * \return false if it is powered down and true if it is powered up.
 */
__STATIC_INLINE bool hw_pd_check_aud_status(void)
{
        return REG_GETF(CRG_TOP, SYS_STAT_REG, AUD_IS_UP) == 1;
}


#endif /* dg_configUSE_HW_PD */


#endif /* HW_PD_H_ */

/**
\}
\}
*/
