/**
 * \addtogroup PLA_DRI_PER_OTHER
 * \{
 * \addtogroup HW_WKUP Wakeup Controller Driver
 * \{
 * \brief Wakeup Controller LLD API
 */

/**
 *****************************************************************************************
 *
 * @file hw_wkup_v2.h
 *
 * @brief Definition of API for the Wakeup Controller Low Level Driver.
 *
 * Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef HW_WKUP_V2_H_
#define HW_WKUP_V2_H_


#if dg_configUSE_HW_WKUP

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"
#include "hw_gpio.h"

#define HW_WKUP_POL_P0_BASE_REG            (volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG)
#define HW_WKUP_SELECT_KEY_P0_BASE_REG     (volatile uint32_t *)(&WAKEUP->WKUP_SELECT_P0_REG)
#define HW_WKUP_SELECT_GPIO_P0_BASE_REG    (volatile uint32_t *)(&WAKEUP->WKUP_SEL_GPIO_P0_REG)
#define HW_WKUP_SELECT1_GPIO_P0_BASE_REG   (volatile uint32_t *)(&WAKEUP->WKUP_SEL1_GPIO_P0_REG)

/**
 * \brief Get the mask of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_MASK(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_POS(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_WKUP_REG_GETF(reg, field) \
                ((WAKEUP->WKUP_##reg##_REG & (WAKEUP_WKUP_##reg##_REG_##field##_Msk)) >> (WAKEUP_WKUP_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_WKUP_REG_SETF(reg, field, new_val) \
               WAKEUP->WKUP_##reg##_REG = ((WAKEUP->WKUP_##reg##_REG & ~(WAKEUP_WKUP_##reg##_REG_##field##_Msk)) | \
                        ((WAKEUP_WKUP_##reg##_REG_##field##_Msk) & ((new_val) << (WAKEUP_WKUP_##reg##_REG_##field##_Pos))))

/**
 * \brief Key and gpio trigger types
 *
 */
typedef enum {
        HW_WKUP_TRIG_DISABLED,          /**< Disabled*/
        HW_WKUP_TRIG_LEVEL_HI_DEB,      /**< Debounced (KEY), level sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_LEVEL_LO_DEB,      /**< Debounced (KEY), level sensitivity, polarity LOW trigger */
        HW_WKUP_TRIG_LEVEL_HI,          /**< Non- debounced (GPIO), level sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_LEVEL_LO,          /**< Non- debounced (GPIO), level sensitivity, polarity LOW trigger */
        HW_WKUP_TRIG_EDGE_HI,           /**< Non- debounced (GPIO), edge sensitivity, polarity HIGH trigger */
        HW_WKUP_TRIG_EDGE_LO            /**< Non- debounced (GPIO), edge sensitivity, polarity LOW trigger */
} HW_WKUP_TRIGGER;

/**
 * \brief Wakeup Controller configuration
 *
 *
 */
typedef struct {
        uint8_t debounce;                              /**< Debounce time in ms */
        uint32_t pin_wkup_state[HW_GPIO_PORT_MAX];     /**< Indicates per GPIO port if the index of a bitmasked pin is '1' that we want to associate a key event to that pin, should be '0' otherwise. */
        uint32_t pin_gpio_state[HW_GPIO_PORT_MAX];     /**< Indicates per GPIO port if the index of a bitmasked pin is '1' that we want to associate a gpio event to that pin, should be '0' otherwise. */
        uint32_t pin_trigger[HW_GPIO_PORT_MAX];        /**< Pin triggers in each port, bitmasks each bit describes HI (when set to '0') or LOW (when set to '1') trigger of corresponding pin in port */
        uint32_t gpio_sense[HW_GPIO_PORT_MAX];         /**< Gpio sensitivity in each port, 0 means level 1 means edge */
} wkup_config;

/**
 * \brief Wake up from hibernation controller pin settings
 *
 */
