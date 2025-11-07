/**
 ****************************************************************************************
 *
 * @file config.c
 *
 * @brief System level configurations settings regarding the debug logging mechanism
 * (UART based retargeting (CONFIG_RETARGET), or SWD based via Segger's SystemView tool
 * (dg_configSYSTEMVIEW) or by employing Segger's Real Time Transfer technology (CONFIG_RTT)),
 * protection checks and related error definitions.
 *
 * Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdbool.h>
#include <stdio.h>
#include "sdk_defs.h"
#ifdef OS_PRESENT
#       include "osal.h"
#endif

/* System debug logging configuration specific header files inclusion */
#ifdef CONFIG_RETARGET
#       include <stddef.h>
#       include "hw_uart.h"
#       include "hw_gpio.h"
#       include "hw_sys.h"
# if  (dg_configUSE_CONSOLE == 1)
#       include "../adapters/src/sys_platform_devices_internal.h"
#       include "console.h"
# endif

# if dg_configSYS_DBG_LOG_PROTECTION
#       include <stdarg.h>
#       include <string.h>
# endif /* dg_configSYS_DBG_LOG_PROTECTION */
#elif defined CONFIG_RTT
#       include <stdarg.h>
#       include "SEGGER_RTT.h"
#elif defined CONFIG_NO_PRINT
#elif dg_configSYSTEMVIEW
#       include <string.h>
#       include <stdarg.h>
#       include "SEGGER_SYSVIEW.h"
#       include "SEGGER_SYSVIEW_ConfDefaults.h"
#endif
#include "interrupts.h"


/* System debug logging configuration specific preprocessor, variables and functions definitions */
#if defined CONFIG_RETARGET

# ifndef CONFIG_RETARGET_UART
        #define CONFIG_RETARGET_UART            SER1_UART

        #define CONFIG_RETARGET_UART_TX_PORT    SER1_TX_PORT
        #define CONFIG_RETARGET_UART_TX_PIN     SER1_TX_PIN
        #define CONFIG_RETARGET_UART_TX_MODE    SER1_TX_MODE
        #define CONFIG_RETARGET_UART_TX_FUNC    SER1_TX_FUNC

        #define CONFIG_RETARGET_UART_RX_PORT    SER1_RX_PORT
        #define CONFIG_RETARGET_UART_RX_PIN     SER1_RX_PIN
        #define CONFIG_RETARGET_UART_RX_MODE    SER1_RX_MODE
        #define CONFIG_RETARGET_UART_RX_FUNC    SER1_RX_FUNC
# endif

# ifndef CONFIG_RETARGET_UART_BAUDRATE
#       define CONFIG_RETARGET_UART_BAUDRATE    HW_UART_BAUDRATE_115200
# endif

# ifndef CONFIG_RETARGET_UART_DATABITS
#       define CONFIG_RETARGET_UART_DATABITS    HW_UART_DATABITS_8
# endif

# ifndef CONFIG_RETARGET_UART_STOPBITS
#       define CONFIG_RETARGET_UART_STOPBITS    HW_UART_STOPBITS_1
# endif

# ifndef CONFIG_RETARGET_UART_PARITY
#       define CONFIG_RETARGET_UART_PARITY      HW_UART_PARITY_NONE
# endif

#define RETARGET_UART_IS_CONFIGURED_FLAG        (0x15)

/* System debug logging protection mechanism definitions */
# if dg_configSYS_DBG_LOG_PROTECTION

static __RETAINED bool single_char_print = false;       /* If true, indicates a single char printf() call. */
static __RETAINED bool retarget_initialized = false;    /* If true, the retarget module (config.c) is initialized. */

static char string[dg_configSYS_DBG_LOG_MAX_SIZE] = "";
        static __RETAINED OS_MUTEX sys_dbg_log_mutex;           /* Provides mutual exclusion for contenting printf()s */
        #define MAX_LEN                         (dg_configSYS_DBG_LOG_MAX_SIZE)
# endif /* dg_configSYS_DBG_LOG_PROTECTION */

