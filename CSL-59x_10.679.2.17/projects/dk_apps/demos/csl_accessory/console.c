/**
 ****************************************************************************************
 *
 * @file console.c
 *
 * @brief Console implementation
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "accessory_config.h"

#if (USE_CONSOLE == 1)
#include "platform_devices.h"
#include "osal.h"
#include "hw_cpm.h"
#include "ad_uart.h"
#include "notification_bits.h"
#include "fast_pair.h"
#include "app_nvparam.h"
#include "app_params.h"
#include "motion_detector.h"
#include "fn_control.h"

#define CONSOLE_MAX_ARGS        4
#define CONSOLE_MAX_CMD_LEN     50
#define CONSOLE_QUOTE_SYMBOL    '\''
#define CONSOLE_ESCAPE_SYMBOL   '\\'
#define CONSOLE_HELP_FORMAT     ( ">%20s %-32s- %s\r\n" )
#define CONSOLE_BACKSPACE       ( 0x08 )  /* Backspace. */
#define CONSOLE_DELETE          ( 0x7F )  /* Delete. */
#define CONSOLE_CR              ( 0x0D )  /* CR. */
#define CONSOLE_SPACE           ( 0x20 )  /* Space. */

#define shell_print(...)        printf(__VA_ARGS__)

extern FINDER_NETWORK_STATE accessory_get_finder_network_state(void);
extern void accessory_start_pairing_mode(void);
extern void accessory_enable_user_consent(void);
extern void accessory_factory_reset(void);
extern void accessory_control_advertising(bool start);

static void cmd_help(uint8_t argc, char *argv[]);
static void cmd_reset(uint8_t argc, char *argv[]);
static void cmd_pairing_mode(uint8_t argc, char *argv[]);
#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
static void cmd_stop_ring(uint8_t argc, char *argv[]);
#endif /* FP_FMDN_RING_COMPONENTS_NUM */
#endif /* FP_FMDN */
static void cmd_user_consent(uint8_t argc, char *argv[]);
static void cmd_advertise(uint8_t argc, char *argv[]);
static void cmd_motion_detected(uint8_t argc, char *argv[]);
static void cmd_serial(uint8_t argc, char *argv[]);

/* Console command handler type */
typedef void (*console_cmd_cb_t)(uint8_t argc, char *argv[]);

/* Console command definition structure */
typedef struct {
        char *name;
        uint8_t min_args;
        uint8_t max_args;
        console_cmd_cb_t handler;
        char *desc;
        char *syntax;
} console_command_t;

/* Buffer for UART receive characters */
__RETAINED static char console_buff[CONSOLE_MAX_CMD_LEN];
/* The length of UART received characters */
__RETAINED static uint8_t console_len;
/* Handle for UART adapter */
__RETAINED static ad_uart_handle_t uart_handle;
/* Current received character */
__RETAINED static char rx_char;
/* OS application task handle */
__RETAINED static OS_TASK taskHandle;

/* List of console command definitions  */
static const console_command_t console_commands[] = {
/*      command,     min max args, handler,             help text,             command args */
        {"help",        0, 0,      cmd_help,            "Display help",        ""},
        {"reset",       0, 1,      cmd_reset,           "Reset device",        "[factory]"},
        {"pairmode",    0, 0,      cmd_pairing_mode,    "Enable pairing mode", ""},
#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        {"stopring",    0, 0,      cmd_stop_ring,       "Stop ringing",        ""},
#endif /* FP_FMDN_RING_COMPONENTS_NUM */
#endif /* FP_FMDN */
        {"userconsent", 0, 0,      cmd_user_consent,    "Enable user consent", ""},
        {"advertise",   1, 1,      cmd_advertise,       "Control advertise",   "stop | start"},
        {"motion",      0, 0,      cmd_motion_detected, "Motion detected",     ""},
        {"serial",      0, 0,      cmd_serial,          "Print serial number", ""},
        {NULL,          0, 0,      NULL,                0,                     0},
};

/* Generic write used by std library */
__USED
int _write (int fd, char *ptr, int len)
{
        ad_uart_write(uart_handle, ptr, len);
        return len;
}

/* Help command handler */
static void cmd_help(uint8_t argc, char *argv[])
{
        const console_command_t *command = &console_commands[0];

        while (command->name) {
                shell_print(CONSOLE_HELP_FORMAT, command->name, command->syntax, command->desc);
                command++;
        }
}

/* Reset command handler */
static void cmd_reset(uint8_t argc, char *argv[])
{
        if ((argc == 2) && (strcmp(argv[1], "factory") == 0)) {
                shell_print("Clearing Storage\r\n");
                accessory_factory_reset();
                hw_cpm_reboot_system();
        } else if (argc == 1) {
                hw_cpm_reboot_system();
        } else {
                shell_print("Error: Invalid syntax for: reset\r\n");
        }
}

/* Pairing mode command handler */
static void cmd_pairing_mode(uint8_t argc, char *argv[])
{
        accessory_start_pairing_mode();
}

