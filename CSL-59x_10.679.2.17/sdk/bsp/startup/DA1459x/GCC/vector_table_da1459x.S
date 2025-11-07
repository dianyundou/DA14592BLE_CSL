/**
 ****************************************************************************************
 *
 * @file vector_table.S
 *
 * @brief Interrupt Vector Table and Patch Table
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

                .syntax	unified
                .arch	armv8-m.main


/*
;<h> Stack Configuration
;  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
#if (dg_configSYSTEMVIEW == 0)
        #ifdef __STACK_SIZE
                .equ    Stack_Size, __STACK_SIZE
        #else
                .equ    Stack_Size, 0xC00
        #endif
#else
        #ifdef __STACK_SIZE
                .equ    Stack_Size, (__STACK_SIZE + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #else
                .equ    Stack_Size, (0xC00 + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #endif
#endif /* (dg_configSYSTEMVIEW == 0) */
                .section .stack
                .align	3
                .globl	__StackTop
                .globl	__StackLimit
__StackLimit:
                .space	Stack_Size
                .size	__StackLimit, . - __StackLimit
__StackTop:
                .size	__StackTop, . - __StackTop


/*
;<h> Heap Configuration
;  <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
#ifdef __HEAP_SIZE
        .equ    Heap_Size, __HEAP_SIZE
#else
        .equ    Heap_Size, 0
#endif

                .if      Heap_Size != 0                     /* Heap is provided */
                .section .heap
                .align	3
                .globl	__HeapBase
                .globl	__HeapLimit
__HeapBase:
                .space	Heap_Size
                .size	__HeapBase, . - __HeapBase
__HeapLimit:
                .size	__HeapLimit, . - __HeapLimit
                .endif

                .section .isr_vector, "a"
                .align 2
                .globl	__isr_vector
                .globl  __isr_vector_end
                .globl  __isr_vector_size
__isr_vector:
                .long   __StackTop              /* Top of Stack */
                .long   Reset_Handler           /* Reset Handler */
                .long   NMI_Handler             /* NMI Handler */
                .long   HardFault_Handler       /* Hard Fault Handler */
                .long   MemManage_Handler       /* MPU Fault Handler */
                .long   BusFault_Handler        /* Bus Fault Handler */
                .long   UsageFault_Handler      /* Usage Fault Handler */
                .long   SecureFault_Handler     /* Secure Fault Handler */
                .long   0                       /* Reserved */
                .long   0                       /* Reserved */
                .long   0                       /* Reserved */
                .long   SVC_Handler             /* SVCall Handler */
                .long   DebugMon_Handler        /* Debug Monitor Handler */
                .long   0                       /* Reserved */
                .long   PendSV_Handler          /* PendSV Handler */
                .long   SysTick_Handler         /* SysTick Handler */

                /* External interrupts */
                .long   RESERVED00_Handler              /*  0: Reserved     		         */
                .long   DMA_Handler                     /*  1: General Purpose DMA           */
                .long   CMAC2SYS_Handler                /*  2: CMAC and mailbox              */
                .long   UART_Handler                    /*  3: UART                          */
                .long   UART2_Handler                   /*  4: UART2                         */
                .long   I2C_Handler                     /*  5: I2C                           */
                .long   SPI_Handler                     /*  6: SPI                           */
                .long   FCU_Handler                     /*  7: FCU                           */
                .long   PCM_Handler                     /*  8: PCM                           */
                .long   SRC_In_Handler                  /*  9: SRC Input                     */
                .long   SRC_Out_Handler                 /* 10: SRC Output                    */
                .long   SRC2_In_Handler                 /* 11: SRC2 Input                    */
                .long   SRC2_Out_Handler                /* 12: SRC2 Output                   */
                .long   MDCT_Handler                    /* 13: MDCT                          */
                .long   Timer_Handler                   /* 14: Timer                         */
                .long   Timer2_Handler                  /* 15: Timer2                        */
                .long   RTC_Handler                     /* 16: RTC                           */
                .long   Key_Wkup_GPIO_Handler           /* 17: Debounced button pressed      */
                .long   PDC_Handler                     /* 18: PDC                           */
                .long   MRM_Handler                     /* 19: Cache Miss Rate Monitor       */
                .long   RESERVED20_Handler              /* 20: Reserved                      */
                .long   QUADDEC_Handler                 /* 21: Quadrature Decoder            */
                .long   RESERVED22_Handler              /* 22: Reserved                      */
                .long   XTAL32M_Ready_Handler           /* 23: XTAL32M osc trimmed & settled */
                .long   Clock_Calibration_Handler       /* 24: Clock_Calibration_Handler     */
                .long   ADC_Handler                     /* 25: Analog-Digital Converter      */
                .long   SDADC_Handler                   /* 26: Analog-Digital Converter 2    */
                .long   Crypto_Handler                  /* 27: Crypto (AES or HASH)          */
                .long   CAPTIMER_Handler                /* 28: GPIO triggered Timer1 Capture */
                .long   RFDIAG_Handler                  /* 29: Baseband/Radio diagnostics    */
                .long   Timer3_Handler                  /* 30: Timer3                        */
                .long   Timer4_Handler                  /* 31: Timer4                        */
                .long   RTC_Event_Handler               /* 32: RTC Event                     */
                .long   GPIO_P0_Handler                 /* 33: GPIO_P0                       */
                .long   GPIO_P1_Handler                 /* 34: GPIO_P1                       */

__isr_vector_end:
                .equ     __isr_vector_size, __isr_vector - __isr_vector
                .size	__isr_vector, . - __isr_vector