typedef enum {
        HW_WKUP_HIBERN_PIN_NONE = 0,    /**< Resets hibernation pins */
        HW_WKUP_HIBERN_P0_14_ONLY,      /**< If set, Pin P0_14 can be used to wake up from hibernation */
        HW_WKUP_HIBERN_P1_04_ONLY,      /**< If set, Pin P1_04 can be used to wake up from hibernation */
        HW_WKUP_HIBERN_BOTH_PINS,       /**< If set, any of the above pins can be used to wake up from hibernation */
} HW_WKUP_HIBERN_PIN;

/**
 * \brief Wake up from hibernation pin polarity settings
 *
 */
typedef enum {
        HW_WKUP_HIBERN_BOTH_PINS_ACTIVE_HIGH = 0,       /**< If set, both hibernation pins are active high */
        HW_WKUP_HIBERN_P0_14_ACTIVE_LOW_ONLY,           /**< If set, P0_14 hibernation pin is active low and P1_04 is active high */
        HW_WKUP_HIBERN_P1_04_ACTIVE_LOW_ONLY,           /**< If set, P1_04 hibernation pin is active low and P0_14 is active high */
        HW_WKUP_HIBERN_BOTH_PINS_ACTIVE_LOW,            /**< If set, both hibernation pins are active low */
} HW_WKUP_HIBERN_POLARITY;

/**
 * \brief Configure wake up from hibernation block
 *
 * \param [in] pin the GPIO that triggers wake up from hibernation
 * \param [in] pol the GPIO polarity
 *
 */
void hw_wkup_configure_hibernation(HW_WKUP_HIBERN_PIN pin, HW_WKUP_HIBERN_POLARITY pol);

typedef void (*hw_wkup_interrupt_cb)(void);

/**
 * \brief Initialize peripheral
 *
 * Resets Wakeup Controller to initial state, i.e. interrupt is disabled and all pin triggers are
 * disabled.
 *
 * cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_init(const wkup_config *cfg);

/**
 * \brief Configure peripheral
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_configure(const wkup_config *cfg);

/**
 * \brief Register KEY interrupt handler
 *
 * A callback function is registered to be called when an interrupt is generated. Interrupt is
 * automatically enabled after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_key_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */
void hw_wkup_register_key_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P0 interrupt handler
 *
 * A callback function is registered to be called when an interrupt is generated. Interrupt is
 * automatically enabled after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_clear_gpio_status(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */
void hw_wkup_register_gpio_p0_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P1 interrupt handler
 *
 * A callback function is registered to be called when an interrupt is generated. Interrupt is
 * automatically enabled after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_clear_gpio_status(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */
void hw_wkup_register_gpio_p1_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);


/**
 * \brief Unregister interrupt handlers
 *
 * Interrupts are automatically disabled in NVIC after calling this function.
 *
 */
void hw_wkup_unregister_interrupts(void);

/**
 * \brief Reset key interrupt
 *
 * \warning Function MUST be called by any user-specified key interrupt callback, to clear the interrupt.
 *
 */
__STATIC_INLINE void hw_wkup_reset_key_interrupt(void)
{
        WAKEUP->WKUP_RESET_IRQ_REG = 1;
}

/**
 * \brief Interrupt handler
 *
 */
void hw_wkup_handler(void);

/**
 * \brief Set debounce time
 *
 * Setting debounce time to 0 will disable hardware debouncing. Maximum debounce time is 63ms.
 *
 * \param [in] time_ms debounce time in milliseconds
 *
 */
__STATIC_INLINE void hw_wkup_set_key_debounce_time(uint8_t time_ms)
{
        ASSERT_WARNING(time_ms <= 63);
        HW_WKUP_REG_SETF(CTRL, WKUP_DEB_VALUE, time_ms);
}

/**
 * \brief Get current debounce time
 *
 * \return debounce time in milliseconds
 *
 */
__STATIC_INLINE uint8_t hw_wkup_get_key_debounce_time(void)
{
        return HW_WKUP_REG_GETF(CTRL, WKUP_DEB_VALUE);
}

/**
 * \brief Configure a gpio or key trigger event
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] trigger gpio or key trigger setting
 *
 */
void hw_wkup_set_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_WKUP_TRIGGER trigger);