#if (FP_FMDN == 1)
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/* Stop ringing command handler */
static void cmd_stop_ring(uint8_t argc, char *argv[])
{
        FINDER_NETWORK_STATE cur_fn_state = accessory_get_finder_network_state();

        if (cur_fn_state == FINDER_NETWORK_GFP_FMDN) {
                if (fp_is_ringing()) {
                        fp_stop_ringing();
                } else {
                        shell_print("Warning: Ringing is already stopped\r\n");
                }

                return;
        }

        shell_print("Warning: Currently not available command\r\n");
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */
#endif /* FP_FMDN */

/* User consent command handler */
static void cmd_user_consent(uint8_t argc, char *argv[])
{
        accessory_enable_user_consent();
}

/* Advertise control command handler */
static void cmd_advertise(uint8_t argc, char *argv[])
{
        if (argc == 2) {
                if (strcmp(argv[1], "start") == 0) {
                        accessory_control_advertising(true);
                        return;
                } else if (strcmp(argv[1], "stop") == 0) {
                        accessory_control_advertising(false);
                        return;
                } else {
                        shell_print("Error: Invalid argument for: advertise\r\n");
                }
        } else {
                shell_print("Error: Invalid syntax for: advertise\r\n");
        }
}

/* Motion detected command handler */
static void cmd_motion_detected(uint8_t argc, char *argv[])
{
        motion_detector_set_motion_detected();
}

static void read_serial_number(uint8_t *serial)
{
        app_params_t params[] = {
                { .param = APP_PARAMS_SERIAL_NUMBER, .data = serial,
                  .len = SERIAL_NUMBER_LENGTH }
        };

        app_params_get_params(params, ARRAY_LENGTH(params), APP_PARAMS_TYPE_OTHER);
}

/* Print serial number */
static void cmd_serial(uint8_t argc, char *argv[])
{
        uint8_t serial_number[SERIAL_NUMBER_LENGTH + 1] = { 0 };

        memset(serial_number, ' ', SERIAL_NUMBER_LENGTH);
        read_serial_number(serial_number);
        shell_print("serial number: %s\r\n", serial_number);
}

/* Prepare arguments array for command */
static uint8_t console_make_argv(char *cmd, char *argv[])
{
        uint8_t argc;
        uint8_t i;
        uint8_t in_text_flag;
        uint8_t quotes;

        /* Break command into strings */
        argc = 0;
        in_text_flag = 0;
        quotes = 0;

        if (!cmd || !argv) {
                return 0;
        }

        for (i = 0; cmd[i] != '\0'; i++) {
                if (((quotes == 0) && ((cmd[i] == ' ') || (cmd[i] == '\t')))) {
                        if (in_text_flag) {
                                /* Set end of command line argument */
                                cmd[i] = '\0';
                                in_text_flag = 0;
                                quotes = 0;
                                /* Clear escape symbols */
                                if (argc > 0) {
                                }
                        }
                } else {
                        /* Got non-whitespace character */
                        if (!in_text_flag) {
                                /* Start of an argument */
                                in_text_flag = 1;

                                if (argc < CONSOLE_MAX_ARGS) {
                                        argv[argc] = &cmd[i + quotes];
                                        argc++;
                                } else {
                                        break;
                                }
                        }
                }
        }
        argv[argc] = 0;

        return argc;
}

/* UARD adapter read callback */
static void console_uart_read_cb(void *user_data, uint16_t transferred)
{
        OS_TASK_NOTIFY_FROM_ISR(taskHandle, CONSOLE_NOTIF, OS_NOTIFY_SET_BITS);
}

/**
 * \brief Initialize UART Console
 *
 * \param [in] appTaskHandle application task handle
 */
void console_uart_init(OS_TASK appTaskHandle)
{
        taskHandle = appTaskHandle;
        ad_uart_init();
        uart_handle = ad_uart_open(CONSOLE);
        ad_uart_read_async(uart_handle, &rx_char, 1, console_uart_read_cb, NULL);
}

/* Process console received characters */
static void console(void)
{
        uint8_t argc;
        const console_command_t *command;
        char *argv[CONSOLE_MAX_ARGS + 1];

        if ((rx_char != CONSOLE_CR) && (console_len < CONSOLE_MAX_CMD_LEN)) {
                switch (rx_char) {
                case CONSOLE_BACKSPACE:
                        case CONSOLE_DELETE:
                        if (console_len > 0) {
                                console_len -= 1;
                                char tmp[3] = { CONSOLE_BACKSPACE, ' ', CONSOLE_BACKSPACE };
                                ad_uart_write(uart_handle, tmp, 3);
                        }
                        break;

                default:
                        if ((console_len + 1) < CONSOLE_MAX_CMD_LEN) {
                                /* Only printable characters. */
                                if ((rx_char >= CONSOLE_SPACE) && (rx_char <= CONSOLE_DELETE)) {
                                        console_buff[console_len++] = rx_char;
                                        ad_uart_write(uart_handle, &rx_char, 1);
                                }
                        }
                        break;
                }
                return;
        }

        console_buff[console_len] = '\0';
        shell_print("\r\n");

        argc = console_make_argv(console_buff, argv);

        if (!argc) {
                shell_print("Shell>");
                return;
        }

        command = &console_commands[0];

        while (command->name) {
                if (strcmp(command->name, argv[0]) == 0) {
                        /* Command found */
                        if (((argc - 1) >= command->min_args)
                                && ((argc - 1) <= command->max_args)) {
                                if (!command->handler) {
                                        return;
                                }
                                command->handler(argc, argv);
                        } else {
                                shell_print("Error: Invalid syntax for: %s\r\n", argv[0]);
                        }
                        goto done;
                }
                command++;
        }

        if (command->name == 0) {
                shell_print("Error: No such command: %s\r\n", argv[0]);
        }

        done:
        console_len = 0;
        memset(console_buff, 0, CONSOLE_MAX_CMD_LEN);
        shell_print("Shell>");
}

/**
 * \brief Notify console processing
 *
 * It handles received character and setups UART to receive next character.
 *
 * \param [in] notif application task notification
 */
void console_process_notif(uint32_t notif)
{
    if (notif & CONSOLE_NOTIF) {
            console();
            ad_uart_read_async(uart_handle, &rx_char, 1, console_uart_read_cb, NULL);
    }
}

#endif /* USE_CONSOLE */
