/*
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
 */



#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/errno.h>
#include <stdlib.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "hw_bod.h"
#include "hw_cache.h"
#include "hw_clk.h"
#include "hw_cpm.h"
#include "hw_fcu.h"
#include "hw_gpio.h"
#include "hw_memctrl.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_pmu.h"
#include "hw_rtc.h"
#include "hw_qspi.h"
#include "hw_sys.h"
#include "qspi_automode.h"
#if (dg_configUSE_SYS_TCS == 1)
#include "sys_tcs.h"
#endif
#include "sys_trng.h"
#include "sys_drbg.h"
#if dg_configUSE_SYS_BOOT
#include "sys_boot.h"
#endif

#if dg_configUSE_CLOCK_MGR
#include "sys_clock_mgr.h"
#include "../../system/sys_man/sys_clock_mgr_internal.h"
#endif

#include "../../peripherals/src/hw_sys_internal.h"


/*
 * Linker symbols
 *
 * Note: if any of them is missing, please correct your linker script. Please refer to the linker
 * script of pxp_reporter.
 */
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint8_t end;
extern uint8_t __HeapLimit;

/*
 * Global variables
 */
__RETAINED_RW static uint8_t *heapend = &end;
__RETAINED_RW uint32_t SystemLPClock = dg_configXTAL32K_FREQ;   /*!< System Low Power Clock Frequency (LP Clock) */
__RETAINED_UNINIT uint8_t reset_stat_reg;


/**
 * @brief  Memory safe implementation of newlib's _sbrk().
 *
 */
__LTO_EXT
void *_sbrk(int incr)
{
        uint8_t *newheapstart;

        if (heapend + incr > &__HeapLimit) {
                /* Hitting this, means that the value of _HEAP_SIZE is too small.
                 * The value of incr is in stored_incr at this point. By checking the equation
                 * above, it is straightforward to determine the missing space.
                 */
                volatile int stored_incr __UNUSED;

                stored_incr = incr;
                ASSERT_ERROR(0);

                errno = ENOMEM;
                return (void *)-1;
        }

        newheapstart = heapend;
        heapend += incr;

        return newheapstart;
}

#if dg_configUSE_SYS_DRBG
/**
* \brief  SDK implementation of stdlib's rand().
*
*/
int rand(void)
{
        uint32_t rand_number = 0;

#if defined(OS_BAREMETAL)
       if (sys_drbg_read_index() >= dg_configUSE_SYS_DRBG_BUFFER_LENGTH) {
               sys_drbg_update();
       }
#endif
        sys_drbg_read_rand(&rand_number);
        return (int)(rand_number % ((uint32_t)RAND_MAX + 1));
}

/**
* \brief  SDK implementation of stdlib's srand().
*
*/
void srand (unsigned __seed)
{
}
#endif /* dg_configUSE_SYS_DRBG */

/*
 * Dialog default priority configuration table.
 *
 * Content of this table will be applied at system start.
 *
 * \note If interrupt priorities provided by Dialog need to be changed, do not modify this file.
 * Create similar table with SAME name instead somewhere inside code without week attribute,
 * and it will be used instead of table below.
 */
#pragma weak __dialog_interrupt_priorities
INTERRUPT_PRIORITY_CONFIG_START(__dialog_interrupt_priorities)
        PRIORITY_0, /* Start interrupts with priority 0 (highest) */
                /*
                 * Note: Interrupts with priority 0 are not
                 * allowed to perform OS calls.
                 */
        PRIORITY_1, /* Start interrupts with priority 1 */
                CMAC2SYS_IRQn,
                CRYPTO_IRQn,
                RFDIAG_IRQn,
        PRIORITY_2, /* Start interrupts with priority 2 */
                DMA_IRQn,
                I2C_IRQn,
                SPI_IRQn,
                GPADC_IRQn,
                SDADC_IRQn,
                SRC_IN_IRQn,
                SRC_OUT_IRQn,
                SRC2_IN_IRQn,
                SRC2_OUT_IRQn,
                FCU_IRQn,
        PRIORITY_3, /* Start interrupts with priority 3 */
                SysTick_IRQn,
                UART_IRQn,
                UART2_IRQn,
                MRM_IRQn,
                XTAL32M_RDY_IRQn,
                CLK_CALIBRATION_IRQn,
                KEY_WKUP_GPIO_IRQn,
                GPIO_P0_IRQn,
                GPIO_P1_IRQn,
                TIMER_IRQn,