/**
 * \brief Get gpio or key trigger configuration
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \return gpio or key trigger settings
 *
 */
__STATIC_INLINE HW_WKUP_TRIGGER hw_wkup_get_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        uint32_t polarity  = (*(HW_WKUP_POL_P0_BASE_REG + port) & (0x1 << pin)) >> pin;
        uint32_t key_enabled = (*(HW_WKUP_SELECT_KEY_P0_BASE_REG + port) & (0x1 << pin)) >> pin;
        uint32_t gpio_enabled = (*(HW_WKUP_SELECT_GPIO_P0_BASE_REG + port) & (0x1 << pin)) >> pin;

        if (key_enabled) {
                return polarity ? HW_WKUP_TRIG_LEVEL_LO_DEB : HW_WKUP_TRIG_LEVEL_HI_DEB;
        }
        else if (gpio_enabled) {
                if ((*(HW_WKUP_SELECT1_GPIO_P0_BASE_REG + port) & (0x1 << pin)) >> pin) {
                        return polarity ? HW_WKUP_TRIG_EDGE_LO : HW_WKUP_TRIG_EDGE_HI;
                }
                return polarity ? HW_WKUP_TRIG_LEVEL_LO : HW_WKUP_TRIG_LEVEL_HI;
        }
        else {
                return HW_WKUP_TRIG_DISABLED;
        }
}

/**
 * \brief Emulate key hit
 *
 * Simulate Key event wake up trigger in case debounce time is set to 0
 *
 */
__STATIC_INLINE void hw_wkup_emulate_key_hit(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 1);
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 0);
}

/**
 * \brief Enable WKUP Key interrupts
 *
 * \note Differs from enabling the IRQs reception on M33 side (NVIC_EnableIRQ)
 *       that takes place during the hw_wkup_register_key_interrupt().
 */
__STATIC_INLINE void hw_wkup_enable_key_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 1);
}

/**
 * \brief Disable WKUP interrupts
 *
 */
__STATIC_INLINE void hw_wkup_disable_key_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 0);
}

/**
 * \brief Freeze wakeup timer
 *
 */
__STATIC_INLINE void hw_wkup_freeze_key_timer(void)
{
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Unfreeze wakeup controller timer
 *
 */
__STATIC_INLINE void hw_wkup_unfreeze_key_timer(void)
{
        GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Get port status on last wake up
 *
 * Meaning of bits in returned bitmask is the same as in hw_wkup_set_trigger().
 *
 * \return port pin event state bitmask
 *
 * \sa hw_wkup_set_trigger
 *
 */
__STATIC_INLINE uint32_t hw_wkup_get_gpio_status(HW_GPIO_PORT port)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                return HW_WKUP_REG_GETF(STATUS_P0, WKUP_STAT_P0);
        case HW_GPIO_PORT_1:
                return HW_WKUP_REG_GETF(STATUS_P1, WKUP_STAT_P1);
        default:
                ASSERT_WARNING(0);// Invalid argument
                return 0;         // Should never reach here
        }
}

/**
 * \brief Clear latch status
 *
 * \param [in] port port number
 * \param [in] status pin status bitmask
 *
 * \warning Function MUST be called by any user-specified interrupt callback, to clear the interrupt latch status
 *
 * \sa hw_wkup_get_gpio_status
 */
__STATIC_INLINE void hw_wkup_clear_gpio_status(HW_GPIO_PORT port, uint32_t status)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                HW_WKUP_REG_SETF(CLEAR_P0, WKUP_CLEAR_P0, status);
                break;
        case HW_GPIO_PORT_1:
                HW_WKUP_REG_SETF(CLEAR_P1, WKUP_CLEAR_P1, status);
                break;
        default:
                ASSERT_WARNING(0);//Invalid argument
        }
}



#endif /* dg_configUSE_HW_WKUP */
#endif /* HW_WKUP_V2_H_ */

/**
 * \}
 * \}
 */
