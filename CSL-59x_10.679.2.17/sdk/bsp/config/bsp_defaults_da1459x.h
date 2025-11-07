/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_CONFIG_DEFAULTS
 * \{
 */
/**
 ****************************************************************************************
 *
 * @file bsp_defaults_da1459x.h
 *
 * @brief Board Support Package. Device-specific system configuration default values.
 *
 * Copyright (C) 2020-2024 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef BSP_DEFAULTS_DA1459X_H_
#define BSP_DEFAULTS_DA1459X_H_


/* Deprecated configuration options must not be defined by the application. */

#define DG_CONFIG_NA_59X_MSG DG_CONFIG_NOT_APPLICABLE_MSG " for the DA1459x device family."
#define DG_CONFIG_NA_59X_FORCE_ZERO_MSG DG_CONFIG_NA_59X_MSG " Forcing to 0 (not used)."
#define DG_CONFIG_TIMER_NA_MSG DG_CONFIG_NA_59X_MSG " Use the generic dg_configUSE_HW_TIMER instead."

#ifdef dg_configTim1Prescaler
# pragma message "dg_configTim1Prescaler" DG_CONFIG_NA_59X_MSG
# undef  dg_configTim1Prescaler
#endif

#ifdef dg_configTim1PrescalerBitRange
# pragma message "dg_configTim1PrescalerBitRange" DG_CONFIG_NA_59X_MSG
# undef dg_configTim1PrescalerBitRange
#endif

#ifdef dg_configEXT_CRYSTAL_FREQ
# pragma message "dg_configEXT_CRYSTAL_FREQ" DG_CONFIG_NA_59X_MSG
# undef  dg_configEXT_CRYSTAL_FREQ
#endif

#ifdef dg_configUSER_CAN_USE_TIMER1
# pragma message "dg_configUSER_CAN_USE_TIMER1" DG_CONFIG_NA_59X_MSG
# undef  dg_configUSER_CAN_USE_TIMER1
#endif

#if (dg_configUSE_HW_RF != 0)
# pragma message "dg_configUSE_HW_RF" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_RF
# define dg_configUSE_HW_RF                             0
#endif

#if (dg_configUSE_HW_COEX != 0)
# pragma message "dg_configUSE_HW_COEX" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_COEX
# define dg_configUSE_HW_COEX                           0
#endif

#if (dg_configUSE_HW_ERM != 0)
# pragma message "dg_configUSE_HW_ERM" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_ERM
# define dg_configUSE_HW_ERM                            0
#endif

#if (dg_configUSE_HW_LCDC != 0)
# pragma message "dg_configUSE_HW_LCDC" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_LCDC
# define dg_configUSE_HW_LCDC                           0
#endif

#if (dg_configUSE_HW_LRA != 0)
# pragma message "dg_configUSE_HW_LRA" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_LRA
# define dg_configUSE_HW_LRA                            0
#endif

#if (dg_configUSE_HW_OTPC != 0)
# pragma message "dg_configUSE_HW_OTPC" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_OTPC
# define dg_configUSE_HW_OTPC                           0
#endif

#if (dg_configUSE_IF_PDM != 0)
# pragma message "dg_configUSE_IF_PDM" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_IF_PDM
# define dg_configUSE_IF_PDM                            0
#endif

#if (dg_configUSE_HW_IRGEN != 0)
# pragma message "dg_configUSE_HW_TEMPSENS" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_IRGEN
# define dg_configUSE_HW_IRGEN                          0
#endif

#if (dg_configUSE_HW_SOC != 0)
# pragma message "dg_configUSE_HW_SOC" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_SOC
# define dg_configUSE_HW_SOC                            0
#endif

#if (dg_configUSE_HW_TRNG != 0)
# pragma message "dg_configUSE_HW_TRNG" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_TRNG
# define dg_configUSE_HW_TRNG                           0
#endif

#if (dg_configUSE_HW_TIMER0 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER0
# define dg_configUSE_HW_TIMER0                         0
#endif

#if (dg_configUSE_HW_TIMER1 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER1
# define dg_configUSE_HW_TIMER1                         0
#endif

#if (dg_configUSE_HW_TIMER2 != 0)
# pragma message "dg_configUSE_HW_TIMERX" DG_CONFIG_TIMER_NA_MSG
# undef  dg_configUSE_HW_TIMER2
# define dg_configUSE_HW_TIMER2                         0
#endif

#if (dg_configUSE_HW_USB != 0)
# pragma message "dg_configUSE_HW_USB" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_USB
# define dg_configUSE_HW_USB                            0
#endif

#if (dg_configUSE_HW_USB_CHARGER != 0)
# pragma message "dg_configUSE_HW_USB_CHARGER" DG_CONFIG_NA_59X_FORCE_ZERO_MSG
# undef  dg_configUSE_HW_USB_CHARGER
# define dg_configUSE_HW_USB_CHARGER                    0
#endif

/* ------------------------------- Peripherals -------------------------------------------------- */

/**
 * \addtogroup PERIPHERALS_590 Peripherals for DA1459x
 *
 * \brief Peripheral Selection for the DA1459x Device Family
 *
 * When enabled the specific low level driver is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Driver                         | Setting                                | Default option
   ------------------------------ | -------------------------------------- | :------------------:
   AES                            | dg_configUSE_HW_AES                    | 0
   Cache Controller               | dg_configUSE_HW_CACHE                  | 1
   HW charger                     | dg_configUSE_HW_CHARGER                | 0
   Clock driver                   | dg_configUSE_HW_CLK                    | 1
   Clock and Power Manager        | dg_configUSE_HW_CPM                    | 1
   Direct Memory Access           | dg_configUSE_HW_DMA                    | 1
   General Purpose A-D  Converter | dg_configUSE_HW_GPADC                  | 1
   General Purpose I/O            | dg_configUSE_HW_GPIO                   | 1
   HASH                           | dg_configUSE_HW_HASH                   | 0
   Inter-Integrated Circuit       | dg_configUSE_HW_I2C                    | 0
   Memory Protection Unit         | dg_configUSE_HW_MPU                    | 0
   PCM                            | dg_configUSE_HW_PCM                    | 0
   Domain Driver                  | dg_configUSE_HW_PD                     | 1
   Power Domains Controller       | dg_configUSE_HW_PDC                    | 1
   PDM                            | dg_configUSE_HW_PDM                    | 0
   Power Manager                  | dg_configUSE_HW_PMU                    | 1
   QSPI controller                | dg_configUSE_HW_QSPI                   | 1
   QSPI2 controller               | dg_configUSE_HW_QSPI2                  | 0
   Real Time Clock                | dg_configUSE_HW_RTC                    | 1
   ΣΔ Analog-Digital Converter    | dg_configUSE_HW_SDADC                  | 0
   Sensor Node Controller         | dg_configUSE_HW_SENSOR_NODE            | 0
   Motor Controller               | dg_configUSE_HW_SMOTOR                 | 0
   Serial Peripheral Interface    | dg_configUSE_HW_SPI                    | 0
   SRC                            | dg_configUSE_HW_SRC                    | 0
   System                         | dg_configUSE_HW_SYS                    | 1
   Timer                          | dg_configUSE_HW_TIMER                  | 1
   UART                           | dg_configUSE_HW_UART                   | 1
   Wakeup timer                   | dg_configUSE_HW_WKUP                   | 1
   Quadrature Decoder             | dg_configUSE_HW_QUAD                   | 0
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------- Peripherals (hw_*) selection -------------------------------- */

#ifndef dg_configUSE_HW_AES
#define dg_configUSE_HW_AES                             (0)
#endif

#ifndef dg_configUSE_HW_CACHE
#define dg_configUSE_HW_CACHE                           (1)
#endif

#ifndef dg_configUSE_HW_CHARGER
#define dg_configUSE_HW_CHARGER                         (0)
#endif

#ifndef dg_configUSE_HW_CLK
#define dg_configUSE_HW_CLK                             (1)
#endif

#ifndef dg_configUSE_HW_CPM
#define dg_configUSE_HW_CPM                             (1)
#endif

#ifndef dg_configUSE_HW_DMA
#define dg_configUSE_HW_DMA                             (1)
#endif

#ifndef dg_configUSE_HW_GPADC
#define dg_configUSE_HW_GPADC                           (1)
#endif

#ifndef dg_configUSE_HW_GPIO
#define dg_configUSE_HW_GPIO                            (1)
#endif

#ifndef dg_configUSE_HW_HASH
#define dg_configUSE_HW_HASH                            (0)
#endif

#ifndef dg_configUSE_HW_I2C
#define dg_configUSE_HW_I2C                             (0)
#endif

#ifndef dg_configUSE_HW_MPU
#define dg_configUSE_HW_MPU                             (0)
#endif

#ifndef dg_configUSE_HW_PCM
#define dg_configUSE_HW_PCM                             (0)
#endif

#ifndef dg_configUSE_HW_PD
#define dg_configUSE_HW_PD                              (1)
#endif

#ifndef dg_configUSE_HW_PDC
#define dg_configUSE_HW_PDC                             (1)
#endif

#ifndef dg_configUSE_HW_PDM
#define dg_configUSE_HW_PDM                             (0)
#endif

#ifndef dg_configUSE_HW_PMU
#define dg_configUSE_HW_PMU                             (1)
#endif

#ifndef dg_configUSE_HW_QSPI
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#define dg_configUSE_HW_QSPI                            (1)
#elif ( (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE) )
#define dg_configUSE_HW_QSPI                            (0)
#endif
#endif

#ifndef dg_configUSE_HW_QUAD
#define dg_configUSE_HW_QUAD                            (0)
#endif

#if (dg_configQSPI_AUTOMODE_ENABLE == 1) && (dg_configUSE_HW_QSPI == 0)
# error "Enabling dg_configQSPI_AUTOMODE_ENABLE requires to set dg_configUSE_HW_QSPI"
#endif

#if (dg_configQSPI_FLASH_AUTODETECT == 1) && (dg_configQSPI_FLASH_CONFIG_VERIFY == 1)
# error "dg_configQSPI_FLASH_AUTODETECT and dg_configQSPI_FLASH_CONFIG_VERIFY are mutually exclusive"
#endif

#ifndef dg_configUSE_HW_QSPI2
#define dg_configUSE_HW_QSPI2                           (0)
#endif

#ifndef dg_configUSE_HW_RTC
#define dg_configUSE_HW_RTC                             (1)
#endif

#ifndef dg_configUSE_HW_SDADC
#define dg_configUSE_HW_SDADC                           (0)
#endif


#ifndef dg_configUSE_HW_SMOTOR
#define dg_configUSE_HW_SMOTOR                          (0)
#endif

#ifndef dg_configUSE_HW_SPI
#define dg_configUSE_HW_SPI                             (0)
#endif

#ifndef dg_configUSE_HW_SRC
#define dg_configUSE_HW_SRC                             (0)
#endif

#ifndef dg_configUSE_HW_SYS
#define dg_configUSE_HW_SYS                             (1)
#endif

#ifndef dg_configUSE_HW_TIMER
#define dg_configUSE_HW_TIMER                           (1)
#endif

#ifndef dg_configUSE_HW_UART
#define dg_configUSE_HW_UART                            (1)
#endif

#ifndef dg_configUSE_HW_WKUP
#define dg_configUSE_HW_WKUP                            (1)
#endif

#ifndef dg_configUSE_HW_FCU
#define dg_configUSE_HW_FCU                             (1)
#endif

#ifndef dg_configGPADC_DMA_SUPPORT
# if dg_configUSE_HW_GPADC
#  define dg_configGPADC_DMA_SUPPORT                    dg_configUSE_HW_DMA
# else
#  define dg_configGPADC_DMA_SUPPORT                    (0)
# endif
#endif

#if (dg_configGPADC_DMA_SUPPORT == 1) && ((dg_configUSE_HW_GPADC == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for GPADC needs both dg_configUSE_HW_GPADC and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configSDADC_DMA_SUPPORT
# if dg_configUSE_HW_SDADC
#  define dg_configSDADC_DMA_SUPPORT                    dg_configUSE_HW_DMA
# else
#  define dg_configSDADC_DMA_SUPPORT                    (0)
# endif
#endif

#if (dg_configSDADC_DMA_SUPPORT == 1) && ((dg_configUSE_HW_SDADC == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for SDADC needs both dg_configUSE_HW_SDADC and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configI2C_DMA_SUPPORT
# if dg_configUSE_HW_I2C
#  define dg_configI2C_DMA_SUPPORT                      dg_configUSE_HW_DMA
# else
#  define dg_configI2C_DMA_SUPPORT                      (0)
# endif
#endif

#if (dg_configI2C_DMA_SUPPORT == 1) && ((dg_configUSE_HW_I2C == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for I2C needs both dg_configUSE_HW_I2C and dg_configUSE_HW_DMA to be 1"
#endif

#ifndef dg_configSPI_DMA_SUPPORT
# if dg_configUSE_HW_SPI
#  define dg_configSPI_DMA_SUPPORT                      dg_configUSE_HW_DMA
# else
#  define dg_configSPI_DMA_SUPPORT                      (0)
# endif
#endif

#if (dg_configSPI_DMA_SUPPORT == 1) && ((dg_configUSE_HW_SPI == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for SPI needs both dg_configUSE_HW_SPI and dg_configUSE_HW_DMA to be 1"
#endif

/**
 * \}
 */

/* ------------------------------- Clock Settings ----------------------------------------------- */

/**
 * \addtogroup CLOCK_SETTINGS
 *
 * \{
 */

#if (dg_configUSE_LP_CLK != LP_CLK_32000) && (dg_configUSE_LP_CLK != LP_CLK_32768) && (dg_configUSE_LP_CLK != LP_CLK_RCX) && (dg_configUSE_LP_CLK != LP_CLK_ANY)
#error "dg_configUSE_LP_CLK has invalid setting"
#endif

#if (dg_configUSE_LP_CLK == LP_CLK_ANY)
#pragma message "In order to support the option LP_CLK_ANY for the low-power clock source, "\
                "some configuration options MUST be defined by the application, including "\
                "dg_configMIN_SLEEP_TIME, dg_configIMAGE_COPY_TIME, dg_configPM_MAX_ADAPTER_DEFER_TIME, "\
                "TICK_PERIOD, BLE_WUP_LATENCY, sleep_duration_in_lp_cycles and rwip_check_wakeup_boundary. "
#pragma message "Additionally, some device-specific configuration options MUST be defined by the application, "\
                "including dg_configXTAL32K_FREQ."
#endif

#ifndef dg_configXTAL32M_FREQ
#define dg_configXTAL32M_FREQ                           (32000000)
#endif

#ifndef dg_configRC32M_FREQ
#define dg_configRC32M_FREQ                             (32000000)
#endif

#ifndef dg_configRC32M_FREQ_MIN
#define dg_configRC32M_FREQ_MIN                         (30000000)
#endif

#ifndef dg_configDIVN_FREQ
#define dg_configDIVN_FREQ                              (32000000)
#endif

#ifndef dg_configDBLR64M_FREQ
#define dg_configDBLR64M_FREQ                           (64000000)
#endif

#if dg_configUSE_LP_CLK == LP_CLK_32768
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (32768)
#elif dg_configUSE_LP_CLK == LP_CLK_32000
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (32000)
#elif dg_configUSE_LP_CLK == LP_CLK_RCX
# undef dg_configXTAL32K_FREQ
# define dg_configXTAL32K_FREQ                          (0)
#endif

/**
 * \brief Value of the RC32K oscillator frequency in Hz
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configRC32K_FREQ
#define dg_configRC32K_FREQ                             (32000)
#endif

/**
 * \brief Acceptable clock tick drift (in parts per million) for the Low-power clock
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configLP_CLK_DRIFT
# if dg_configUSE_LP_CLK == LP_CLK_RCX
#  define dg_configLP_CLK_DRIFT                         (500) //ppm
# else
#  define dg_configLP_CLK_DRIFT                         (50) //ppm
# endif
#endif

/**
 * \brief Time needed for the settling of the LP clock, in msec.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configXTAL32K_SETTLE_TIME
# if dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG
#  define dg_configXTAL32K_SETTLE_TIME                  (8000)
# else
#  define dg_configXTAL32K_SETTLE_TIME                  (1000)
# endif
#endif

/**
 * \brief XTAL32M maximum expected settling time
 *
 * Time (in usec) reserved by the system for the settling of XTAL32M (from enabling
 * XTAL32M till the XTAL32M_READY bit is set). This defines the start value to be
 * set to the XtalRdy counter, which basically determines how soon the XTALR32M_RDY
 * IRQ will fire every time Xtal32M is enabled (e.g. on each wakeup of the system).
 *
 * \note If dg_configENABLE_XTAL32M_ON_WAKEUP == 1, this time is taken into account by the power
 *       manager when setting up the sleep time of the system (i.e. the system will make sure that
 *       it wakes up a little sooner than actually required, so that XTAL32M has settled by then).
 *       Thus, using a very high value will reduce the sleep time of the system considerably!
 *
 * \note If set to zero, the maximum settling time will be calculated automatically at startup.
 */
#ifndef dg_configXTAL32M_SETTLE_TIME_IN_USEC
#define dg_configXTAL32M_SETTLE_TIME_IN_USEC            (0x0)
#endif

/**
 * \brief Enable XTAL32M upon system wake-up
 *
 * If set to 1 the PDC will enable XTAL32M when it wakes-up M33
 *
 */
#ifndef dg_configENABLE_XTAL32M_ON_WAKEUP
#define dg_configENABLE_XTAL32M_ON_WAKEUP               (1)
#endif

/**
 * \brief The time in us needed to wake-up in normal wake-up mode.
 *
 * This is the maximum time needed to wake-up the chip and start executing code
 * in normal wake-up mode.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_NORMAL                          (300)

/**
 * \brief The time in us needed to wake up in fast wake up mode.
 *
 * This is the maximum time needed to wake-up the chip and start executing code.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#define dg_configWAKEUP_FAST                            (12)

/**
 * \brief RC32M trimming default settings
 */
#ifndef dg_configDEFAULT_CLK_RC32M_REG_RC32M_BIAS__VALUE
#define dg_configDEFAULT_CLK_RC32M_REG_RC32M_BIAS__VALUE                (0xD)
#endif

#ifndef dg_configDEFAULT_CLK_RC32M_REG_RC32M_RANGE__VALUE
#define dg_configDEFAULT_CLK_RC32M_REG_RC32M_RANGE__VALUE               (0x1)
#endif

#ifndef dg_configDEFAULT_CLK_RC32M_REG_RC32M_COSC__VALUE
#define dg_configDEFAULT_CLK_RC32M_REG_RC32M_COSC__VALUE                (0x4)
#endif

/**
 * \brief XTAL32M trimming default settings
 */
#ifndef dg_configDEFAULT_XTAL32M_TRIM_REG__XTAL32M_TRIM__VALUE
#define dg_configDEFAULT_XTAL32M_TRIM_REG__XTAL32M_TRIM__VALUE         (0x45)
#endif

#ifndef dg_configUSE_CLOCK_MGR
# ifdef OS_BAREMETAL
#  define dg_configUSE_CLOCK_MGR                        (0)
# elif defined(OS_FREERTOS)
#  define dg_configUSE_CLOCK_MGR                        (1)
# endif
#endif

/**
 * \}
 */

/* ------------------------------- System configuration settings -------------------------------- */

/**
 * \addtogroup SYSTEM_CONFIGURATION_SETTINGS
 *
 * \{
 */

#if (dg_configUSE_WDOG == 0) && defined(dg_configWDOG_IDLE_RESET_VALUE)
# pragma message "dg_configWDOG_IDLE_RESET_VALUE is ignored. Maximum watchdog value will be used."
# undef dg_configWDOG_IDLE_RESET_VALUE
#endif

/**
 * \brief Reset value for Watchdog when system is idle.
 */
#ifndef dg_configWDOG_IDLE_RESET_VALUE
#define dg_configWDOG_IDLE_RESET_VALUE  (SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >> SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Pos)
#endif

/**
 * \brief Maximum watchdog tasks
 *
 * Maximum number of tasks that the Watchdog Service can monitor.
 * It can be larger than needed (up to 32 minus the system-tasks serviced),
 * at the expense of increased Retention Memory requirement.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configWDOG_MAX_TASKS_CNT
#define dg_configWDOG_MAX_TASKS_CNT                     (8)
#endif

/**
 * \brief Watchdog guards the idle-task in OS applications
 *
 * If set to 1, the idle task will register with the Watchdog System Service
 * at system initialization.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configWDOG_GUARD_IDLE_TASK
#define dg_configWDOG_GUARD_IDLE_TASK                   (1)
#endif

/**
 * \brief System debug logging protection mechanism.
 *
 *        When set to 1, a mutual exclusion mechanism is employed and any ongoing printing activity
 *        will not be interpolated by another printing attempt that is initiated from another OS task.
 *
 *        \note The debug logging protection mechanism is not available in Baremetal build configurations
 *
 *        In particular, the libC standard output functions are overridden in sdk/bsp/startup/config.c by a:
 *              - custom printf, in case the debug logging string contains:
 *                     -# only characters, e.g. printf("a b c d e f"); or any char-only string prefixed
 *                        with newline char ("\n"), e.g. printf("\na b c d e f");
 *                     -# any format specifiers (subsequences beginning with %), e.g. printf("a %d c\n",2);
 *              - custom puts, in case the debug logging string contains one or more newline chars ("\n")
 *                      in the end but no format specifiers, e.g. printf("\na b c d e f\n"); or printf("f\n");
 *              - custom putchar, in case the debug logging string is of one only character, even if it is
 *                      an escaping one, e.g. printf("#"); or printf("\n");
 *
 *        When set to 0, contenting printing attempts initiated from different contexts may end up in a race
 *        condition that can result in a disordered and unreadable serial output.
 *
 *        The debug logging protection mechanism cannot be employed if the console service is enabled.
 *
 *        \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYS_DBG_LOG_PROTECTION
# if (!dg_configUSE_CONSOLE)
#       define dg_configSYS_DBG_LOG_PROTECTION                  (1)
# endif
#endif

/* The system debug logging protection mechanism does not support M33 baremetal build configurations. */
#if (dg_configSYS_DBG_LOG_PROTECTION == 1)
# ifdef OS_BAREMETAL
#  undef dg_configSYS_DBG_LOG_PROTECTION
#       define dg_configSYS_DBG_LOG_PROTECTION                  (0)
# endif
#endif

/* Maximum number of characters of a debug logging string that can be printed at a time. A set of chars is
 * also reserved for printing the processing unit prefix where the string originated from. If the string
 * is greater in length than the maximum characters minus the prefix then an error message is displayed
 * instead. This prefix related limitation does not apply for single processing unit applications. */
#if dg_configSYS_DBG_LOG_PROTECTION
# define dg_configSYS_DBG_LOG_MAX_SIZE                  200
#endif

/**
 * \brief Changes the amount of code in RAM when XIP memory is embedded flash.
 *
 * When set to 1, the embedded flash (EFLASH) is put to sleep right before the SYS CPU goes to sleep and wakes up
 * when the SYS CPU is awakened. In this mode, the EFLASH remains active while the SYS CPU is awake, and the number of functions that need to be retained in RAM is limited.
 *
 * When set to 0, the EFLASH is put to sleep when the power manager decides that the SYS CPU can go to sleep and wakes up
 * during the execution of the wake-up sequence. Since the embedded flash is in sleep mode while the SYS CPU is active, more functions need to be retained in SysRAM.
 *
 * \note dg_configREDUCE_RETAINED_CODE cannot be set to 1 when code is executed from QSPI.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configREDUCE_RETAINED_CODE
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
#define dg_configREDUCE_RETAINED_CODE                   (1)
#else
#define dg_configREDUCE_RETAINED_CODE                   (0)
#endif
#endif


/*
 * \brief When set to 1, PD_COM is enabled by power manager when Cortex-M33 master is active. This allows
 * the master to have access to I2C, SPI, UART and SDADC interfaces, GPIO multiplexing and Timer4.
 * When set to 0, PD_COM can be enabled by the adapters or the application. PDC can also be configured to
 * enable PD_COM by setting the appropriate flag in the PDC LUT entry.
 *
 * \note In case this macro is disabled, it is application responsibility to apply PD_COM trim and preferred settings
 *       after enabling the power domain.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE
#define dg_configPM_ENABLES_PD_COM_WHILE_ACTIVE             (1)
#endif

/**
 * \}
 */

/* -------------------------------------- Flash settings ---------------------------------------- */

/**
 * \addtogroup FLASH_SETTINGS
 *
 * \{
 */

/**
 * \brief The rail from which the external Flash is powered, if an external Flash is used.
 *
 * - FLASH_IS_NOT_CONNECTED
 * - FLASH_CONNECTED_TO_1V8
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
# ifndef dg_configFLASH_CONNECTED_TO
#  error "dg_configFLASH_CONNECTED_TO is not defined!"
# endif
#endif

/**
 * \brief When set to 1, the 1V8 rail is powered, when the system is in active state.
 * When set to 2 the rail configuration will be defined by the application
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configPOWER_1V8_ACTIVE
# define dg_configPOWER_1V8_ACTIVE                      (2)
#else
# if (dg_configPOWER_1V8_ACTIVE == 0) && (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8)
#  error "Flash is connected to the 1V8 rail but the rail is turned off. Please do not set dg_configPOWER_1V8_ACTIVE to 0."
# endif
#endif

/**
 * \brief When set to 1, the 1V8 is powered during sleep.
 * When set to 2 the rail configuration will be defined by the application
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configPOWER_1V8_SLEEP
#define dg_configPOWER_1V8_SLEEP                        (2)
#endif

/**
 * \brief When set to 1, the system activates the QSPI flash power-down mode when it enters sleep
 *        in order to minimize overall power consumption.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_FLASH_POWER_DOWN
#define dg_configQSPI_FLASH_POWER_DOWN                  (1)
#endif

/**
 * \brief Set the Drive Strength of the QSPI Controller
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 * \sa HW_QSPI_DRIVE_CURRENT
 */
#ifndef dg_configQSPI_DRIVE_CURRENT
#define dg_configQSPI_DRIVE_CURRENT                     (HW_QSPI_DRIVE_CURRENT_4)
#endif

/**
 * \brief Set the Slew Rate of the QSPI Controller
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 * \sa HW_QSPI_SLEW_RATE
 */
#ifndef dg_configQSPI_SLEW_RATE
#define dg_configQSPI_SLEW_RATE                         (HW_QSPI_SLEW_RATE_0)
#endif

/**
 * \brief Select whether the QSPI Flash memory will be erased in Auto or in Manual Access Mode.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dgconfigQSPI_ERASE_IN_AUTOMODE
#define dgconfigQSPI_ERASE_IN_AUTOMODE                 (1)
#endif

#if (dg_configQSPI_FLASH_AUTODETECT == 0)

/**
 * \brief The Flash Driver header file to include
 *
 * The header file must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_FLASH_HEADER_FILE
#define dg_configQSPI_FLASH_HEADER_FILE                 "qspi_mx25u3235_v2.h"
#endif /* dg_configQSPI_FLASH_HEADER_FILE */

/**
 * \brief The Flash Driver configuration structure
 *
 * The configuration structure must be in the include path of the compiler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_FLASH_CONFIG
#define dg_configQSPI_FLASH_CONFIG                      qspi_mx25u3235_cfg
#endif /* dg_configQSPI_FLASH_CONFIG */

/**
 * \brief Flash device configuration verification.
 *
 * When set to 1, the Flash device id configuration is checked against the JEDEC ID read
 * from the controller.
 *
 * Applicable only when flash auto detection is not enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_FLASH_CONFIG_VERIFY
#define dg_configQSPI_FLASH_CONFIG_VERIFY               (0)
#endif
#endif /* (dg_configQSPI_FLASH_AUTODETECT == 0) */

/**
 * \brief EFLASH memory mass erase time in usec
 *
 * EFLASH memory mass erase time. Minimum: 80000 usec Maximum: 160000 usec
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEFLASH_MASS_ERASE_TIME
#define dg_configEFLASH_MASS_ERASE_TIME 160000
#endif

/**
 * \brief EFLASH memory page erase time in usec
 *
 * EFLASH memory page erase time. Minimum: 80000 usec Maximum: 160000 usec
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEFLASH_PAGE_ERASE_TIME
#define dg_configEFLASH_PAGE_ERASE_TIME 160000
#endif

/**
 * \brief EFLASH memory one word write time in usec
 *
 * EFLASH memory one word write time. Minimum: 8 usec Maximum: 16 usec
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEFLASH_WORD_WRITE_TIME
#define dg_configEFLASH_WORD_WRITE_TIME 16
#endif

/**
 * \brief Activate EFLASH optimum wait cycle functionality
 *
 * EFLASH wait cycles depend on AHB bus frequency and VDD voltage level. Enabling this functionality
 * EFLASH is configured with the correct wait cycles.
 *
 * When set to 0, the EFLASH wait cycles are configured to the default and secure option of 2 wait cycles,
 * regardless of the AHB frequency and VDD voltage level.
 * When set to 1, the EFLASH wait cycles are dynamically configured based on AHB bus frequency and VDD voltage level.
 * When set to 2, in addition to dynamic configuration of wait cycles in EFLASH, like when is set to 1, the AHB frequency
 * and VDD voltage level are monitored in order to indicate if there was a change in the above parameters without
 * triggering wait cycles change. In this mode, extra code will be added in order to monitor the above settings and in case
 * of an untraced change an ASSERT_WARNING will be triggered.
 *
 * \note It is not recommended to set dg_configHW_FCU_WAIT_CYCLES_MODE to 2 in production code because it will add extra code
 *       and will trigger an ASSERT_WARNING when (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) when an untraced change is detected.
 *       It is recommended to use this setting during development phase.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configHW_FCU_WAIT_CYCLES_MODE
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#define dg_configHW_FCU_WAIT_CYCLES_MODE            (2)
#else
#define dg_configHW_FCU_WAIT_CYCLES_MODE            (1)
#endif /* dg_configIMAGE_SETUP */
#endif /* dg_configHW_FCU_WAIT_CYCLES_MODE */

/**
 * \}
 */

/* ----------------------------------- Charger settings ----------------------------------------- */

/**
 * \addtogroup CHARGER_SETTINGS Charger configuration settings
 *
 * \{
 */

/**
 * \brief When set to 1, State of Charge function is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_SOC
#define dg_configUSE_SOC                                (0)
#endif

/**
 * \}
 */

/* ----------------------------------- UART settings -------------------------------------------- */

/**
 * \addtogroup UART_SETTINGS
 *
 * \{
 */
#ifndef dg_configUART_DMA_SUPPORT
# if dg_configUSE_HW_UART
#  define dg_configUART_DMA_SUPPORT                     dg_configUSE_HW_DMA
# else
#  define dg_configUART_DMA_SUPPORT                     (0)
# endif
#endif

#if (dg_configUART_DMA_SUPPORT == 1) && ((dg_configUSE_HW_UART == 0) || (dg_configUSE_HW_DMA == 0))
# pragma message "DMA support for UART needs both dg_configUSE_HW_UART and dg_configUSE_HW_DMA to be 1"
#endif

#if (dg_configUART_RX_CIRCULAR_DMA == 1) && (dg_configUART_DMA_SUPPORT == 0)
#error "dg_configUART_RX_CIRCULAR_DMA requires dg_configUART_DMA_SUPPORT to be enabled!"
#endif

/**
 * \}
 */

/* ----------------------------------- MPU settings -------------------------------------------- */

/**
 * \addtogroup MPU_SETTINGS
 *
 * \{
 */

#if defined(CONFIG_USE_BLE)
/**
 * \brief MPU region for CMAC protection
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configCMAC_PROTECT_REGION
#define dg_configCMAC_PROTECT_REGION                    MPU_REGION_6
#endif
#endif /* CONFIG_USE_BLE */

/**
 * \brief MPU region for IVT protection
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configIVT_PROTECT_REGION
#define dg_configIVT_PROTECT_REGION                     MPU_REGION_7
#endif

/**
 * \}
 */

/* ----------------------------------- Asymmetric SUOTA settings -------------------------------- */

/**
 * \addtogroup ASUOTA_SETTINGS
 *
 * \{
 */

/**
 * \brief Asymmetric SUOTA firmware magic word
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configASYMMETRIC_SUOTA_MAGIC
#define dg_configASYMMETRIC_SUOTA_MAGIC                 "ASYM_SUOTA_FW"
#endif

/**
 * \}
 */

/*
 */
/*------------------------------------ BOARDS DEFINITIONS ----------------------------------------*/

/**
 * \brief Set the board that is used.
 */
#ifndef dg_configUSE_BOARD
# include "boards/brd_prodk_da1459x.h"
# define dg_configUSE_BOARD
#endif


#endif /* BSP_DEFAULTS_DA1459X_H_ */
/**
 * \}
 * \}
 */