void retarget_init(void)
{
#if dg_configUSE_CONSOLE
        console_init(&sys_platform_console_controller_conf);
#endif /* dg_configUSE_CONSOLE */

#if dg_configSYS_DBG_LOG_PROTECTION
        if (!retarget_initialized) {
                OS_MUTEX_CREATE(sys_dbg_log_mutex);
                retarget_initialized = true;
        }
#endif /* dg_configSYS_DBG_LOG_PROTECTION */
}

#if !dg_configUSE_CONSOLE
static void retarget_reinit(void)
{
        uart_config uart_init = {
                .baud_rate = CONFIG_RETARGET_UART_BAUDRATE,
                .data      = CONFIG_RETARGET_UART_DATABITS,
                .stop      = CONFIG_RETARGET_UART_STOPBITS,
                .parity    = CONFIG_RETARGET_UART_PARITY,
                .use_fifo  = 1,
# if (HW_UART_DMA_SUPPORT == 1)
                .use_dma   = 0,
                .rx_dma_channel = HW_DMA_CHANNEL_INVALID,
                .tx_dma_channel = HW_DMA_CHANNEL_INVALID,
# endif
        };

        hw_uart_init(CONFIG_RETARGET_UART, &uart_init);
        hw_uart_write_scr(CONFIG_RETARGET_UART, RETARGET_UART_IS_CONFIGURED_FLAG);
}

__STATIC_INLINE bool uart_needs_initialization(void)
{
        const uint32_t uart_clk_enables = CRG_COM->CLK_COM_REG;
        enum {
                UART_ENABLE = CRG_COM_CLK_COM_REG_UART_ENABLE_Msk,
                UART2_ENABLE = CRG_COM_CLK_COM_REG_UART2_ENABLE_Msk
        };
        if (CONFIG_RETARGET_UART == HW_UART2) {
                return (!(uart_clk_enables & UART2_ENABLE)
                        || (hw_uart_read_scr(HW_UART2) != RETARGET_UART_IS_CONFIGURED_FLAG));
        } else {
                return (!(uart_clk_enables & UART_ENABLE)
                        || (hw_uart_read_scr(HW_UART1) != RETARGET_UART_IS_CONFIGURED_FLAG));
        }
        /* Code flow should not reach here */
        ASSERT_WARNING("UART not initialized");
        return true;
}

# if !dg_configSYS_DBG_LOG_PROTECTION
/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RETARGET UART.
 */
__USED
__WEAK int _write (int fd, char *ptr, int len)
{
        (void) fd;  /* Not used, avoid warning */
        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /* Write "len" of char from "ptr" to file id "fd"
         * Return number of char written. */
        hw_uart_send(CONFIG_RETARGET_UART, ptr, len, NULL, NULL);

        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}
        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();

        return len;
}

/* Writes (wr) a character (ch) to the console (tty) */
void _ttywrch(int ch)
{
        _write(1, (char*) &ch, 1);
}
# endif /* !dg_configSYS_DBG_LOG_PROTECTION */

__USED
int _read (int fd, char *ptr, int len)
{
        (void) fd;  /* Not used, avoid warning */
        int ret = 0;

        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_RX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_RX);

        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /*
         * we need to wait for anything to read and return since stdio will assume EOF when we just
         * return 0 from _read()
         */
        while (!hw_uart_is_data_ready(CONFIG_RETARGET_UART)) {
# if defined(OS_PRESENT)
                /*
                 * Use some short sleep to give a time for the Idle task to make its work
                 * e.g. freeing memory in OS e.g. deleting task if it is not needed anymore.
                 */
                OS_DELAY(2);
# endif /* OS_PRESENT */
        }

        /* and now read as much as possible */
        while (hw_uart_is_data_ready(CONFIG_RETARGET_UART) && ret < len) {
                ptr[ret++] = hw_uart_read(CONFIG_RETARGET_UART);
        }

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_RX);
        hw_sys_pd_com_disable();

        return ret;
}

# if dg_configSYS_DBG_LOG_PROTECTION
/*
 * Overridden libC standard output functions. They are enabled only for M33 OS-based applications
 * that aim to resolve contention issues when more than one M33 tasks aim to print in parallel.
 */