#ifndef OS_PRESENT
                TIMER2_IRQn,
#endif /* OS_PRESENT */
                TIMER3_IRQn,
                TIMER4_IRQn,
                CAPTIMER_IRQn,
                RTC_IRQn,
                RTC_EVENT_IRQn,
                QUADDEC_IRQn,
                PCM_IRQn,
        PRIORITY_4, /* Start interrupts with priority 4 */
        PRIORITY_5, /* Start interrupts with priority 5 */
        PRIORITY_6, /* Start interrupts with priority 6 */
        PRIORITY_7, /* Start interrupts with priority 7 */
        PRIORITY_8, /* Start interrupts with priority 8 */
        PRIORITY_9, /* Start interrupts with priority 9 */
        PRIORITY_10, /* Start interrupts with priority 10 */
        PRIORITY_11, /* Start interrupts with priority 11 */
        PRIORITY_12, /* Start interrupts with priority 12 */
        PRIORITY_13, /* Start interrupts with priority 13 */
        PRIORITY_14, /* Start interrupts with priority 14 */
        PRIORITY_15, /* Start interrupts with priority 15 (lowest) */
#ifdef OS_PRESENT
                TIMER2_IRQn,
#endif /* OS_PRESENT */
INTERRUPT_PRIORITY_CONFIG_END

void set_interrupt_priorities(const int8_t prios[])
{
        uint32_t old_primask, iser, iser2;
        int i = 0;
        uint32_t prio = 0;

        // Assign all bit for preemption to be preempt priority bits.
        // (required by OS)
        NVIC_SetPriorityGrouping(0);

        /*
         * We shouldn't change the priority of an enabled interrupt.
         *  1. Globally disable interrupts, saving the global interrupts disable state.
         *  2. Explicitly disable all interrupts, saving the individual interrupt enable state.
         *  3. Set interrupt priorities.
         *  4. Restore individual interrupt enables.
         *  5. Restore global interrupt enable state.
         */
        old_primask = __get_PRIMASK();
        __disable_irq();
        iser  = NVIC->ISER[0];
        iser2 = NVIC->ISER[1];
        NVIC->ICER[0] = iser;
        NVIC->ICER[1] = iser2;

        for (i = 0; prios[i] != PRIORITY_TABLE_END; ++i) {
                switch (prios[i]) {
                case PRIORITY_0:
                case PRIORITY_1:
                case PRIORITY_2:
                case PRIORITY_3:
                case PRIORITY_4:
                case PRIORITY_5:
                case PRIORITY_6:
                case PRIORITY_7:
                case PRIORITY_8:
                case PRIORITY_9:
                case PRIORITY_10:
                case PRIORITY_11:
                case PRIORITY_12:
                case PRIORITY_13:
                case PRIORITY_14:
                case PRIORITY_15:
                        prio = prios[i] - PRIORITY_0;
                        break;
                default:
                        NVIC_SetPriority(prios[i], prio);
                        break;
                }
        }

        NVIC->ISER[0] = iser;
        NVIC->ISER[1] = iser2;
        __set_PRIMASK(old_primask);

        // enable Usage-, Bus-, and MMU Fault
        SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                   |  SCB_SHCSR_BUSFAULTENA_Msk
                   |  SCB_SHCSR_MEMFAULTENA_Msk;
}


#if dg_configUSE_CLOCK_MGR == 0

void XTAL32M_Ready_Handler(void)
{
        /* Hitting this assertion means that the settling of XTAL32M happened at some point to
         * take longer than expected. (The settling time for XTAL32M may vary from time to time,
         * depending on the crystal used, the off time (how long XTAL32M has remained disabled
         * before enabling it) and also on other conditions (environment etc). Normally these
         * variations are taken into account by configuring a high enough maximum expected settling
         * time, but there might be cases, were these expectations are exceeded). Thus: */
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                /* Consider increasing the margin added for the maximum expected settling time in
                 * hw_clk_xtalm_configure_irq(). Alternatively, a fixed maximum expected settling
                 * time can be applied by setting dg_configXTAL32M_SETTLE_TIME_IN_USEC to a
                 * sufficiently high value. */
        } else {
                /* Consider increasing dg_configXTAL32M_SETTLE_TIME_IN_USEC. */
        }
        ASSERT_WARNING(hw_clk_is_xtalm_started());
}

