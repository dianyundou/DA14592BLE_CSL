/*
 * Copyright (C) 2020 Renesas Electronics Corporation and/or its affiliates.
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

/* Linker script to configure memory regions.
 * May need to be modified for a specific board.
 *   ROM.ORIGIN: starting address of read-only RAM area
 *   ROM.LENGTH: length of read-only RAM area
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 */

/*
 * The size of the interrupt vector table
 */
#define IVT_AREA_OVERHEAD               0x200

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        #define CODE_BASE_ADDRESS               (0x0)   /* Remapped address will be at 0x20000000. */
        #define CODE_SZ                         (CODE_SIZE)

        MEMORY
        {
                /* CODE and RAM are merged into a single RAM section */
                RAM (rx) : ORIGIN = CODE_BASE_ADDRESS, LENGTH = CODE_SZ
        }
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH) || (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
        #define CODE_BASE_ADDRESS               (0x0)   /* Remapped address will be at any offset according to the image header. */
        #define CODE_SZ                         (CODE_SIZE)
        #define RAM_BASE_ADDRESS                (0x20000000 + IVT_AREA_OVERHEAD)
        #define RAM_SZ                          (RAM_SIZE - IVT_AREA_OVERHEAD)

        MEMORY
        {
                ROM (rx) : ORIGIN = CODE_BASE_ADDRESS, LENGTH = CODE_SZ
                RAM (rw) : ORIGIN = RAM_BASE_ADDRESS,  LENGTH = RAM_SZ
        }
#else
        #error "Unknown code location type..."
#endif