/* Ancillary print function that calls UART LLD with the specified string to be output. */
static int vprint(const char *fmt, va_list argp)
{
        int len = 0;

        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        len = strlen(fmt);

        if (len > MAX_LEN) {
                hw_uart_send(CONFIG_RETARGET_UART, "Error in printing more than max chars!\n", 39, NULL, NULL);
        } else {
                /* Print the actual debug logging string */
                len = vsnprintf(string, MAX_LEN, fmt, argp);
                if (len > 0) {
                        hw_uart_send(CONFIG_RETARGET_UART, string, len, NULL, NULL);
                } else {
                        hw_uart_send(CONFIG_RETARGET_UART, "Error in composing the debug message!\n", 38, NULL, NULL);
                }
        }
        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();
        return len;
}

/* Overridden libC printf() with a mutual exclusion mechanism for contenting debug logging activity. */
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        int ret;
        va_list param_list;

        /* Enable retarget mechanism if it's not enabled - can happen if it was omitted in application */
        if (!retarget_initialized) {
            retarget_init();
        }

        /* Intra-processor (M33) mutual exclusion mechanism based on mutex use */
        OS_MUTEX_GET(sys_dbg_log_mutex, OS_MUTEX_FOREVER);
        /* ------ CRITICAL SECTION START ------ */
        va_start(param_list, format);
        ret = vprint(format, param_list);
        va_end(param_list);
        /* ------ CRITICAL SECTION END -------- */

        OS_MUTEX_PUT(sys_dbg_log_mutex);
        return ret;
}

/*
 * Overridden libC puts() with a mutual exclusion mechanism for contenting debug logging activity.
 * It is used when calling directly puts() in application context, or in case the debug logging string
 * when calling printf() contains one or more newline chars ('\n') in the end but no format specifiers.
 * It is used also by the overridden putchar() when invoked for single character printf() statements.
 *
 * In an application deployment it is recommended to avoid using directly puts() as by design an extra
 * newline char ('\n') is appended in the end of the debug logging string.
 */
int puts(const char *s)
{
        int len = 0;

        /* Enable retarget mechanism if it's not enabled - can happen if it was omitted in application */
        if (!retarget_initialized) {
            retarget_init();
        }

        /* Intra-processor (M33) mutual exclusion mechanism based on mutex use */
        OS_MUTEX_GET(sys_dbg_log_mutex, OS_MUTEX_FOREVER);

        /* ------ CRITICAL SECTION START ------ */
        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
            retarget_reinit();
        }

        len = strlen(s);

        /* The puts() libC implementation discards a newline char ('\n') in the end of a string
         * that was attempted to be printed via printf() so we need to send it independently. */
        if (len > 1 || ((len == 1) && !single_char_print)) {
                if (len > MAX_LEN) {
                        hw_uart_send(CONFIG_RETARGET_UART, "Error in printing more than max chars!\n", 39, NULL, NULL);
                } else {
                        /* Step 2 - Print the actual debug logging string */
                        hw_uart_send(CONFIG_RETARGET_UART, s, len, NULL, NULL);
                        /* Step 3 - Append a newline char ('\n') in the debug logging string */
                        hw_uart_send(CONFIG_RETARGET_UART, "\n", 1, NULL, NULL);
                }
        }
        /* When invoked from a printf() that aims to send a single character to serial output */
        else {
                hw_uart_send(CONFIG_RETARGET_UART, s, len, NULL, NULL);
        }
        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();
        /* ------ CRITICAL SECTION END -------- */

        OS_MUTEX_PUT(sys_dbg_log_mutex);
        return len;
}
/* Overridden libC putchar() with a mutual exclusion mechanism via puts() for contenting debug logging activity. */
int putchar(int a)
{
        int ret;
        char *ptr = (char *)&a;
        single_char_print = true;
        ret = puts(ptr);

        return ret;
}
#  endif /* dg_configSYS_DBG_LOG_PROTECTION */
# endif /* !dg_configUSE_CONSOLE */
#elif defined CONFIG_RTT

/*
 * override libC printf()
 */
extern int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));