/* carry out clock initialization sequence */
static void nortos_clk_setup(void)
{
         hw_clk_enable_lpclk(LP_CLK_IS_RCLP);
         hw_clk_set_lpclk(LP_CLK_IS_RCLP);

         NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
         NVIC_EnableIRQ(XTAL32M_RDY_IRQn);              // Activate XTAL32M Ready IRQ

         hw_clk_xtalm_irq_enable();
         /* XTAL32M is already enabled in hw_clk_xtalm_configure_irq */
         /* Wait for XTAL32M to settle */
         while (!hw_clk_is_xtalm_started());

         hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);
}

#endif  /* dg_configUSE_CLOCK_MGR */

/*
 *  \brief Calculates and sets the appropriate (e)flash cacheable memory length
 *
 *  The booter sets a cacheable memory length for the (e)flash as it is defined in
 *  the CACHE_(E)FLASH_REG[(E)FLASH_REGION_SIZE] field. Nevertheless, the actual FW
 *  image is of a different size (in principle smaller) and to that extent we need
 *  to set the appropriate cacheable memory length that corresponds to the actual
 *  FW image size via the iCache Controller LLD API.
 *
 *  */
static __RETAINED_CODE void set_cacheable_memory_length(void)
{
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)) && (dg_configUSE_HW_CACHE == 1)

        uint32_t product_header_addr;
        uint32_t active_fw_image_addr;
        uint32_t active_fw_size;
        uint32_t cache_len;
        uint32_t scanned_sectors = 0;

        /* Calculate cacheable memory length according to the 'Active FW image address'
         * field of the product header and 'FW Size' field of the active FW image header. */

        /* Parse CS script to locate the actual location of product header.
         * Product header is located either at start of (e)FLASH or at the sector
         * boundary (0x4000) if a configuration script is used */
        HW_SYS_REMAP_ADDRESS_0 remap_adr0 = hw_sys_get_memory_remapping();
        if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_EFLASH) {
                /* eFLASH cached base address, which contains the instruction code,
                 * is remapped at address 0x0 for execution */
                product_header_addr = MEMORY_EFLASH_S_BASE;
        } else {
                /* FLASH cached base address, which contains the instruction code,
                 * is remapped at address 0x0 for execution */
                product_header_addr = MEMORY_QSPIF_S_BASE;
        }

        while ((((uint8_t*) product_header_addr)[0] != 0x50) &&
                (((uint8_t*) product_header_addr)[1] != 0x70) &&
                (scanned_sectors < 10)) {
                if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_EFLASH) {
                        /* Page size is 2KB */
                        product_header_addr += 0x800;
                } else if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH) {
                        /* Sector size is 4KB */
                        product_header_addr += 0x1000;
                }
                scanned_sectors++;
        }

        /* Get active_fw_image_addr */
        ASSERT_WARNING(((uint8_t*) product_header_addr)[0] == 0x50);
        ASSERT_WARNING(((uint8_t*) product_header_addr)[1] == 0x70);

        memcpy((uint8_t*) &active_fw_image_addr, &((uint8_t*) product_header_addr)[2], 4);
        if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH) {
                active_fw_image_addr += MEMORY_QSPIF_S_BASE;
        } else if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_EFLASH) {
                active_fw_image_addr += MEMORY_EFLASH_S_BASE;
        }

        /* Get active_fw_size and align it to 64K boundary */
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[0] == 0x51);
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[1] == 0x71);
        memcpy((uint8_t*) &active_fw_size, &((uint8_t*) active_fw_image_addr)[2], 4);
        active_fw_size += (0x10000 - (active_fw_size % 0x10000)) % 0x10000;

        /* Calculate length of eFLASH cacheable memory
         * (Cached area len will be (cache_len * 64) KBytes, cache_len can be 0 to 512) */
        cache_len = active_fw_size >> 16;

        if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH) {
                hw_cache_set_extflash_cacheable_len(cache_len);
        } else if (remap_adr0 == HW_SYS_REMAP_ADDRESS_0_TO_EFLASH) {
                hw_cache_set_eflash_cacheable_len(cache_len);
        }
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)) && (dg_configUSE_HW_CACHE == 1) */
}

