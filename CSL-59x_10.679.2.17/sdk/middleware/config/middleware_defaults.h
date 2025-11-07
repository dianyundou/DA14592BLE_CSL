/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup MIDDLEWARE_CONFIG_DEFAULTS
 *
 * \brief Middleware default configuration values
 *
 * The following tags are used to describe the type of each configuration option.
 *
 * - **\bsp_config_option_build**        : To be changed only in the build configuration
 *                                                of the project ("Defined symbols -D" in the
 *                                                preprocessor options).
 *
 * - **\bsp_config_option_app**          : To be changed only in the custom_config*.h
 *                                                project files.
 *
 * - **\bsp_config_option_expert_only**  : To be changed only by an expert user.
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file middleware_defaults.h
 *
 * @brief Middleware. System Configuration file default values.
 *
 * Copyright (C) 2018-2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef MIDDLEWARE_DEFAULTS_H_
#define MIDDLEWARE_DEFAULTS_H_

/**
 * \addtogroup ADAPTER_SELECTION Adapters enabled by default
 *
 * \brief Adapter selection
 *
 * When enabled the specific adapter is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Adapter                        | Setting                                | Default option
   ------------------------------ | -------------------------------------- | :------------------:
   Table not yet fixed | dg_configXXXXX_ADAPTER                   | 1
 *
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */


/* -------------------------------- Adapters (ad_*) selection -------------------------------- */

#ifndef dg_configFLASH_ADAPTER
#define dg_configFLASH_ADAPTER                  (1)
#endif

#ifndef dg_configI2C_ADAPTER
#define dg_configI2C_ADAPTER                    (0)
#endif

#ifndef dg_configNVMS_ADAPTER
#define dg_configNVMS_ADAPTER                   (1)
#endif

#ifndef dg_configNVMS_FLASH_CACHE
#define dg_configNVMS_FLASH_CACHE               (0)
#endif

#ifndef dg_configNVMS_VES
#define dg_configNVMS_VES                       (1)
#endif

#ifndef dg_configSPI_ADAPTER
#define dg_configSPI_ADAPTER                    (0)
#endif

#ifndef dg_configUART_ADAPTER
#define dg_configUART_ADAPTER                   (0)
#endif

#ifndef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (0)
#endif

#ifndef dg_configSDADC_ADAPTER
#define dg_configSDADC_ADAPTER                  (0)
#endif

#ifdef dg_configTEMPSENS_ADAPTER
#error "Configuration option dg_configTEMPSENS_ADAPTER  is no longer supported"
#endif

#ifdef dg_configBATTERY_ADAPTER
#error "Configuration option dg_configBATTERY_ADAPTER  is no longer supported"
#endif

#ifndef dg_configNVPARAM_ADAPTER
#define dg_configNVPARAM_ADAPTER                (0)
#endif

#ifndef dg_configNVPARAM_APP_AREA
#define dg_configNVPARAM_APP_AREA               (0)
#endif

#ifndef dg_configCRYPTO_ADAPTER
#define dg_configCRYPTO_ADAPTER                 (0)
#endif

#if dg_configCRYPTO_ADAPTER
#if !dg_configUSE_HW_AES && !dg_configUSE_HW_HASH
#error "The Crypto adapter requires hw_aes or/and hw_hash"
#endif
#endif

#ifndef dg_configKEYBOARD_SCANNER_ADAPTER
#define dg_configKEYBOARD_SCANNER_ADAPTER       (0)
#endif



#ifndef dg_configPMU_ADAPTER
#define dg_configPMU_ADAPTER                    (1)
#endif

#if (dg_configISO7816_ADAPTER == 1)
# error "dg_configISO7816_ADAPTER is not supported in DA1459X devices"
#else
# undef dg_configISO7816_ADAPTER
# define dg_configISO7816_ADAPTER               (0)
#endif


/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */


/**
 * \addtogroup CONSOLE_IO_SETTINGS Console I/O Settings
 *
 * \brief Console IO configuration settings
 *
 * \{
   Description                               | Setting                    | Default option
   ----------------------------------------- | -------------------------- | :---------------:
   Enable serial console module              | dg_configUSE_CONSOLE       | 0
   Enable serial console stubbed API         | dg_configUSE_CONSOLE_STUBS | 0
   Enable Command Line Interface module      | dg_configUSE_CLI           | 0
   Enable Command Line Interface stubbed API | dg_configUSE_CLI_STUBS     | 0

   \see console.h cli.h

   \note CLI module requires dg_configUSE_CONSOLE to be enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------------- Console IO configuration settings --------------------- */

#ifndef dg_configUSE_CONSOLE
#define dg_configUSE_CONSOLE                    (0)
#endif

#ifndef dg_configUSE_CONSOLE_STUBS
#define dg_configUSE_CONSOLE_STUBS              (0)
#endif

#ifndef dg_configUSE_CLI
#define dg_configUSE_CLI                        (0)
#endif

#ifndef dg_configUSE_CLI_STUBS
#define dg_configUSE_CLI_STUBS                  (0)
#endif
/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ----------------------------- DGTL ----------------------------------------------------------- */

/**
 * \brief Enable D.GTL interface
 *
 * When this macro is enabled, the DGTL framework is available for use.
 * The framework must furthermore be initialized in the application using
 * dgtl_init(). Additionally, the UART adapter must be initialized accordingly.
 *
 * Please see sdk/middleware/dgtl/include/ for further DGTL configuration
 * (in dgtl_config.h) and API.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configUSE_DGTL
#define dg_configUSE_DGTL                       (0)
#endif

/**
 * \addtogroup MIDDLEWARE_DEBUG_SETTINGS Debug Settings
 *
 * \{
 */
/* -------------------------------------- Debug settings ---------------------------------------- */

/**
 * \brief Enable task monitoring.
 *
 * \note Task monitoring can only be enabled if RTT or RETARGET is enabled
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_TASK_MONITORING
#define dg_configENABLE_TASK_MONITORING         (0)
#endif



/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------- OS related configuration ---------------------------------- */

/**
 * \brief Monitor OS heap allocations
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configTRACK_OS_HEAP
#define dg_configTRACK_OS_HEAP                  (0)
#endif

/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ---------------------------------- SYSTEM CONFIGURATION ------------------------------------ */
/**
 * \brief When set to 1, the sys adc service is used to monitor gpadc and support:
 *        - RF calibration if dg_configRF_ENABLE_RECALIBRATION is set.
 *        - RCX calibration if RCX is used as low power clock.
 *        - RCLP and RC32M calibration.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if defined(OS_PRESENT)
#ifndef dg_configUSE_SYS_ADC
#define dg_configUSE_SYS_ADC                    (1)
#endif
#endif

/**
 * \brief Enable System Boot handler
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_BOOT
# if defined(OS_PRESENT) && (dg_configCODE_LOCATION != NON_VOLATILE_IS_NONE)
#  define dg_configUSE_SYS_BOOT                 (1)
# else
#  define dg_configUSE_SYS_BOOT                 (0)
# endif
#endif /* dg_configUSE_SYS_BOOT */

/**
 * \brief When set to 1, the sys trng service is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_TRNG
#if defined(CONFIG_USE_BLE)
#define dg_configUSE_SYS_TRNG                           (1)
#else
#define dg_configUSE_SYS_TRNG                           (0)
#endif
#endif

/**
 * \brief When set to 1, the sys drbg service is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_DRBG
#if defined(CONFIG_USE_BLE)
#define dg_configUSE_SYS_DRBG                           (1)
#else
#define dg_configUSE_SYS_DRBG                           (0)
#endif
#endif

/**
 * \brief A pointer to the physical address of the SYSRAM that is used as entropy source.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 * \note The address MUST be word aligned!
 */
#if (dg_configUSE_SYS_TRNG == 1)
# ifndef dg_configSYS_TRNG_ENTROPY_SRC_ADDR
# define dg_configSYS_TRNG_ENTROPY_SRC_ADDR     MEMORY_CMAC_CACHERAM_BASE
# endif
#endif

