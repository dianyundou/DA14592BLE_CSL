/**
\addtogroup PLA_DRI_PER_OTHER
\{
\addtogroup HW_SYS System Hardware Driver
\{
\brief System Driver
*/

/**
 ****************************************************************************************
 *
 * @file hw_sys.h
 *
 * @brief System Driver header file.
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

#ifndef HW_SYS_H_
#define HW_SYS_H_
#if dg_configUSE_HW_SYS

#include "sdk_defs.h"
#include "hw_pd.h"

#include <hw_memctrl.h>

/* \brief Specifies the HW BSR mask used for accessing the SW BSR */
#define SW_BSR_HW_BSR_MASK     (0x3)

/*
 * \brief Enumerations used when accessing the HW BSR register
 */
typedef enum {
        HW_BSR_MASTER_NONE = 0,
        HW_BSR_MASTER_SYSCPU = 2,
        HW_BSR_MASTER_CMAC = 3,
        HW_BSR_MASTER_NUM,
} HW_BSR_MASTER_ID;

/*
 * \brief HW BSR position
 */
typedef enum {
        HW_BSR_SW_POS = 0,
        HW_BSR_POWER_CTRL_POS = 28,
        HW_BSR_WAKEUP_CONFIG_POS = 30,
} HW_BSR_POS;

/*
 * \brief Enumerations used when accessing the SW BSR variable
 */
typedef enum {
        SW_BSR_MASTER_NONE = 1 << HW_BSR_MASTER_NONE,
        SW_BSR_MASTER_SYSCPU = 1 << HW_BSR_MASTER_SYSCPU,
        SW_BSR_MASTER_CMAC = 1 << HW_BSR_MASTER_CMAC,
} SW_BSR_MASTER_ID;

typedef enum {
        BSR_PERIPH_ID_SPI = 1,
        BSR_PERIPH_ID_UART1 = 2,
        BSR_PERIPH_ID_UART2 = 3,
        BSR_PERIPH_ID_I2C = 4,
        BSR_PERIPH_ID_GPADC = 5,
        BSR_PERIPH_ID_SDADC = 6,
        BSR_PERIPH_ID_MAX = 14,
} HW_SYS_BSR_PERIPH_ID;

/**
 * \brief Register configuration
 *
 */
typedef struct {
        __IO uint32_t *addr;    //!< Register address
        uint32_t value;         //!< Register value
} hw_sys_reg_config_t;

/*
 * Export hw_sys_sw_bsr so that other masters have access to its address.
 */
extern __RETAINED uint32_t hw_sys_sw_bsr[BSR_PERIPH_ID_MAX];

typedef enum {
        HW_SYS_REMAP_ADDRESS_0_TO_ROM,
        HW_SYS_REMAP_ADDRESS_0_TO_EFLASH,
        HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH,
        HW_SYS_REMAP_ADDRESS_0_TO_RAM,
} HW_SYS_REMAP_ADDRESS_0;

__STATIC_INLINE void hw_sys_set_memory_remapping(HW_SYS_REMAP_ADDRESS_0 value)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0, value);
        GLOBAL_INT_RESTORE();
}

__STATIC_INLINE HW_SYS_REMAP_ADDRESS_0 hw_sys_get_memory_remapping(void)
{
        HW_SYS_REMAP_ADDRESS_0 value;
        GLOBAL_INT_DISABLE();
        value = REG_GETF(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0);
        GLOBAL_INT_RESTORE();
        return value;
}

/**
 * \brief Enable Cache retainability.
 *
 */
__STATIC_INLINE void hw_sys_set_cache_retained(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE);
        GLOBAL_INT_RESTORE();
}


/**
 * \brief Setup the Retention Memory configuration.
 *
 */
