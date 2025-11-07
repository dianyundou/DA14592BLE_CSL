/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup FCU
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_fcu.c
 *
 * @brief Implementation of the FCU Low Level Driver.
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

#if dg_configUSE_HW_FCU

#include "sdk_defs.h"
#include "hw_fcu.h"

#define HW_FCU_DUMMY_DATA               (0x00FF00FF)

struct hw_fcu_write_state_t {
        const uint32_t *data;
        uint32_t len;
        uint32_t address;
};

struct hw_fcu_t {
        struct hw_fcu_write_state_t write_state;

        struct hw_fcu_operation_params_t params;
};

__RETAINED static struct hw_fcu_t hw_fcu;


__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_configure_erase_page_suspend(HW_FCU_ERASE_SUSPEND_MODE mode,
                                                                 uint32_t page_erase_time,
                                                                 uint32_t segment_erase_time)
{
        if (!WITHIN_RANGE(page_erase_time, HW_FCU_ERASE_FLASH_PAGE_MIN_TIME,
                          HW_FCU_ERASE_FLASH_PAGE_MAX_TIME + 1) ||
            !WITHIN_RANGE(segment_erase_time, HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MIN_TIME,
                          HW_FCU_ERASE_FLASH_PAGE_SEGMENT_MAX_TIME + 1) ||
            page_erase_time % segment_erase_time != 0) {
                return HW_FCU_ERROR_INVALID_FUNCTION_INPUT;
        }

        while (!hw_fcu_is_available());
        hw_fcu_set_erase_suspend_mode(mode);
        hw_fcu_set_flash_page_erase_time(page_erase_time);
        hw_fcu_set_flash_page_segment_erase_time(segment_erase_time);

        return HW_FCU_ERROR_NONE;
}

__ALWAYS_RETAINED_CODE static void hw_fcu_enable_irq()
{
        hw_fcu_clear_interrupt();
        NVIC_ClearPendingIRQ(FCU_IRQn);
        NVIC_EnableIRQ(FCU_IRQn);
}

__ALWAYS_RETAINED_CODE static void hw_fcu_disable_irq()
{
        NVIC_DisableIRQ(FCU_IRQn);
}

__ALWAYS_RETAINED_CODE static void hw_fcu_reset_operation_params()
{
        hw_fcu.params.cb = NULL;
        hw_fcu.params.user_data = NULL;
#if dg_configUSE_HW_DMA
        hw_fcu.params.dma_channel = HW_DMA_CHANNEL_INVALID;
#endif
}

__ALWAYS_RETAINED_CODE static HW_FCU_ERROR hw_fcu_erase(uint32_t address,
                                                 struct hw_fcu_operation_params_t *params)
{
        if (((address & 0x00000003) != 0) || (address >= MEMORY_EFLASH_SIZE)) {
                return HW_FCU_ERROR_INVALID_ADDRESS;
        }

        /* check if flash operations are prohibited or write operations are protected */
        if (hw_fcu_is_protected_against_actions(REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT) |
                                                REG_MSK(FCU, FLASH_CTRL_REG, FLASH_WPROT))) {
                return HW_FCU_ERROR_PROTECTED_AGAINST_ACTION;
        }

        /* return error if an erase is in progress */
        if (hw_fcu_is_erase_in_progress()) {
                return HW_FCU_ERROR_ERASE_IN_PROGRESS;
        }

        uint32_t addr = MEMORY_EFLASH_S_BASE + address;
        ASSERT_ERROR(IS_EFLASH_S_ADDRESS(addr));

        if (params) {
                hw_fcu.params.cb = params->cb;
                hw_fcu.params.user_data = params->user_data;
        } else {
                hw_fcu.params.cb = NULL;
                hw_fcu.params.user_data = NULL;
        }
#if dg_configUSE_HW_DMA
                hw_fcu.params.dma_channel = HW_DMA_CHANNEL_INVALID;
#endif
        hw_fcu_enable_reset_delay();
        /* If no callback is registered, use blocking mode */
        if (hw_fcu.params.cb == NULL) {
                *((uint32_t *)addr) = HW_FCU_DUMMY_DATA;
                while (hw_fcu_is_erase_in_progress());
                hw_fcu_disable_reset_delay();
        } else  {
                hw_fcu_enable_irq();
                *((uint32_t *)addr) = HW_FCU_DUMMY_DATA;
        }

        return HW_FCU_ERROR_NONE;
}

__ALWAYS_RETAINED_CODE void hw_fcu_wakeup(void)
{
        /* Make a dummy read to wake-up eflash */
        /* cppcheck-suppress unreadVariable */
        __UNUSED volatile uint32_t eflash = *((uint32_t *)MEMORY_EFLASH_S_BASE);

        /* wait till the HW actually wakes up */
        while (hw_fcu_is_asleep());
}

