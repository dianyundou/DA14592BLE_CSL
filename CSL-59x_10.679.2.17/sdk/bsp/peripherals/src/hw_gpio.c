/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup GPIO
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpio.c
 *
 * @brief Implementation of the GPIO Low Level Driver.
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
 ****************************************************************************************
 */
#if dg_configUSE_HW_GPIO


#include <stdint.h>
#include "hw_gpio.h"

/* Register adresses */
#define PX_DATA_REG_ADDR(_port)         ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_DATA_REG)) + _port)
#define PX_DATA_REG(_port)              *PX_DATA_REG_ADDR(_port)
#define PX_SET_DATA_REG_ADDR(_port)     ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_SET_DATA_REG)) + _port)
#define PX_SET_DATA_REG(_port)          *PX_SET_DATA_REG_ADDR(_port)
#define PX_RESET_DATA_REG_ADDR(_port)   ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_RESET_DATA_REG)) + _port)
#define PX_RESET_DATA_REG(_port)        *PX_RESET_DATA_REG_ADDR(_port)
#define PXX_MODE_REG_ADDR(_port, _pin)  ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_00_MODE_REG)) + (_port * 16)  + _pin)
#define PXX_MODE_REG(_port, _pin)       *PXX_MODE_REG_ADDR(_port, _pin)
#define PX_PADPWR_CTRL_REG_ADDR(_port)
#define PX_PADPWR_CTRL_REG(_port)

#define PX_WEAK_CTRL_REG_ADDR(_port)    ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_WEAK_CTRL_REG)) + _port)
#define PX_WEAK_CTRL_REG(_port)         *PX_WEAK_CTRL_REG_ADDR(_port)




#if (dg_configIMAGE_SETUP == PRODUCTION_MODE) && (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
        #error "GPIO assignment monitoring is active in PRODUCTION build!"
#endif

static volatile uint32_t GPIO_status[HW_GPIO_NUM_PORTS] = { 0 };

const uint8_t hw_gpio_port_num_pins[HW_GPIO_NUM_PORTS] = {
                                                HW_GPIO_PORT_0_NUM_PINS, HW_GPIO_PORT_1_NUM_PINS};

#if dg_configUSE_STATIC_IO_CONFIG
/* Static GPIO power configuration per port */
__RETAINED_RW uint32_t io_static_power_configuration[HW_GPIO_NUM_PORTS] = { 0 };
#endif /* dg_configUSE_STATIC_IO_CONFIG */


/*
 * Global Functions
 ****************************************************************************************
 */

void hw_gpio_configure(const gpio_config cfg[])
{
#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
        int num_pins = 0;
        uint32_t set_mask[HW_GPIO_NUM_PORTS] = { };
#endif

        if (!cfg) {
                return;
        }

        while (cfg->pin != 0xFF) {
                uint8_t port = cfg->pin >> HW_GPIO_PIN_BITS;
                uint8_t pin = cfg->pin & ((1 << HW_GPIO_PIN_BITS) - 1);

#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
                if (port >= HW_GPIO_NUM_PORTS || pin >= hw_gpio_port_num_pins[port]) {
                        /*
                         * invalid port or pin number specified, it was either incorrectly specified
                         * of cfg was not terminated properly using HW_GPIO_PINCONFIG_END so we're
                         * reading garbage
                         */
                        __BKPT(0);
                }

                if (++num_pins > HW_GPIO_NUM_PINS) {
                        /*
                         * trying to set more pins than available, perhaps cfg was not terminated
                         * properly using HW_GPIO_PINCONFIG_END?
                         */
                        __BKPT(0);
                }

                if (set_mask[port] & (1 << pin)) {
                        /*
                         * trying to set pin which has been already set by this call which means
                         * there is duplicated pin in configuration - does not make sense
                         */
                        __BKPT(0);
                }

                set_mask[port] |= (1 << pin);
#endif

                if (cfg->reserve) {
                        hw_gpio_reserve_and_configure_pin(port, pin, cfg->mode, cfg->func, cfg->high);
                } else {
                        hw_gpio_configure_pin(port, pin, cfg->mode, cfg->func, cfg->high);
                }
                cfg++;
        }
}