/* this function configures the PDC table only the first time it is called */
static void configure_pdc(void)
{
        uint32_t pdc_entry_index __UNUSED;
        bool no_syscpu_pdc_entries = true;
        NVIC_DisableIRQ(PDC_IRQn);
        NVIC_ClearPendingIRQ(PDC_IRQn);

#if defined(CONFIG_USE_BLE) || (dg_configENABLE_DEBUGGER == 1)
        /* Set up PDC entry for CMAC2SYS IRQ or debugger */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_BLE)
        /*
         * Set up PDC entry for CMAC wakeup from MAC timer.
         * This entry is also used for the SYS2CMAC mailbox interrupt.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                        HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                        HW_PDC_PERIPH_TRIG_ID_MAC_TIMER,
                                                        HW_PDC_MASTER_CMAC,
                                                        HW_PDC_LUT_ENTRY_EN_XTAL));

        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
#endif

#if defined(OS_PRESENT)
        /* OS Timer requires PD_TIM to be always on */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 0) {
        }

        /* Add PDC entry to wakeup from Timer2 */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        /* cppcheck-suppress redundantAssignment */
        no_syscpu_pdc_entries = false;
#endif

        if (!no_syscpu_pdc_entries) {
                /* Let SYSCPU goto sleep when needed */
                REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 1);
                /* Leave XTAL32M enabled in case PDC is not triggered and XTAL32M is not controlled by PDC. */
#if dg_configENABLE_XTAL32M_ON_WAKEUP
                /* In all syscpu PDC entries the XTAL32M is configured to be controlled by PDC so we have to
                 * clear the XTAL32M_XTAL_ENABLE bit to allow PDC to disable XTAL32M when going to sleep. */
                hw_clk_disable_sysclk(SYS_CLK_IS_XTAL32M);
#else
                /* There is no PDC entry for XTAL32M. XTAL32M_XTAL_ENABLE bit is already set and will remain so.
                 * Application should configure PDC to control XTAL32M. */
#endif
        }

       /* clear the PDC IRQ since it will be pending here */
        NVIC_ClearPendingIRQ(PDC_IRQn);
}

#if dg_configUSE_CLOCK_MGR && dg_configUSE_HW_RTC
/* this function configures the RTC clock and RTC_KEEP_RTC_REG*/
static void configure_rtc(void)
{
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        cm_update_rtc_divider(rcx_clock_hz);
#else
        cm_update_rtc_divider(dg_configXTAL32K_FREQ);
#endif
        hw_rtc_set_keep_reg_on_reset(true);
}
#endif

/**
 * Weak function that enables an application function to be called immediately after SystemInitPre()
 */
__WEAK void UserApp_SystemInitPre(void)
{
}

/**
 * Basic system setup.
 *
 * @brief  Setup the AMBA clocks. Ensure proper alignment of copy and zero table entries.
 *
 * @note   No variable initialization should take place here, since copy & zero tables
 *         have not yet been initialized yet and any modifications to variables will
 *         be discarded. For the same reason, functions that initialize or are
 *         using initialized variables should not be called from here.
 */