__ALWAYS_RETAINED_CODE void hw_fcu_sleep(void)
{
        while (!hw_fcu_is_available());
        /* Set flash to sleep mode */
        uint32_t ctrl = FCU->FLASH_CTRL_REG;
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_SEL, ctrl, HW_FCU_FLASH_ACCESS_MODE_READ);
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_MODE, ctrl, HW_FCU_FLASH_PROG_MODE_READ);
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, SLEEP_MODE, ctrl, 1);
        FCU->FLASH_CTRL_REG = ctrl;

        while (!hw_fcu_is_asleep());
}

__ALWAYS_RETAINED_CODE void hw_fcu_enable_erase(HW_FCU_FLASH_PROG_MODE mode)
{
        ASSERT_ERROR(mode == HW_FCU_FLASH_PROG_MODE_ERASE_BLOCK ||
                     mode == HW_FCU_FLASH_PROG_MODE_ERASE_PAGE);

        while (!hw_fcu_is_available());
        /* Set flash to erase mode */
        uint32_t ctrl = FCU->FLASH_CTRL_REG;
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_SEL, ctrl, HW_FCU_FLASH_ACCESS_MODE_WRITE_ERASE);
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_MODE, ctrl, mode);
        FCU->FLASH_CTRL_REG = ctrl;
}

__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_erase_page(uint32_t address,
                                               struct hw_fcu_operation_params_t *params)
{
        hw_fcu_enable_erase(HW_FCU_FLASH_PROG_MODE_ERASE_PAGE);

        return hw_fcu_erase(address, params);
}

__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_erase_block(uint32_t address,
                                                struct hw_fcu_operation_params_t *params)
{
        hw_fcu_enable_erase(HW_FCU_FLASH_PROG_MODE_ERASE_BLOCK);

        return hw_fcu_erase(address, params);
}

#if dg_configUSE_HW_DMA
__ALWAYS_RETAINED_CODE static void hw_fcu_dma_transfer_completed(void *user_data, dma_size_t len)
{
        HW_FCU_FLASH_PROG_MODE mode = hw_fcu_get_flash_programming_mode();
        if (mode == HW_FCU_FLASH_PROG_MODE_WRITE_PAGE) {
                hw_fcu_disable_dma();
                if (hw_fcu.params.cb) {
                        if (!hw_fcu_is_write_in_progress()) {
                                hw_fcu.params.cb(user_data);
                                if (NVIC_GetPendingIRQ(FCU_IRQn) == 0) {
                                        hw_fcu_disable_irq();
                                        hw_fcu_reset_operation_params();
                                        hw_fcu_disable_reset_delay();
                                }
                        } else {
                                /*
                                 * If DMA_EN is cleared in hw_fcu_dma_transfer_completed() before
                                 * the last word is written in flash, hw_fcu_write_irq_hander will
                                 * be called. So we set hw_fcu.write_state.len = 1 in order to call
                                 * user callback from FCU IRQ instead of DMA IRQ
                                 */
                                hw_fcu.write_state.len = 1;
                        }
                }
        } else if (mode == HW_FCU_FLASH_PROG_MODE_READ) {
                if (hw_fcu.params.cb) {
                        hw_fcu.params.cb(user_data);
                }
        }
}

__ALWAYS_RETAINED_CODE static void hw_fcu_prepare_dma(HW_DMA_CHANNEL dma_channel, uint32_t *src,
                                               uint32_t *dst, uint32_t len)
{
        DMA_setup dma = {
                .channel_number = dma_channel,
                .bus_width = HW_DMA_BW_WORD,
                .irq_enable = HW_DMA_IRQ_STATE_ENABLED,
                .irq_nr_of_trans = 0,
                .burst_mode = HW_DMA_BURST_MODE_DISABLED,
                .a_inc = HW_DMA_AINC_TRUE,
                .b_inc = HW_DMA_BINC_TRUE,
                .circular = HW_DMA_MODE_NORMAL,
                .dma_prio = HW_DMA_PRIO_7,
                .dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE,
                .dma_init = HW_DMA_INIT_AX_BX_AY_BY,
                .src_address = (uint32_t) src,
                .dest_address = (uint32_t) dst,
                .length = len,
                .callback = hw_fcu_dma_transfer_completed,
                .user_data = hw_fcu.params.user_data,
        };

        if (hw_fcu_get_flash_programming_mode() == HW_FCU_FLASH_PROG_MODE_READ) {
                dma.dreq_mode = HW_DMA_DREQ_START;
                dma.dma_req_mux = HW_DMA_TRIG_NONE;
        } else if (hw_fcu_get_flash_programming_mode() == HW_FCU_FLASH_PROG_MODE_WRITE_PAGE) {
                dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                dma.dma_req_mux = HW_DMA_TRIG_SD_ADC_FCU;
        }
        hw_dma_channel_initialization(&dma);
}
#endif /* dg_configUSE_HW_DMA */