/**
 * \brief The length of the buffer which holds the random numbers.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_DRBG_BUFFER_LENGTH
#define dg_configUSE_SYS_DRBG_BUFFER_LENGTH             (30)
#endif

/**
 * \brief Threshold (index) in the buffer which holds the random numbers. When the buffer index
 *        reaches the threshold or becomes greater than the threshold a request for buffer update
 *        will be issued.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD
#define dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD          (24)
#endif

#if (dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD >= dg_configUSE_SYS_DRBG_BUFFER_LENGTH)
#error "The threshold must be less than the buffer length"
#endif

#if (dg_configUSE_SYS_DRBG_BUFFER_THRESHOLD <= 0)
#error "The threshold must be greater than zero"
#endif

/**
 * \brief When set to 1, the ChaCha20 random number generator is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_CHACHA20_RAND
#define dg_configUSE_CHACHA20_RAND                      (1)
#endif

/**
 * \brief When set to 1, the stdlib.h random number generator is enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_STDLIB_RAND
#define dg_configUSE_STDLIB_RAND                        (0)
#endif

#if ((dg_configUSE_CHACHA20_RAND + dg_configUSE_STDLIB_RAND) != 1)
      #error "Only one random number generator must be enabled each time."
#endif


/* ---------------------------------------------------------------------------------------------- */

/* ----------------------------------- Driver dependencies -------------------------------------- */


#  if dg_configUSE_SYS_ADC
#   undef dg_configUSE_HW_GPADC
#   define dg_configUSE_HW_GPADC               (1)
#   undef dg_configGPADC_ADAPTER
#   define dg_configGPADC_ADAPTER              (1)
#  endif /* dg_configUSE_SYS_ADC */




/* If RF is enabled, we need to enable GPADC adapter as well */
#if  dg_configRF_ADAPTER
#undef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (1)
#endif

/*
 * \brief Enable the RTC correction mechanism
 *
 * When RCX is set as the low power clock and Real Time Clock is used (i.e. dg_configUSE_HW_RTC is defined),
 * setting this macro to a non-zero value enables the RTC correction mechanism.
 * The SYS_ADC service is required to trigger the drift compensation mechanism, correcting the RTC time.
 * The value of this macro triggers the correction every (dg_configRTC_CORRECTION * SYS_ADC_PERIOD_TICKS).
 * A drift compensation is also applied after every RCX calibration.
 *
 * \note Using the fastest available system clock is recommended to achieve high RTC accuracy.
 * This minimizes the intrinsic drift due to the correction algorithm, which is executed while the system is at wake-time.
 *
 * \note Although not mandatory, a power-of-2 value for this macro is advised.
 */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS) && dg_configUSE_HW_RTC && dg_configUSE_SYS_ADC)
#ifndef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION                 (64)
#endif
#else
#if dg_configRTC_CORRECTION
# pragma message "dg_configRTC_CORRECTION is only used in RTOS based projects when RCX is set as low power clock. Forcing to 0."
#undef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION                 (0)
#endif
#endif

#if (dg_configSPI_ADAPTER)
/*
 * \brief CS configuration limit for the SPI adapter.
 *
 * The maximum number of available pads in multiple CS configuration.
 *
 * \note Up to (UINT8_MAX - 1). UINT8_MAX is reserved
 */
#ifndef dg_configSPI_ADAPTER_CS_MAX
#define dg_configSPI_ADAPTER_CS_MAX             (8)
#endif

#endif

/*
 * If SYS_TRNG is enabled for DA1459X, is also necessary to enable the AES/HASH driver because
 * sys_trng makes use of aes ecb encryption functionality
 */
#if (dg_configUSE_SYS_TRNG == 1)
#undef  dg_configUSE_HW_AES
#define dg_configUSE_HW_AES                (1)
#endif /* dg_configUSE_SYS_TRNG */
/* ---------------------------------------------------------------------------------------------- */

#endif /* MIDDLEWARE_DEFAULTS_H_ */
/**
\}
\}
*/