void SystemInitPre(void) __attribute__((section("text_reset")));
void SystemInitPre(void)
{
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        assertion_functions_set_to_uninit();
#endif
        // Populate device information attributes
        ASSERT_WARNING(hw_sys_device_info_init());

        /*
         * Enable M33 debugger.
         */
        if (dg_configENABLE_DEBUGGER) {
                ENABLE_DEBUGGER;
        } else {
                DISABLE_DEBUGGER;
        }

        /*
         * Enable CMAC debugger.
         */
        if (dg_configENABLE_CMAC_DEBUGGER) {
                ENABLE_CMAC_DEBUGGER;
        } else {
                DISABLE_CMAC_DEBUGGER;
        }

        /*
         * Bandgap has already been set by the bootloader.
         * Use fast clocks from now on.
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, 0);
        hw_clk_set_pclk_div(0);

        /*
         * Disable pad latches
         */
        hw_gpio_pad_latch_disable_all();

        /*
         * Check that the firmware and the chip that it runs on are compatible with each other.
         */
        ASSERT_WARNING(hw_sys_is_compatible_chip());

        /*
         * Ensure 4-byte alignment for all elements of each entry in the Copy Table.
         * If any of the assertions below hits, please correct your linker script
         * file accordingly!
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {

                const uint32_t copy_table_size = (uintptr_t) &__copy_table_end__ - (uintptr_t)&__copy_table_start__;

                const uint32_t *copy_table_end = &__copy_table_start__ + (copy_table_size / sizeof(uint32_t));

                const uint32_t zero_table_size = (uintptr_t) &__zero_table_end__ - (uintptr_t)&__zero_table_start__;

                const uint32_t *zero_table_end = &__zero_table_start__ + (zero_table_size / sizeof(uint32_t));

                uint32_t *p;

                for (p = &__copy_table_start__; p < copy_table_end;) {
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );     // from
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );     // to
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );     // size
                }
                /*
                 * Ensure 4-byte alignment for all elements of each entry in the Data Table.
                 * If any of the assertions below hits, please correct your linker script
                 * file accordingly!
                 */
                for (p = &__zero_table_start__; p < zero_table_end;) {
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // start at
                        ASSERT_WARNING( (*p++ & 0x3) == 0 );    // size
                }
        }

        /*
         * Clear all PDC entries and make sure SYS_SLEEP is 0.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 0);
        hw_pdc_lut_reset();

        /*
         * Reset memory controller.
         */
        hw_memctrl_reset();

        /*
         * Initialize power domains
         */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_DOWN));
        /*
         * PD_TIM is kept active so that XTAL and PLL registers
         * can be programmed properly in SystemInit.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));
        GLOBAL_INT_RESTORE();

        /*
         * Keep CMAC core under reset
         */
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_SYNCH_RESET, 1);

        /*
         * Disable unused peripherals
         */

        /*
         * The booter activates the QSPI controller to access the external QSPI flash memory.
         * However, if the application does not utilize an external QSPI flash memory, the
         * QSPIC will be deactivated to reduce power consumption.
         */
#if (dg_configUSE_HW_QSPI == 0)
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE, 0);
#endif

        REG_SETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE, 0);

        UserApp_SystemInitPre();
}


__STATIC_FORCEINLINE void switch_to_rc32_and_configure_cur_set(void)
{
        /* Switch to RC32M if needed. */
        if (hw_clk_get_sysclk() != SYS_CLK_IS_RC32) {
                hw_clk_enable_sysclk(SYS_CLK_IS_RC32);
                hw_clk_set_sysclk(SYS_CLK_IS_RC32);
                hw_clk_disable_xtalm();
        }
        ASSERT_WARNING(hw_clk_xtalm_configure_cur_set() >= 0);
}

__STATIC_FORCEINLINE void setup_clocks_and_pdc(void)
{
        /* When the XiP Flash contains a secure image, the bootrom performs a CRC calculation/validation of the image header.
         * This validation process is resource-intensive and takes a significant amount of time.
         * To accelerate this process, the bootrom enables the doubler (which uses Xtal32M) to increase the speed.
         * However, if a host application attempts to load a RAM project (such as uartboot) during this validation,
         * the bootrom execution is interrupted before switching the system clock back to RC32M.
         * In that case, since, in the following steps, the Xtal32M is configured (and the system needs to be running on RC32M
         * in order to do that), we have to switch to RC32M manually.
         */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        if ((hw_clk_get_sysclk() == SYS_CLK_IS_DBLR)) {
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);
                hw_clk_dblr_sys_off();
                hw_pmu_vdd_set_voltage(HW_PMU_VDD_VOLTAGE_SLEEP_0V90);
                hw_pmu_vdcdc_set_voltage(HW_PMU_VDCDC_VOLTAGE_1V10);
        }
#endif

#if (dg_configUSE_SYS_TCS == 1)
        /* If no trim value for XTAL32M_TRIM_REG is stored in CS, calculate and apply
         * the optimum value for XTAL32M_TRIM_REG[XTAL32M_CUR_SET]. */
        bool is_reg_trimmed = false;
        const uint32_t reg_in_cs = (uint32_t) &CRG_XTAL->XTAL32M_TRIM_REG;
        if (!sys_tcs_reg_pairs_in_cs(&reg_in_cs, 1, &is_reg_trimmed)) {
                switch_to_rc32_and_configure_cur_set();
        }
