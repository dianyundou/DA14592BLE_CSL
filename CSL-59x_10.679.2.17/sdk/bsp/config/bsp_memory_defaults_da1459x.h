/**
 * \addtogroup BSP_MEMORY_DEFAULTS
 * \{
 *
 * \addtogroup MEMORY_LAYOUT_SETTINGS Memory Layout Configuration Settings
 *
 * \brief Memory Layout Configuration Settings
 * \{
 *
 */
/**
 ****************************************************************************************
 *
 * @file bsp_memory_defaults_da1459x.h
 *
 * @brief Board Support Package. Device-specific system configuration default values.
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
#ifndef BSP_MEMORY_DEFAULTS_DA1459X_H_
#define BSP_MEMORY_DEFAULTS_DA1459X_H_


#if defined(CONFIG_USE_BLE)


/* The maximum expected RAM area in bytes for ROM patches */
#define __ROM_PATCHES_MAX_AREA_BYTES                    (10 * 1024)

/* The remaining RAM area in bytes for ROM patches */
#define __ROM_PATCHES_REMAINING_AREA_BYTES              (__ROM_PATCHES_MAX_AREA_BYTES - CMAC_AREA_CURRENT_PATCH_SIZE)

/**
 * \brief Reserved RAM space in bytes for potential ROM patches.
 *
 * \note The size should be at least larger than the remaining RAM area for ROM patches
 *       to avoid the risk of accepting future ROM patches.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES
#define dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES      __ROM_PATCHES_MAX_AREA_BYTES
#endif

#if (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES < __ROM_PATCHES_REMAINING_AREA_BYTES)
#warning "Non-recommended SYSRAM size is reserved for future ROM patches." \
         "Accepting future ROM patches is in risk."
#endif

#else /* non-BLE case */

#define dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES      (0)

#endif /* CONFIG_USE_BLE */


#ifdef dg_config_RETAINED_UNINIT_SECTION_SIZE
#error "dg_config_RETAINED_UNINIT_SECTION_SIZE is deprecated! "\
       "Please use dg_configRETAINED_UNINIT_SECTION_SIZE instead (no underscore)!"
#endif

/**
 * \brief Size of the RETAINED_RAM_UNINIT section, in bytes.
 *
 * This section is not initialized during startup by either the bootloader or
 * the application. It can be therefore used to maintain debug or other relevant
 * information that will no be lost after reset. It should be guaranteed that
 * both the bootloader (if any) and the application are using the same value for
 * this option (or otherwise the booloader can corrupt the contents of the section).
 * To use this section for a specific variable, use the __RETAINED_UNINIT attribute.
 */
#ifndef dg_configRETAINED_UNINIT_SECTION_SIZE
#if defined(CONFIG_USE_BLE)
#define dg_configRETAINED_UNINIT_SECTION_SIZE           (200)
#else
#define dg_configRETAINED_UNINIT_SECTION_SIZE           (128)
#endif /* defined(CONFIG_USE_BLE) */
#endif /* dg_configRETAINED_UNINIT_SECTION_SIZE */

/**
 * \brief Code size in QSPI projects for DA1459x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CODE_SIZE_AA
#define dg_configQSPI_CODE_SIZE_AA                      (256 * 1024) /* Take into account CMI firmware size */
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
/**
 * \brief Maximum size (in bytes) of image in the QSPI flash.
 *
 * The image in the QSPI flash contains the text (code + const data) and any other initialized data.
 *
 * \note This size should not be larger than the flash partition where the image is stored.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_MAX_IMAGE_SIZE
#define dg_configQSPI_MAX_IMAGE_SIZE                    ( IMAGE_PARTITION_SIZE )
#endif

#if dg_configQSPI_MAX_IMAGE_SIZE < dg_configQSPI_CODE_SIZE_AA
#error "dg_configQSPI_MAX_IMAGE_SIZE cannot be smaller than dg_configQSPI_CODE_SIZE_AA"
#endif
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) */

/**
 * \brief RAM-block size in cached mode for DA1459x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CACHED_RAM_SIZE_AA
# if dg_configENABLE_MTB
/* Reserve space at the end of RAM for MTB. */
#  define dg_configQSPI_CACHED_RAM_SIZE_AA              (96 * 1024 - (MTB_BUFFER_SIZE) - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# else
#  define dg_configQSPI_CACHED_RAM_SIZE_AA              (96 * 1024 - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# endif
#endif