__STATIC_INLINE void hw_sys_setup_retmem(void)
{
        GLOBAL_INT_DISABLE();

        CRG_TOP->RAM_PWR_CTRL_REG = dg_configMEM_RETENTION_MODE;

        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable memory retention.
 *
 */
__STATIC_INLINE void hw_sys_no_retmem(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, PMU_CTRL_REG, RETAIN_CACHE);
        CRG_TOP->RAM_PWR_CTRL_REG = RETMEM_RETAIN_NONE;
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Prepare RESET type tracking.
 */
__STATIC_FORCEINLINE void hw_sys_track_reset_type(void)
{
        CRG_TOP->RESET_STAT_REG = 0;
}


/**
 * \brief Activate the "Reset on wake-up" functionality.
 *
 */
__STATIC_INLINE void hw_sys_enable_reset_on_wup(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, PMU_CTRL_REG, RESET_ON_WAKEUP);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the preferred settings of a power domain.
 *
 * \param [in] pd power domain
 */
void hw_sys_set_preferred_values(HW_PD pd);

/**
 * \brief Set the GPIO used for the SW cursor to High-Z.
 *
 */
void hw_sys_setup_sw_cursor(void);

/**
 * \brief Triggers the GPIO used for the SW cursor.
 *
 */
void hw_sys_trigger_sw_cursor(void);

/**
 * \brief Enable the debugger.
 *
 */
__STATIC_FORCEINLINE void hw_sys_enable_debugger(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Disable the debugger.
 *
 */
__STATIC_FORCEINLINE void hw_sys_disable_debugger(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, DEBUGGER_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Check if the debugger is attached.
 *
 * \return true, if the debugger is attached, else false.
 *
 */
__STATIC_FORCEINLINE bool hw_sys_is_debugger_attached(void)
{
        return (REG_GETF(CRG_TOP, SYS_STAT_REG, DBG_IS_ACTIVE) != 0);
}


/**
 * \brief Enable hibernation mode.
 *
 */
__STATIC_FORCEINLINE void hw_sys_enable_hibernation(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, HIBERN_CTRL_REG, HIBERNATION_ENABLE);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enable CMAC cache RAM.
 *
 * \warning To be called only prior to BLE initialization. After processing the respected
 *          memory block that falls into the CMAC cache RAM range, it should be disabled.
 *          \sa hw_sys_disable_cmac_cache_ram.
 */
__STATIC_FORCEINLINE void hw_sys_enable_cmac_cache_ram(void)
{
        hw_pd_power_up_rad();
        REG_SET_BIT(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE);
}

/**
 * \brief Disable CMAC cache RAM.
 *
 * \warning To be called only prior to BLE initialization, after processing a memory block
 *          that falls into the CMAC cache RAM range. \sa hw_sys_enable_cmac_cache_ram.
 */
__STATIC_FORCEINLINE void hw_sys_disable_cmac_cache_ram(void)
{
        REG_CLR_BIT(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE);
        hw_pd_power_down_rad();
}


/**
 * \brief  Trigger a GPIO when ASSERT_WARNING() or ASSERT_ERROR() hits.
 *
 */
__ALWAYS_RETAINED_CODE void hw_sys_assert_trigger_gpio(void);

/**
 * \brief Enables read only protection in CMAC code and data
 *
 */
void hw_sys_enable_cmac_mem_protection(void);

/**
 * \brief Enables "Read-only by any privilege level" and "execute_never" memory protection of IVT
 *
 */
void hw_sys_enable_ivt_mem_protection(void);


/**
 * \brief Set POR-trigger minimum duration
 *
 * The function configures the Power-On-Reset (POR) timer, which
 * defines how long the nRST or the POR-configured GPIO pin should
 * be asserted in order to trigger a POR.
 *
 * \param [in] time time in 4096 * {RCLP@32kHz period} units (~ 0.128 sec).
 *                  Should be less than 128 (~16.2 sec).
 *
 * \note In case of triggering a POR from a GPIO, hw_gpio_configure_por_pin()
 *       should also be used.
 *
 * \note Setting \b time to 0 disables POR generation completely (even for nRST).
 *
 * \note Default (reset) value is 0x18 (~3 sec).
 *
 * \note If a POR is successfully triggered, the POR timer is re-set (at startup)
 *       to its default value (Of course, later, the application may set it again
 *       to a different value, as desired).
 *
 * \sa hw_gpio_configure_por_pin()
 *
 */
__STATIC_FORCEINLINE void hw_sys_set_por_timer(uint8_t time)
{
        ASSERT_WARNING(time <= (REG_MSK(CRG_TOP, POR_TIMER_REG, POR_TIME)
                             >> REG_POS(CRG_TOP, POR_TIMER_REG, POR_TIME)));
        REG_SETF(CRG_TOP, POR_TIMER_REG, POR_TIME, time);
}

/**
 * \brief  Try to lock a BSR entry
 *
 * \param [in] hw_bsr_master_id The HW BSR ID of the relevant master
 * \param [in] pos The position of the entry in BSR. Valid positions are provided in HW_BSR_POS.
 *
 * \return true if the BSR entry has been acquired, else false.
 *
 */
bool hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos);

/**
 * \brief  Unlock a BSR entry
 *
 * \param [in] hw_bsr_master_id The HW BSR ID of the relevant master
 * \param [in] pos The position of the entry in BSR. Valid positions are provided in HW_BSR_POS.
 *
 */
void hw_sys_hw_bsr_unlock(HW_BSR_MASTER_ID hw_bsr_master_id, HW_BSR_POS pos);

/**
 * \brief Initializes the software busy status register.
 */
void hw_sys_sw_bsr_init(void);

/**
 * \brief Tries to acquire exclusive access to a specific peripheral when
 *        it is also used by other masters (CMAC).
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be granted.
 *                       Valid range is (0 - BSR_PERIPH_ID_MAX). Check HW_SYS_BSR_PERIPH_ID.
 *
 * \return true if peripheral exclusive access has been acquired, else false.
 */
bool hw_sys_sw_bsr_try_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Checks if exclusive access to a specific peripheral has been acquired
 *        from a given master.
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access will be checked.
 *                       Valid range is (0 - BSR_PERIPH_ID_MAX). Check HW_SYS_BSR_PERIPH_ID.
 * \return true if peripheral exclusive access has been acquired from the specific master, else false.
 */
bool hw_sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Releases the exclusive access from a specific peripheral so it
 *        it can be also used by other masters (CMAC).
 *
 * \param [in] sw_bsr_master_id The SW BSR ID of the relevant master
 * \param [in] periph_id The peripheral id for which exclusive access must be released.
 *                       Valid range is (0 - BSR_PERIPH_ID_MAX). Check HW_SYS_BSR_PERIPH_ID.
 */
void hw_sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id);

/**
 * \brief Enables the COM power domain.
 *
 */
void hw_sys_pd_com_enable(void);

/**
 * \brief Disables the COM power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
void hw_sys_pd_com_disable(void);

/**
 * \brief Enables the PERIPH power domain.
 *
 */
void hw_sys_pd_periph_enable(void);

/**
 * \brief Disables the PERIPH power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
void hw_sys_pd_periph_disable(void);

/**
 * \brief Enables the AUDIO power domain.
 *
 */
__RETAINED_CODE void hw_sys_pd_audio_enable(void);

/**
 * \brief Disables the AUDIO power domain. If it has not
 *        been enabled by any other modules, it will be disabled.
 */
__RETAINED_CODE void hw_sys_pd_audio_disable(void);

/**
 * \brief Add register configuration entries in the system register configuration table
 *
 * \param [in] config pointer to the structure containing the register configuration
 * \param [in] num_of_entries the number of entries in the register configuration structure
 *
 * \return the index of the first entry in the configuration table
 */
uint32_t hw_sys_reg_add_config(const hw_sys_reg_config_t *config, uint32_t num_of_entries);

/**
 * \brief Get a register configuration entry
 *
 * \param [in] index the index of the entry in the register configuration table
 *
 * \return a pointer to the register configuration entry
 */
hw_sys_reg_config_t *hw_sys_reg_get_config(uint32_t index);

/**
 * \brief Modify a register configuration entry
 *
 * \param [in] index the index of the entry in the register configuration table
 * \param [in] addr the new register address
 * \param [in] value the new register value
 */
void hw_sys_reg_modify_config(uint32_t index, __IO uint32_t *addr, uint32_t value);

/**
 * \brief Get the number of entries in the system register configuration table
 *
 * \return a pointer to the number of entries
 */
uint32_t *hw_sys_reg_get_num_of_config_entries(void);

/**
 * \brief Apply system register configuration
 *
 * Configure non-retained system registers using the entries in the system register
 * configuration table.
 */
__RETAINED_CODE void hw_sys_reg_apply_config(void);
/**
 * \brief Checks whether there are register entries in CS for the following registers
 *       - CLK_RC32M_REG
 *       - XTAL32M_START_REG
 *       - XTAL32M_TRIM_REG
 * For each register that there is no register entry the function applies the default values
 * for specific fields of this register
 *       - CLK_RC32M_REG: RC32M_BIAS, RC32M_RANGE, RC32M_COSC
 *       - XTAL32M_START_REG: XTAL32M_TRIM
 *       - XTAL32M_TRIM_REG: XTAL32M_TRIM_REG
 */
void hw_sys_apply_default_values(void);


#endif /* dg_configUSE_HW_SYS */
#endif /* HW_SYS_H_ */

/**
 * \}
 * \}
 */