#else
        /* We cannot check if a trim value for XTAL32M_TRIM_REG is stored in CS.
         * Thus, we have to apply the preferred value for XTAL32M_TRIM and then also
         * calculate and apply the optimum value for XTAL32M_CUR_SET anyway. */
        REG_SETF(CRG_XTAL, XTAL32M_TRIM_REG, XTAL32M_TRIM,
                dg_configDEFAULT_XTAL32M_TRIM_REG__XTAL32M_TRIM__VALUE);
        switch_to_rc32_and_configure_cur_set();
#endif /* dg_configUSE_SYS_TCS */

#if dg_configUSE_CLOCK_MGR
#if (CM_ENABLE_RC32M_CALIBRATION == 1)
        /* Trim RC32M. This needs to be done before calling hw_clk_xtalm_configure_irq(),
         * because the XTAL32M's digital FSM is clocked by RC32M. */
        if (hw_clk_get_sysclk() == SYS_CLK_IS_RC32) {
                hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);
                while (!hw_clk_is_xtalm_started());
                hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);
        }

        cm_calibrate_rc32m();
#endif
#endif

        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                hw_clk_xtalm_configure_irq();
        }
#if dg_configUSE_CLOCK_MGR
        cm_clk_init_low_level_internal();

        configure_pdc();

        // Always enable the XTAL32M
        cm_enable_xtalm();
        while (!cm_poll_xtalm_ready());                 // Wait for XTAL32M to settle
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32M as sys_clk

#if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX))
        /*
         * Note: If the LP clock is the RCX then we have to wait for the XTAL32M to settle
         *       since we need to estimate the frequency of the RCX before continuing
         *       (calibration procedure).
         */
        cm_rcx_init();
        hw_clk_set_lpclk(LP_CLK_IS_RCX);        // Set RCX as the LP clock
#endif

#if dg_configUSE_HW_RTC
        configure_rtc();
#endif

#if (CM_ENABLE_RCLP_CALIBRATION == 1)
        cm_calibrate_rclp();
#endif

#else
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {
                hw_clk_set_xtalm_settling_time(XTAL32M_USEC_TO_250K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC)/8, false);
        }

        configure_pdc();

        /* perform clock initialization here, as there is no clock manager to do it later for us */
        nortos_clk_setup();
#endif /* dg_configUSE_CLOCK_MGR */
}