int printf(const char *__restrict format, ...)
{
        int ret;
        va_list param_list;

        va_start(param_list, format);
        ret = SEGGER_RTT_vprintf(0, format, &param_list);
        va_end(param_list);
        return ret;
}

/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        SEGGER_RTT_Write(0, ptr, len);
        return len;
}

/*
 *       _read()
 *
 * Function description
 *   Low-level read function.
 *   libC subroutines will use this system routine for input to all files,
 *   including stdin.
 *   Read data via RTT.
 */
int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = SEGGER_RTT_WaitKey();

        if (len > 1) {
                ret += SEGGER_RTT_Read(0, ptr + 1, len - 1);
        }

        return ret;
}

int _putc(int a)
{
        char *ptr = (char *)&a;
        int ret;
        ret = SEGGER_RTT_Write(0, ptr, 1);
        return ret;
}
#elif dg_configSYSTEMVIEW

# if ( !defined(SEGGER_RTT_MAX_INTERRUPT_PRIORITY) || (SEGGER_RTT_MAX_INTERRUPT_PRIORITY > configMAX_SYSCALL_INTERRUPT_PRIORITY) )
#  pragma GCC error "\r\n\n\t\t\
The SEGGER_RTT_MAX_INTERRUPT_PRIORITY must be defined and be less or equal to configMAX_SYSCALL_INTERRUPT_PRIORITY. \r\n\t\t\
Please set the correct value by defining the dg_configSEGGER_RTT_MAX_INTERRUPT_PRIORITY macro in the application's custom_config_qspi.h file, \r\n\t\t\
and set it to value smaller or equal to the one defined for the configMAX_SYSCALL_INTERRUPT_PRIORITY macro in the FreeRTOSConfig.h \r\n"
# endif

extern void _VPrintHost(const char* s, U32 Options, va_list* pParamList);

int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        va_list ParamList;
        va_start(ParamList, format);
        _VPrintHost(format, SEGGER_SYSVIEW_LOG, &ParamList);
        va_end(ParamList);
        return 0;
}

/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        static char send_buf[SEGGER_SYSVIEW_MAX_STRING_LEN - 1];
        int send_len;

        /*
         * Messages bigger than SEGGER_SYSVIEW_MAX_STRING_LEN are not supported by
         * systemview, so only the first SEGGER_SYSVIEW_MAX_STRING_LEN chars will
         * be actually sent to host.
         */
        send_len = (sizeof(send_buf) - 1 > len) ? len : sizeof(send_buf) - 1 ;
        memcpy(send_buf, ptr, send_len);
        send_buf[send_len] = '\0';
        SEGGER_SYSVIEW_Print(send_buf);

        return len;
}


/*
 *       _read()
 *
 * Function description
 *   Low-level read function.
 *   libC subroutines will use this system routine for input to all files,
 *   including stdin.
 *   Empty stub.
 */
int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}
#elif defined CONFIG_NO_PRINT || (!defined CONFIG_CUSTOM_PRINT && !defined(CONFIG_SEMIHOSTING))

/* CONFIG_NO_PRINT, by default */

/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libC subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Empty stub.
 */
int _write(int file, char *ptr, int len) {
        return len;
}

/*
 *       _read()
 *
 * Function description
 *   Low-level read function.
 *   libC subroutines will use this system routine for input to all files,
 *   including stdin.
 *   Empty stub.
 */
int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}

/*
 * override libC printf()
 *
 * empty stub
 */
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        return 0;
}

int puts(const char *s)
{
        return EOF;
}

int _putc(int c)
{
        return EOF;
}

int putchar(int c)
{
        return EOF;
}
#endif

/*
 * System configuration checks
 */
#ifdef PRINT_POWER_RAIL_SETUP

# define PPRS_THEME "\n\n******* 1V8 and 1V8P power rails & Flash operational mode configuration *******\n"



# if (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8P)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8P"
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif
#  define PPRS_FLASH_POWER_OFF "\t"

# elif (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8"
#  if (dg_configFLASH_POWER_OFF)
#   define PPRS_FLASH_POWER_OFF "\nFlash Power Off mode is on"
#  else
#   define PPRS_FLASH_POWER_OFF "\t"
#  endif
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif

# else
#  define PPRS_FLASH_TITLE "A Flash is not connected"
#  define PPRS_FLASH_POWER_OFF "\t"
#  define PPRS_FLASH_POWER_DOWN "\n"
# endif /* dg_configFLASH_CONNECTED_TO */


# undef PPRS_THEME
# undef PPRS_1V8_TITLE
# undef PPRS_1V8_ACTIVE
# undef PPRS_1V8_SLEEP
# undef PPRS_1V8P_TITLE
# undef PPRS_1V8P_ACTIVE
# undef PPRS_1V8P_SLEEP
# undef PPRS_FLASH_TITLE
# undef PPRS_FLASH_POWER_OFF
# undef PPRS_FLASH_POWER_DOWN
#endif /* PRINT_POWER_RAIL_SETUP */

#if (dg_configIMAGE_SETUP == PRODUCTION_MODE)
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#   pragma message"Production mode build: code will be built for RAM execution!"
# endif
#else /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OTP)
#  pragma message"Development mode build: code will be built for OTP execution!"
# endif


#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) && \
    (dg_configFLASH_CONNECTED_TO == FLASH_IS_NOT_CONNECTED)
# error "Building for QSPI Flash code but a Flash is not connected!"
#endif



# if (dg_configREDUCE_RETAINED_CODE) && (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#  error "dg_configREDUCE_RETAINED_CODE option is applicable only for embedded flash."
# endif

# if dg_configENABLE_MTB
#  if (dg_configMTB_MASK > 12)
#   error "dg_configMTB_MASK setting exceeds the maximum allowed value!"
#  endif
# endif /* dg_configENABLE_MTB */

# if dg_configPMU_ADAPTER
#  if (dg_configPOWER_1V8_ACTIVE == 0) && (dg_configPOWER_1V8_SLEEP == 1)
#   error "1V8 rail set to off during active and on during sleep..."
#  endif
# endif /* dg_configPMU_ADAPTER */

# if (dg_configHW_FCU_WAIT_CYCLES_MODE == 2) && (dg_configIMAGE_SETUP == PRODUCTION_MODE)
#   pragma message "dg_configHW_FCU_WAIT_CYCLES_MODE is set to 2. It is not recommended for production code since extra sanity code is added. Please refer to documentation for more details"
# endif /* dg_configHW_FCU_WAIT_CYCLES_MODE */


#  if (dg_configFLASH_POWER_DOWN == 1)
#   if defined(dg_configFLASH_POWER_OFF) && (dg_configFLASH_POWER_OFF == 1)
#    error "dg_configFLASH_POWER_DOWN and dg_configFLASH_POWER_OFF cannot be both set to 1"
#   endif
#  endif /* dg_configFLASH_POWER_DOWN */


# if (dg_configLOG_BLE_STACK_MEM_USAGE == 1) && (dg_configIMAGE_SETUP != DEVELOPMENT_MODE)
#  error "dg_configLOG_BLE_STACK_MEM_USAGE must not be set when building for PRODUCTION_MODE "
# endif

#endif /* dg_configIMAGE_SETUP */



# if (dg_configNVPARAM_ADAPTER)
#  if (dg_configNVMS_ADAPTER != 1)
#   pragma message "NVMS adapter is mandatory to make use of NVPARAM and will be enabled silently"
#   undef dg_configNVMS_ADAPTER
#   define dg_configNVMS_ADAPTER 1
#  endif
# endif

#if (dg_configRF_ENABLE_RECALIBRATION && !defined(CONFIG_USE_BLE))
#error "RF re-calibration is out of scope for non BLE application"
#endif

#if defined(CONFIG_RETARGET) && (dg_configUSE_HW_UART == 0)
#error "Enabling CONFIG_RETARGET requires to set dg_configUSE_HW_UART"
#endif


#if defined(OS_PRESENT)
# if (dg_configUSE_SYS_ADC == 0)
#   warning "The SYS_ADC is a prerequisite for enabling both the RC clocks calibration and the RF calibration. Disabling this feature is not recommended."
# endif
#endif /* OS_PRESENT */