/**
 * \brief Code size in EFLASH projects for DA1459x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configEFLASH_CODE_SIZE_AA
#define dg_configEFLASH_CODE_SIZE_AA                    (120 * 1024) /* Take into account CMI firmware size */
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
/**
 * \brief Maximum size (in bytes) of image in the EFLASH.
 *
 * The image in the EFLASH contains the text (code + const data) and any other initialized data.
 *
 * \note This size should not be larger than the flash partition where the image is stored.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configEFLASH_MAX_IMAGE_SIZE
#define dg_configEFLASH_MAX_IMAGE_SIZE                  ( IMAGE_PARTITION_SIZE )
#endif

#if dg_configEFLASH_MAX_IMAGE_SIZE < dg_configEFLASH_CODE_SIZE_AA
#error "dg_configEFLASH_MAX_IMAGE_SIZE cannot be smaller than dg_configEFLASH_CODE_SIZE_AA"
#endif
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH) */

/**
 * \brief RAM-block size in EFLASH cached mode for DA1459x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configEFLASH_CACHED_RAM_SIZE_AA
# if dg_configENABLE_MTB
/* Reserve space at the end of RAM for MTB. */
#  define dg_configEFLASH_CACHED_RAM_SIZE_AA            (96 * 1024 - (MTB_BUFFER_SIZE) - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# else
#  define dg_configEFLASH_CACHED_RAM_SIZE_AA            (96 * 1024 - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# endif
#endif

/**
 * \brief Code and RAM size in RAM projects for DA1459xAA.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configRAM_CODE_SIZE_AA
/* CODE and RAM are merged into a single RAM section */
# if dg_configENABLE_MTB
/* Reserve space at the end of RAM for MTB. */
#  define dg_configRAM_CODE_SIZE_AA                     (96 * 1024 - (MTB_BUFFER_SIZE) - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# else
#  define dg_configRAM_CODE_SIZE_AA                     (96 * 1024 - (dg_configRESERVED_RAM_SIZE_FOR_ROM_PATCHES))
# endif /* dg_configENABLE_MTB */
#endif /* dg_configRAM_CODE_SIZE_AA */

/**
 * \brief Retention memory configuration.
 *
 * 16 bits field; each couple of bits controls whether the relevant memory block will be retained or not.
 * -  bits 0-1   : SYSRAM1
 * -  bits 2-3   : SYSRAM2
 * -  bits 4-5   : SYSRAM3
 *
 * Available presets: RETMEM_RETAIN_ALL, RETMEM_RETAIN_NONE
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configMEM_RETENTION_MODE
#define dg_configMEM_RETENTION_MODE                     RETMEM_RETAIN_ALL
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  define CODE_SIZE                                     dg_configQSPI_CODE_SIZE_AA
#  define RAM_SIZE                                      dg_configQSPI_CACHED_RAM_SIZE_AA
# else // MIRRORED
#  error "QSPI mirrored mode is not supported!"
# endif
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  pragma message "RAM cached mode is not supported! Resetting to RAM (mirrored) mode!"
#  undef dg_configEXEC_MODE
#  define dg_configEXEC_MODE                            MODE_IS_RAM
# endif
# define CODE_SIZE                                      dg_configRAM_CODE_SIZE_AA
# if (CODE_SZ > 512)
#  error "The used CODE_SZ value exceed the total amount of RAM!"
# endif
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  define CODE_SIZE                                     dg_configEFLASH_CODE_SIZE_AA
#  define RAM_SIZE                                      dg_configEFLASH_CACHED_RAM_SIZE_AA
# else // MIRRORED
#  error "EFLASH mirrored mode is not supported!"
# endif
#else
# error "Unknown configuration..."
#endif /* dg_configCODE_LOCATION */

/**
 * \brief Output binary can be loaded by SEGGER FLASH Loader.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)) && (dg_configEXEC_MODE == MODE_IS_CACHED)
#ifndef dg_configUSE_SEGGER_FLASH_LOADER
#define dg_configUSE_SEGGER_FLASH_LOADER                (1)
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */
#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)) && (dg_configEXEC_MODE == MODE_IS_CACHED)*/


#endif /* BSP_MEMORY_DEFAULTS_DA1459X_H_ */

/**
 * \}
 * \}
 */