bool hw_gpio_reserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if ((GPIO_status[port] & (1 << pin))) {
                return false;
        }

        GPIO_status[port] |= (1 << pin);

        return true;
}

bool hw_gpio_reserve_and_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                HW_GPIO_FUNC function, bool high)
{
        if (!hw_gpio_reserve_pin(port, pin)) {
                return false;
        }

        hw_gpio_configure_pin(port, pin, mode, function, high);

        return true;
}

void hw_gpio_unreserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        GPIO_status[port] &= ~(1 << pin);
}

static void hw_gpio_verify_reserved(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
#if (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
        if (!(GPIO_status[port] & (1 << pin))) {
                // If debugger stops at this line, there is configuration problem
                // pin is used without being reserved first
                __BKPT(0); /* this pin has not been previously reserved! */
        }
#endif // (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
}


void hw_gpio_set_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                        HW_GPIO_FUNC function)
{
        hw_gpio_verify_reserved(port, pin);

        PXX_MODE_REG(port, pin) = mode | function;
}

void hw_gpio_get_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE* mode,
                                                                        HW_GPIO_FUNC* function)
{
        uint16_t val;

        hw_gpio_verify_reserved(port, pin);

        val = PXX_MODE_REG(port, pin);
        *mode = val & 0x0700;
        *function = val & 0x00ff;
}

void hw_gpio_set_pin_drive_strength(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_DRIVE_STRENGTH strength)
{
        /* GPIOs P0_0 .. P0_5 do not support the weak drive strength feature
         * Trying to set the Weak Drive on these GPIOs has no effect */
        if (port == HW_GPIO_PORT_0) {
                if (pin < HW_GPIO_PIN_6) {
                        return;
                }
        }

        /* For GPIOs other than P0_0 .. P0_5 set the drive strength
         * passed by hw_gpio_drive_strength */
        if (strength == HW_GPIO_DRIVE_STRENGTH_NORMAL) {
                PX_WEAK_CTRL_REG(port) &= ~(1 << pin);
        } else if (strength == HW_GPIO_DRIVE_STRENGTH_WEAK) {
                PX_WEAK_CTRL_REG(port) |= (1 << pin);
        }
}

HW_GPIO_DRIVE_STRENGTH hw_gpio_get_pin_drive_strength(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if ((PX_WEAK_CTRL_REG(port) & (1 << pin)) == 0) {
                return HW_GPIO_DRIVE_STRENGTH_NORMAL;
        } else {
                return HW_GPIO_DRIVE_STRENGTH_WEAK;
        }
}

void hw_gpio_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                        HW_GPIO_FUNC function, const bool high)
{
        hw_gpio_verify_reserved(port, pin);

        if (high)
                hw_gpio_set_active(port, pin);
        else
                hw_gpio_set_inactive(port, pin);

        hw_gpio_set_pin_function(port, pin, mode, function);
}


void hw_gpio_set_active(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        PX_SET_DATA_REG(port) = 1 << pin;
}

void hw_gpio_set_inactive(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        PX_RESET_DATA_REG(port) = 1 << pin;
}

bool hw_gpio_get_pin_status(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        return ( (PX_DATA_REG(port) & (1 << pin)) != 0 );
}

void hw_gpio_toggle(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        if (hw_gpio_get_pin_status(port, pin))
                hw_gpio_set_inactive(port, pin);
        else
                hw_gpio_set_active(port, pin);
}

int hw_gpio_get_pins_with_function(HW_GPIO_FUNC func, uint8_t *buf, int buf_size)
{
        int count = 0;
        int port;
        int pin;
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC pin_func;

        for (port = 0; port < HW_GPIO_NUM_PORTS; ++port) {
                for (pin = 0; pin < hw_gpio_port_num_pins[port]; ++pin) {
                        hw_gpio_get_pin_function(port, pin, &mode, &pin_func);
                        if (pin_func != func) {
                                continue;
                        }
                        if (count < buf_size && buf != NULL) {
                                buf[count] = (uint8_t) ((port << HW_GPIO_PIN_BITS) | pin);
                        }
                        count++;
                }
        }
        return count;
}

#endif /* dg_configUSE_HW_GPIO */
/**
 * \}
 * \}
 * \}
 */