static void da1459x_SystemInit(void)
{
        /* Keep RESET status for debugging purpose */
        reset_stat_reg = CRG_TOP->RESET_STAT_REG;
        CRG_TOP->RESET_STAT_REG = 0;

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
        assertion_functions_set_to_init();
#endif
        // Populate device information attributes
        ASSERT_WARNING(hw_sys_device_info_init());

        /*
         * Apply default priorities to interrupts.
         */
        set_interrupt_priorities(__dialog_interrupt_priorities);

        SystemLPClock = dg_configXTAL32K_FREQ;

#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED) && (dg_configUSE_HW_CACHE == 1))
        /* Disable the iCache Controller before reinitializing the QSPI */
        hw_cache_disable();
#endif


        /* The bootloader may have left the Flash in wrong mode */
#if dg_configQSPI_AUTOMODE_ENABLE
        qspi_automode_init();
#endif /* dg_configQSPI_AUTOMODE_ENABLE */

#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED) && (dg_configUSE_HW_CACHE == 1))
        /* Re-enable the iCache Controller after the QSPI is reinitialized */
        hw_cache_enable();
#endif

#if (HW_FCU_ENABLE_BUS_ERROR_AT_BOOT)
        hw_fcu_enable_bus_error();
#endif
        hw_fcu_set_flash_mass_erase_time(dg_configEFLASH_MASS_ERASE_TIME);
        hw_fcu_set_flash_page_erase_time(dg_configEFLASH_PAGE_ERASE_TIME);
        hw_fcu_set_flash_program_time(dg_configEFLASH_WORD_WRITE_TIME);
#if dg_configUSE_SYS_BOOT && (dg_configCODE_LOCATION != NON_VOLATILE_IS_NONE)
        sys_boot_restore_product_headers();
#endif

        /* Calculate and set the appropriate cacheable (e)flash memory length */
        set_cacheable_memory_length();

        /* Already up in SystemInitPre()
         * PD_TIM is kept active here in order to program XTAL and PLL registers*/
        ASSERT_WARNING(hw_pd_check_tim_status());
#if (dg_configUSE_SYS_TCS == 1)
        /* get TCS values */
        sys_tcs_get_trim_values_from_cs();

        /*
         * Populate device variant information. This function must be called after retrieving the
         * TCS values from CS otherwise the relevant information is not available.
         */
        ASSERT_WARNING(hw_sys_device_variant_init());
#endif
        /*
         * Apply trimmed values for xtal32m in case no entry exists in OTP
         */
        hw_sys_apply_default_values();

#if (dg_configUSE_SYS_TRNG == 1)  && (dg_configUSE_SYS_DRBG == 1)
        if (sys_trng_can_run()) {
                /* After a power cycle the TRNG module can be fed with random data.
                 * This is a prerequisite for the generation of a random seed.
                 */
                ASSERT_WARNING(SYS_TRNG_ERROR_NONE == sys_trng_init());

                /* Set the seed for the random number generator function (it runs only once). */
                sys_drbg_srand();

                /* Generate random numbers */
                sys_drbg_init();
        } else {
                /* Should not end up here after a power cycle! */
                sys_drbg_init();
        }
#else
#if (dg_configUSE_SYS_DRBG == 1)
        if (sys_drbg_can_run()) {
                /* Set the seed for the random number generator function (it runs only once). */
                sys_drbg_srand();
        }
        /* Generate random numbers */
        sys_drbg_init();
#endif /* dg_configUSE_SYS_DRBG */
#endif /* dg_configUSE_SYS_TRNG && dg_configUSE_SYS_DRBG */


        /*
         * Initialize busy status register
         */
        hw_sys_sw_bsr_init();

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        /* This is needed to initialize stdout, so that it can be used by putchar (that doesn't initialize stdout,
         * contrary to printf). Putchar is needed by the Unity test framework
         * This also has the side effect of changing stdout to unbuffered (which seems more reasonable)
         */
        setvbuf(stdout, NULL, _IONBF, 0);
#endif

        /*
         * Keep PD_PER enabled.
         */
        hw_sys_pd_periph_enable();

#if (dg_configUSE_SYS_TCS == 1)
        /*
         * Apply tcs settings.
         * They need to be re-applied when the blocks they contain are enabled.
         * PD_MEM is by default enabled.
         * PD_AON settings are applied by the booter
         */
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_MEM);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_PER);
#endif
        /* In non baremetal apps PD_COMM will be opened by the  power manager */
#ifdef OS_BAREMETAL
        hw_sys_pd_com_enable();
#if (dg_configUSE_SYS_TCS == 1)
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_COMM);
#endif
#endif
#if (dg_configUSE_SYS_TCS == 1)
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SYS);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_TMR);

        /*
         * Apply custom trim settings which don't require the respective block to be enabled
         */
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, sys_tcs_custom_values_system_cb, NULL);
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, sys_tcs_custom_values_system_cb, NULL);
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_TEMP_SENS_25C, sys_tcs_custom_values_system_cb, NULL);
#endif
        /*
         * Apply preferred settings on top of tcs settings.
         */
        hw_sys_set_preferred_values(HW_PD_AON);
        hw_sys_set_preferred_values(HW_PD_SYS);
        hw_sys_set_preferred_values(HW_PD_TMR);
#ifdef OS_BAREMETAL
        hw_sys_set_preferred_values(HW_PD_COM);
#endif

        setup_clocks_and_pdc();

        /*
         * BOD protection
         */
#if (dg_configUSE_BOD == 1)
        /* BOD has already been enabled at this point but it must be reconfigured */
        hw_bod_configure();
#else
        hw_bod_deactivate();
#endif




}

typedef void (* init_func_ptr)(void);
/*
 * Add pointer to da1459x_SystemInit() in an array that will go in the .preinit_array section.
 * __libc_init_array() (which is called by _start()) calls all function pointers in .preinit_array.
 */
__attribute__ ((__used__, section(".preinit_array"), aligned(__alignof__(init_func_ptr))))
static init_func_ptr __da1459x_SystemInit_init_array_entry[] = {
        da1459x_SystemInit,
};


