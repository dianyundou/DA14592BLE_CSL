/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_MEMORY_DEFAULTS Memory Default Configuration Values
 *
 * \brief BSP memory default configuration values
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
 * @file bsp_memory_defaults.h
 *
 * @brief Board Support Package. Memory Configuration file default values.
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

#ifndef BSP_MEMORY_DEFAULTS_H_
#define BSP_MEMORY_DEFAULTS_H_

#define PARTITION2(...)
#include "partition_table.h"
#undef PARTITION2

/* ---------------------------------- Heap size configuration ----------------------------------- */

/**
 * \brief Heap size for used libc malloc()
 *
 * Specifies the amount of RAM that will be used as heap for libc malloc() function.
 * It can be configured in bare metal projects to match application's requirements.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef __HEAP_SIZE
# if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
#  define __HEAP_SIZE                                   0x0600
# else
#  define __HEAP_SIZE                                   0x0100
# endif
#endif


/* ---------------------------------------------------------------------------------------------- */

/* --------------------------------- Stack size configuration ----------------------------------- */

/**
 * \brief Stack size for main() function and interrupt handlers.
 *
 * Specifies the amount of RAM that will be used as stack for the main() function and the interrupt
 * handlers.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef __STACK_SIZE
#define __STACK_SIZE                                    0x0200
#endif

/* -------------------------- INCLUDE MEMORY LAYOUT CONFIGURATION ------------------------------- */

#include "bsp_memory_defaults_da1459x.h"

#endif /* BSP_MEMORY_DEFAULTS_H_ */
/**
 * \}
 *
 * \}
 */