__ALWAYS_RETAINED_CODE void hw_fcu_enable_write(void)
{
        while (!hw_fcu_is_available());
        /* Set flash to write mode */
        uint32_t ctrl = FCU->FLASH_CTRL_REG;
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_SEL, ctrl, HW_FCU_FLASH_ACCESS_MODE_WRITE_ERASE);
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_MODE, ctrl, HW_FCU_FLASH_PROG_MODE_WRITE_PAGE);
        FCU->FLASH_CTRL_REG = ctrl;
}

__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_write(uint32_t *src, uint32_t address, uint32_t len,
                                          struct hw_fcu_operation_params_t *params)
{
        /*
         * src pointer address should be word aligned.
         * Avoid casting uint8_t* or uint16_t* to uint32_t*, as it may cause unaligned access
         */
        ASSERT_ERROR(((uint32_t)src & 0x00000003) == 0);

        if ((src == NULL) || (len == 0) || (((len << 2) + address) > MEMORY_EFLASH_SIZE)) {
                return HW_FCU_ERROR_INVALID_FUNCTION_INPUT;
        }

        if (((address & 0x00000003) != 0) || (address >= MEMORY_EFLASH_SIZE)) {
                return HW_FCU_ERROR_INVALID_ADDRESS;
        }
#if dg_configUSE_HW_DMA
        if (params) {
                if ((params->dma_channel != HW_DMA_CHANNEL_INVALID) &&
                    ((params->dma_channel & 0x1) == 0)) {
                        return HW_FCU_ERROR_INVALID_FUNCTION_INPUT;
                }
        }
#endif
        /* check if flash operations are prohibited or write operations are protected */
        if (hw_fcu_is_protected_against_actions(REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT) |
                                                REG_MSK(FCU, FLASH_CTRL_REG, FLASH_WPROT))) {
                return HW_FCU_ERROR_PROTECTED_AGAINST_ACTION;
        }

        /* return error if a write is in progress */
        if (hw_fcu_is_write_in_progress()) {
                return HW_FCU_ERROR_WRITE_IN_PROGRESS;
        }

        hw_fcu_enable_write();

        hw_fcu.write_state.address = MEMORY_EFLASH_S_BASE + address;
        hw_fcu.write_state.data = src;
        hw_fcu.write_state.len = len;

        ASSERT_ERROR(IS_EFLASH_S_ADDRESS(hw_fcu.write_state.address));

        if (params) {
                hw_fcu.params.cb = params->cb;
                hw_fcu.params.user_data = params->user_data;
#if dg_configUSE_HW_DMA
                hw_fcu.params.dma_channel = params->dma_channel;
#endif
        }

        uint32_t *flash_address = (uint32_t*)hw_fcu.write_state.address;
        hw_fcu_enable_reset_delay();
        if (hw_fcu.params.cb == NULL) {
                while (hw_fcu.write_state.len > 0) {
                        flash_address = (uint32_t*)hw_fcu.write_state.address;
                        *flash_address = *hw_fcu.write_state.data;
                        hw_fcu.write_state.data++;
                        hw_fcu.write_state.address += 4;
                        hw_fcu.write_state.len--;
                        while (hw_fcu_is_write_in_progress());
                }
                hw_fcu_disable_reset_delay();
        } else {
                hw_fcu_enable_irq();
#if dg_configUSE_HW_DMA
                if (hw_fcu.params.dma_channel != HW_DMA_CHANNEL_INVALID) {
                        hw_fcu_prepare_dma(hw_fcu.params.dma_channel,
                                (uint32_t*)hw_fcu.write_state.data,
                                flash_address, hw_fcu.write_state.len);
                        hw_fcu_enable_dma();
                        hw_dma_channel_enable(hw_fcu.params.dma_channel, HW_DMA_STATE_ENABLED);
                        return HW_FCU_ERROR_NONE;
                }
#endif
                while (hw_fcu_is_write_in_progress())                                ;
                *flash_address = *hw_fcu.write_state.data;
        }

        return HW_FCU_ERROR_NONE;
}

__ALWAYS_RETAINED_CODE void hw_fcu_enable_read(void)
{
        while (!hw_fcu_is_available());

        /* Set flash to read mode */
        uint32_t ctrl = FCU->FLASH_CTRL_REG;
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_SEL, ctrl, HW_FCU_FLASH_ACCESS_MODE_READ);
        REG_SET_FIELD(FCU, FLASH_CTRL_REG, PROG_MODE, ctrl, HW_FCU_FLASH_PROG_MODE_READ);
        FCU->FLASH_CTRL_REG = ctrl;
}

