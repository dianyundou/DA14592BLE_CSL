/*
 * Copyright (C) 2020-2025 Renesas Electronics Corporation and/or its affiliates.
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
/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions ROM and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __Vectors_End
 *   __Vectors_Size
 */

/* Library configurations */
GROUP(libgcc.a libc.a libm.a libnosys.a)

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
#define IMG_FW_IVT_OFFSET               (0x400)
#define RETENTION_RAM_INIT_SIZE         (__retention_ram_init_end__ - __retention_ram_init_start__)
#define NON_RETENTION_RAM_INIT_SIZE     (__non_retention_ram_init_end__ - __non_retention_ram_init_start__)
#else
/* CODE and RAM are merged into a single RAM section */
#define ROM                             RAM
#endif

#if ( dg_configUSE_SEGGER_FLASH_LOADER == 1 )
#define CS_OFFSET 0x1000
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
/* Could be anything. Recommended by Open_Flashloader wiki
 * to use the real address of the non volatile device. Should it be changed,
 * the segger_flash_loader project  needs to be modified accordingly.
 */
#define LOAD_ADDRESS_OFFSET 0x31000000
#define PROD_HEAD_OFFSET 0x800
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#define LOAD_ADDRESS_OFFSET 0x32000000
#define PROD_HEAD_OFFSET 0x1000
#else
#error "Flash Loader does not support other code location."
#endif
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
#define IMG_FW_BASE_OFFSET      0x2000
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
#define IMG_FW_BASE_OFFSET      0x3000
#endif /* dg_configCODE_LOCATION */
#define IMG_FW_BASE_ADDRESS     (IMG_FW_BASE_OFFSET + LOAD_ADDRESS_OFFSET)
#define IMG_FW_IVT_BASE_ADDRESS (IMG_FW_BASE_OFFSET + IMG_FW_IVT_OFFSET + LOAD_ADDRESS_OFFSET)
#else
#define IMG_FW_IVT_BASE_ADDRESS 0x0
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */

ENTRY(Reset_Handler)