__ALWAYS_RETAINED_CODE HW_FCU_ERROR hw_fcu_read(uint32_t address, uint32_t *dst, uint32_t len,
                                         struct hw_fcu_operation_params_t *params)
{
        /*
         * dst pointer address should be word aligned.
         * Avoid casting uint8_t* or uint16_t* to uint32_t*, as it may cause unaligned access
         */
        ASSERT_ERROR(((uint32_t)dst & 0x00000003) == 0);

        if ((dst == NULL) || (len == 0) || (((len << 2) + address) > MEMORY_EFLASH_SIZE)) {
                return HW_FCU_ERROR_INVALID_FUNCTION_INPUT;
        }

        if (((address & 0x00000003) != 0) || (address >= MEMORY_EFLASH_SIZE)) {
                return HW_FCU_ERROR_INVALID_ADDRESS;
        }

        /* check if flash operations are prohibited or read operations are protected */
        if (hw_fcu_is_protected_against_actions(REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT) |
                                                REG_MSK(FCU, FLASH_CTRL_REG, FLASH_RPROT))) {
                return HW_FCU_ERROR_PROTECTED_AGAINST_ACTION;
        }

        hw_fcu_enable_read();

        if (params) {
                hw_fcu.params.cb = params->cb;
                hw_fcu.params.user_data = params->user_data;
#if dg_configUSE_HW_DMA
                hw_fcu.params.dma_channel = params->dma_channel;
#endif
        }

        uint32_t *flash_address = (uint32_t *)(MEMORY_EFLASH_S_BASE + address);
        ASSERT_ERROR(IS_EFLASH_S_ADDRESS((uint32_t)flash_address));
#if dg_configUSE_HW_DMA
        if (hw_fcu.params.cb != NULL &&
            hw_fcu.params.dma_channel  != HW_DMA_CHANNEL_INVALID) {
                hw_fcu_prepare_dma(hw_fcu.params.dma_channel, flash_address, dst, len);
                hw_dma_channel_enable(hw_fcu.params.dma_channel, HW_DMA_STATE_ENABLED);
                return HW_FCU_ERROR_NONE;
        }
#endif
        for (uint32_t i = 0; i < len; i++) {
                dst[i] = flash_address[i];
        }

        return HW_FCU_ERROR_NONE;
}

__ALWAYS_RETAINED_CODE bool hw_fcu_is_available(void)
{
        uint32_t mask = REG_MSK(FCU, FLASH_CTRL_REG, PROG_ERS) |
                        REG_MSK(FCU, FLASH_CTRL_REG, PROG_WRS) |
                        REG_MSK(FCU, FLASH_CTRL_REG, PROG_RMIN) |
                        REG_MSK(FCU, FLASH_CTRL_REG, SLEEP) |
                        REG_MSK(FCU, FLASH_CTRL_REG, FLASH_PROT);

        return ((FCU->FLASH_CTRL_REG & mask) == 0);
}

__ALWAYS_RETAINED_CODE static void hw_fcu_write_irq_hander(void)
{
        hw_fcu.write_state.len--;
        if (hw_fcu.write_state.len > 0) {
                uint32_t *flash_address;

                hw_fcu.write_state.data++;
                hw_fcu.write_state.address += 4;
                flash_address = (uint32_t*)hw_fcu.write_state.address;
                *flash_address = *hw_fcu.write_state.data;
        } else {
                if (hw_fcu.params.cb) {
                        hw_fcu.params.cb(hw_fcu.params.user_data);
                }
                if (NVIC_GetPendingIRQ(FCU_IRQn) == 0) {
                        hw_fcu_disable_irq();
                        hw_fcu_reset_operation_params();
                        hw_fcu_disable_reset_delay();
                }
        }
}

__ALWAYS_RETAINED_CODE static void hw_fcu_erase_irq_handler(void)
{
        if (hw_fcu.params.cb) {
                hw_fcu.params.cb(hw_fcu.params.user_data);
        }
        if (NVIC_GetPendingIRQ(FCU_IRQn) == 0) {
                hw_fcu_disable_irq();
                hw_fcu_reset_operation_params();
                hw_fcu_disable_reset_delay();
        }
}

__ALWAYS_RETAINED_CODE void FCU_Handler(void)
{
        HW_FCU_FLASH_PROG_MODE mode;

        hw_fcu_clear_interrupt();
        mode = hw_fcu_get_flash_programming_mode();

        if (mode == HW_FCU_FLASH_PROG_MODE_WRITE_PAGE) {
                hw_fcu_write_irq_hander();
        } else if ((mode == HW_FCU_FLASH_PROG_MODE_ERASE_PAGE) ||
                   (mode == HW_FCU_FLASH_PROG_MODE_ERASE_BLOCK)) {
                hw_fcu_erase_irq_handler();
        }
}

#endif /* dg_configUSE_HW_FCU */
/**
 * \}
 * \}
 * \}
 */