SECTIONS
{
        .init_text :
#if ( dg_configUSE_SEGGER_FLASH_LOADER == 1 )
        AT (IMG_FW_IVT_BASE_ADDRESS)
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */
        {
                KEEP(*(.isr_vector))
                /* Interrupt vector remmaping overhead */
                . = 0x200;
                __Vectors_End = .;
                __Vectors_Size = __Vectors_End - __isr_vector;

                KEEP(*(__external_application_parameters__))

                . = ALIGN(4);

                *(text_reset*)
        } > ROM

        .text :
        {
                /* Optimize the code of specific libgcc files by executing them
                 * from the .retention_ram_init section. */
                *(EXCLUDE_FILE(*libnosys.a:sbrk.o
                               *libgcc.a:_aeabi_uldivmod.o
                               *libgcc.a:_muldi3.o
                               *libgcc.a:_dvmd_tls.o
                               *libgcc.a:bpabi.o
                               *libgcc.a:_udivdi3.o
                               *libgcc.a:_clzdi2.o
                               *libgcc.a:_clzsi2.o) .text*)

                . = ALIGN(4);

#ifdef CONFIG_USE_BLE
#if (dg_configEXEC_MODE != MODE_IS_CACHED)
                . = ALIGN(0x100); /* Code region should start at 256 bytes boundary  */
                cmi_fw_dst_addr = .;
#endif

                /*
                 * Section used to store the CMAC FW.
                 * Code should copy this FW to address 'cmi_fw_dst_addr' and
                 * configure the memory controller accordingly.
                 */
                __cmi_fw_area_start = .;
                KEEP(*(.cmi_fw_area*))
                __cmi_fw_area_end = .;

#if (dg_configEXEC_MODE != MODE_IS_CACHED)
                __cmi_section_start__ = .;

                . = ALIGN(4); /* Data region should start at 4-bytes boundary */

                /*
                 * Create space for CMAC data
                 */
                KEEP(*(.cmi_data_area*))

                . = ALIGN(4); /* CMI_END_REG does not exist in DA1459x, so there is no alignment requirement */

                __cmi_section_end__ = . - 1;

                KEEP(*(.cmi_data_ext_area*))
#endif
#endif /* CONFIG_USE_BLE */

                . = ALIGN(4);
                __start_adapter_init_section = .;
                KEEP(*(adapter_init_section))
                __stop_adapter_init_section = .;

                . = ALIGN(4);
                __start_bus_init_section = .;
                KEEP(*(bus_init_section))
                __stop_bus_init_section = .;

                . = ALIGN(4);
                __start_device_init_section = .;
                KEEP(*(device_init_section))
                __stop_device_init_section = .;

                KEEP(*(.init))
                KEEP(*(.fini))

                /* .ctors */
                *crtbegin.o(.ctors)
                *crtbegin?.o(.ctors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
                *(SORT(.ctors.*))
                *(.ctors)

                /* .dtors */
                *crtbegin.o(.dtors)
                *crtbegin?.o(.dtors)
                *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
                *(SORT(.dtors.*))
                *(.dtors)

                . = ALIGN(4);
                /* preinit data */
                PROVIDE_HIDDEN (__preinit_array_start = .);
                KEEP(*(.preinit_array))
                PROVIDE_HIDDEN (__preinit_array_end = .);

                . = ALIGN(4);
                /* init data */
                PROVIDE_HIDDEN (__init_array_start = .);
                KEEP(*(SORT(.init_array.*)))
                KEEP(*(.init_array))
                PROVIDE_HIDDEN (__init_array_end = .);

                . = ALIGN(4);
                /* finit data */
                PROVIDE_HIDDEN (__fini_array_start = .);
                KEEP(*(SORT(.fini_array.*)))
                KEEP(*(.fini_array))
                PROVIDE_HIDDEN (__fini_array_end = .);

                *(.rodata*)

                KEEP(*(.eh_frame*))
        } > ROM

        .ARM.extab :
        {
                *(.ARM.extab* .gnu.linkonce.armextab.*)
        } > ROM

        __exidx_start = .;
        .ARM.exidx :
        {
                *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        } > ROM
        __exidx_end = .;

        /* To copy multiple ROM to RAM sections,
         * uncomment .copy.table section and,
         * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */

        .copy.table :
        {
                . = ALIGN(4);
                __copy_table_start__ = .;
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                LONG (__etext)
                LONG (__retention_ram_init_start__)
                LONG (RETENTION_RAM_INIT_SIZE)

                LONG (__etext + (RETENTION_RAM_INIT_SIZE))
                LONG (__non_retention_ram_init_start__)
                LONG (NON_RETENTION_RAM_INIT_SIZE)
#endif
                __copy_table_end__ = .;
        } > ROM


        /* To clear multiple BSS sections,
         * uncomment .zero.table section and,
         * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */

        .zero.table :
        {
                . = ALIGN(4);
                __zero_table_start__ = .;
                LONG (__retention_ram_zi_start__)
                LONG (__retention_ram_zi_end__ - __retention_ram_zi_start__)
#ifdef CONFIG_USE_BLE
                LONG (__cmi_section_retained_zi_start__)
                LONG (__cmi_section_retained_zi_end__ - __cmi_section_retained_zi_start__)
#endif /* CONFIG_USE_BLE */
                __zero_table_end__ = .;
        } > ROM

        __etext = .;

        /*
        * Retention ram that should not be initialized during startup.
        * On QSPI cached images, it should be at a fixed RAM address for both
        * the bootloader and the application, so that the bootloader will not alter
        * those data due to conflicts between its .data/.bss sections with application's
        * .retention_ram_uninit section.
        * - On QSPI images it is relocated to the first RAM address after IVT_AREA_OVERHEAD
        *       with fixed size of dg_configRETAINED_UNINIT_SECTION_SIZE bytes.
        * - On RAM images the section is not located at a fixed location.
        */
        .retention_ram_uninit (NOLOAD) :
        {
                __retention_ram_uninit_start__ = .;
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                ASSERT( . == ORIGIN(RAM), ".retention_ram_uninit section moved!");
#endif /* (dg_configEXEC_MODE == MODE_IS_CACHED) */
                KEEP(*(nmi_info))
                KEEP(*(hard_fault_info))
                KEEP(*(retention_mem_uninit))

                ASSERT( . <= __retention_ram_uninit_start__ + dg_configRETAINED_UNINIT_SECTION_SIZE,
                        "retention_ram_uninit section overflowed! Increase dg_configRETAINED_UNINIT_SECTION_SIZE.");

                . = __retention_ram_uninit_start__ + dg_configRETAINED_UNINIT_SECTION_SIZE;
                __retention_ram_uninit_end__ = .;
        } > RAM

        /*
         * Initialized retention RAM
         */
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
        .retention_ram_init : AT (IMG_FW_IVT_BASE_ADDRESS +__etext)
#else
        /*
         * No need to add this to the copy table,
         * copy will be done by the debugger.
         */
        .retention_ram_init :
#endif
        {
                __retention_ram_init_start__ = .;
                . = ALIGN(4); /* Required by copy table */

                /*
                 * Retained .text sections moved to RAM that need to be initialized
                 */
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                /* Retained code exists only in QSPI projects */
                *(text_retained)
#endif
                /* Make the '.text' section of specific libgcc files retained, to
                 * optimize perfomance */
                *libnosys.a:sbrk.o (.text*)
                *libgcc.a:_aeabi_uldivmod.o (.text*)
                *libgcc.a:_muldi3.o (.text*)
                *libgcc.a:_dvmd_tls.o (.text*)
                *libgcc.a:bpabi.o (.text*)
                *libgcc.a:_udivdi3.o (.text*)
                *libgcc.a:_clzdi2.o (.text*)
                *libgcc.a:_clzsi2.o (.text*)

                /*
                 * Retained .data sections that need to be initialized
                 */

                /* Retained data */
                *(privileged_data_init)
                *(.retention)

                *(vtable)

                *(retention_mem_init)
                *(retention_mem_const)

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
                *libg_nano.a:* (.data*)
                *libnosys.a:* (.data*)
                *libgcc.a:* (.data*)
                *libble_stack_da1459x.a:* (.data*)
                *crtbegin.o (.data*)
#else /* dg_configEXEC_MODE */
                *(.data*)
#endif /* dg_configEXEC_MODE */

                KEEP(*(.jcr*))
                . = ALIGN(4); /* Required by copy table */
                /* All data end */
                __retention_ram_init_end__ = .;
        } > RAM

        /*
         * Zero-initialized retention RAM
         */
        .retention_ram_zi (NOLOAD) :
        {
                __retention_ram_zi_start__ = .;

                *(privileged_data_zi)
                *(retention_mem_zi)
                *(afmn_pair_retention_mem_zi)
                *(afmn_unpaired_retention_mem_zi)

                *libg_nano.a:* (.bss*)
                *libnosys.a:* (.bss*)
                *libgcc.a:* (.bss*)
                *libble_stack_da1459x.a:* (.bss*)
                *crtbegin.o (.bss*)

                *(os_heap)

                __HeapBase = .;
                __end__ = .;
                end = __end__;
                KEEP(*(.heap*))
                __HeapLimit = .;

                __retention_ram_zi_end__ = .;
        } > RAM

        .stack_section (NOLOAD) :
        {
                /* Advance the address to avoid output section discarding */
                . = . + 1;
                /* 8-byte alignment guaranteed by vector_table_da1459x.S, also put here for
                 * clarity. */
                . = ALIGN(8);
                __StackLimit = .;
                KEEP(*(.stack*))
                . = ALIGN(8);
                __StackTop = .;
                /* Provide __StackTop to newlib by defining __stack externally. SP will be
                 * set when executing __START. If not provided, SP will be set to the default
                 * newlib nano value (0x80000) */
                PROVIDE(__stack = __StackTop);
        } > RAM


#ifdef CONFIG_USE_BLE
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
        /*
         * CMAC interface section
         */
        .cmi_section (NOLOAD) :
        {
                __cmi_section_start__ = .;

                . = ALIGN(0x100); /* Code region should start at 1Kb boundary */

                /*
                 * The actual CMAC code (copied from '.cmi_fw_area')
                 * will be running here.
                 */
                cmi_fw_dst_addr = .;

                /*
                 * Create space to copy/expand the CMAC image to.
                 */
                . += (__cmi_fw_area_end - __cmi_fw_area_start);

                . = ALIGN(4); /* Data region should start at 4-bytes boundary */

                /*
                 * Create space for CMAC data
                 */
                KEEP(*(.cmi_data_area*))

                . = ALIGN(4); /* CMI_END_REG does not exist in DA1459x, so there is no alignment requirement */

                __cmi_section_end__ = . - 1;
                KEEP(*(.cmi_data_ext_area*))
        } > RAM
#endif /* (dg_configEXEC_MODE == MODE_IS_CACHED) */
        .cmi_section_retained_zi (NOLOAD) :
        {
                __cmi_section_retained_zi_start__ = .;
                *(retention_mem_cmi_zi)
                __cmi_section_retained_zi_end__ = .;
        } > RAM
#endif /* CONFIG_USE_BLE */

        __non_retention_ram_start__ = .;

        /*
         * Initialized RAM area that does not need to be retained during sleep.
         * On RAM projects, they are located in the .retention_ram_init section
         * for better memory handling.
         */
#if (dg_configEXEC_MODE == MODE_IS_CACHED)
        .non_retention_ram_init :  AT (IMG_FW_IVT_BASE_ADDRESS + __etext + (RETENTION_RAM_INIT_SIZE))
        {
                __non_retention_ram_init_start__ = .;
                . = ALIGN(4); /* Required by copy table */
                *(EXCLUDE_FILE(*libg_nano.a:* *libnosys.a:* *libgcc.a:* *libble_stack_da1459x.a:* *crtbegin.o) .data*)

                . = ALIGN(4); /* Required by copy table */
                __non_retention_ram_init_end__ = .;
        } > RAM
#endif /* (dg_configEXEC_MODE == MODE_IS_CACHED) */

        /*
         * Note that region [__bss_start__, __bss_end__] will be also zeroed by newlib nano,
         * during execution of __START.
         */
        .bss :
        {
                . = ALIGN(4);
                __bss_start__ = .;

                *(EXCLUDE_FILE(*libg_nano.a:* *libnosys.a:* *libgcc.a:* *libble_stack_da1459x.a:* *crtbegin.o) .bss*)

                *(COMMON)
                . = ALIGN(4);
                __bss_end__ = .;
        } > RAM

        __non_retention_ram_end__ = .;

        __unused_ram_start__ = . + 1;

#if ( dg_configUSE_SEGGER_FLASH_LOADER == 1 )
        /* The CS_OFFSET region is reserved for CS and keys. */
        .cs_and_keys :
        AT (LOAD_ADDRESS_OFFSET)
        {
                /* By default the booter searches the default product header address at the start
                 * of eFlash. Add a CS part to override the default address.
                 */
                LONG(0xA5A5A5A5)
                LONG(0x60000000 | CS_OFFSET)
#if ( dg_configSUOTA_ASYMMETRIC == 1 )
                /* TCS values for 64Kb Flash region size */
                LONG(0x1A0C0044)
                LONG(0x00A00007)
                /* Fill the reserved region with a dummy value. */
                FILL(0xFFFFFFFF)
                . += CS_OFFSET - 16 - 1;
#elif (defined(CONFIG_USE_BLE) && (dg_configSUOTA_SUPPORT == 0))
                /* TCS values for 256Kb Flash region size */
                LONG(0x1A0C0044)
                LONG(0x00A09005)

                /* Fill the reserved region with a dummy value. */
                FILL(0xFFFFFFFF)

                . += CS_OFFSET - 16 - 1;
#else
                /* Fill the reserved region with a dummy value. */
                FILL(0xFFFFFFFF)
                . += CS_OFFSET - 8 - 1;
#endif /* (defined(CONFIG_USE_BLE) && (dg_configSUOTA_SUPPORT == 0)) */

                BYTE(0xFF)
        } > ROM

        .prod_head :
        AT (CS_OFFSET + LOAD_ADDRESS_OFFSET)

        SUBALIGN(1)
        {
                SHORT(0x7050)                   // 'Pp' flag
                LONG(IMG_FW_BASE_OFFSET)        // active image pointer
                LONG(IMG_FW_BASE_OFFSET)        // update image pointer
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
                KEEP(*(__product_header_primary__))
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
                LONG(0xFFFFFFFF)                // busrtcmdA
                LONG(0xFFFFFFFF)                // busrtcmdB
                SHORT(0x11AA)                   // Flash config section
                SHORT(0x0000)                   // Flash config length
                SHORT(0x3E66)                   // CRC

#endif /* dg_configCODE_LOCATION */
                . = PROD_HEAD_OFFSET;
        } > ROM = 0xFFFFFFFF

        .prod_head_backup :
        AT (CS_OFFSET + PROD_HEAD_OFFSET + LOAD_ADDRESS_OFFSET)

        SUBALIGN(1)
        {
                SHORT(0x7050)                   // 'Pp' flag
                LONG(IMG_FW_BASE_OFFSET)        // active image pointer
                LONG(IMG_FW_BASE_OFFSET)        // update image pointer
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)
                KEEP(*(__product_header_backup__))
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_EMBEDDED_FLASH)
                LONG(0xFFFFFFFF)                // busrtcmdA
                LONG(0xFFFFFFFF)                // busrtcmdB
                SHORT(0x11AA)                   // Flash config section
                SHORT(0x0000)                   // Flash config length
                SHORT(0x3E66)                   // CRC

#endif /* dg_configCODE_LOCATION */
                . = PROD_HEAD_OFFSET;
        } > ROM = 0xFFFFFFFF

        .img_head :
        AT (IMG_FW_BASE_ADDRESS)
        {
                SHORT(0x7151)                   // 'Qq' flag
                LONG(SIZEOF(.text))
                LONG(0x00000000)                // crc, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // version, doesn't matter
                LONG(0x00000000)                // timestamp, doesn't matter
                LONG(IMG_FW_IVT_OFFSET)         // IVT pointer
                SHORT(0x22AA)                   // Security section type
                SHORT(0x0000)                   //Security section length
                SHORT(0x44AA)                   // Device admin type
                SHORT(0x0000)                   // Device admin length
                . = IMG_FW_IVT_OFFSET;
        } > ROM = 0xFFFFFFFF
#endif /* dg_configUSE_SEGGER_FLASH_LOADER */

#if (dg_configEXEC_MODE == MODE_IS_CACHED)
        /* Make sure that the initialized data fits in flash. Make sure that image header also
         * fits in MAX_IMAGE_SIZE.
         */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_QSPI_FLASH)

        ASSERT(__etext + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE + IMG_FW_IVT_OFFSET <
                        ORIGIN(ROM) + dg_configQSPI_MAX_IMAGE_SIZE, "ROM space overflowed")
#else
        ASSERT(__etext + RETENTION_RAM_INIT_SIZE + NON_RETENTION_RAM_INIT_SIZE + IMG_FW_IVT_OFFSET <
                        ORIGIN(ROM) + dg_configEFLASH_MAX_IMAGE_SIZE, "ROM space overflowed")
#endif

#endif
}
