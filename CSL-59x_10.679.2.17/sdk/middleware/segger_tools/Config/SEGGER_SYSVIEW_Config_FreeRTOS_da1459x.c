/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2021 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the SystemView and RTT protocol, and J-Link.       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: 3.30                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_FreeRTOS.c
Purpose : Sample setup configuration of SystemView with FreeRTOS.
Revision: $Rev: 7745 $
*/

#if (dg_configSYSTEMVIEW == 1)
#if (DEVICE_FAMILY == DA1459X)
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"
#include "sys_timer.h"
#include "interrupts.h"
#include "SEGGER_RTT.h"
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "DemoApp"

// The target device name
#define SYSVIEW_DEVICE_NAME     "DA1459x"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (configSYSTICK_CLOCK_HZ)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        configCPU_CLOCK_HZ

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (MEMORY_SYSRAM_S_BASE)
/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void)
{
        /*
           * The maximum size of the string passed as argument to SEGGER_SYSVIEW_SendSysDesc()
           * should not exceed SEGGER_SYSVIEW_MAX_STRING_LEN (128) bytes. Values can be comma
           * seperated.
           *
           * More ISR entries could be added but this would result in a slower system and might
           * also affect time critical tasks or trigger assertions.
           *
           * This is because multiple SEGGER_SYSVIEW_SendSysDesc() calls will result in multiple
           * RTT transactions.
           *
           * Note also that _cbSendSystemDesc() is called multiple times from the host PC and not
           * just during initialization, so assertions may occur anytime during SystemView monitoring.
           *
           */
        const char* sys_desc =
                       "N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=FreeRTOS,"
                       "I#15=SysTick,"
#if dg_configUNDISCLOSED_SNC
                       //"I#16=Sensor_Node,"
#else
                       //"I#16=RSVD00,"
#endif
                       //"I#17=DMA,"
                       "I#18=CMAC2SYS,"
                       //"I#19=UART,"
                       //"I#20=UART2,"
                       //"I#21=I2C,"
                       //"I#22=SPI,"
                       //"I#23=FCU,"
                       //"I#24=PCM,"
                       //"I#25=SRC_In,"
                       //"I#26=SRC_Out,"
                       //"I#27=SRC2_In,"
                       //"I#28=SRC2_Out,"
                       //"I#29=MDCT,"
                       //"I#30=Timer,"
                       "I#31=Timer2,"
                       //"I#32=RTC,"
                       //"I#33=Key_Wkup_GPIO,"
                       //"I#34=PDC,"
                       //"I#35=MRM,"
                       //"I#36=RSVD20,"
                       //"I#37=QUADDEC,"
                       //"I#38=RSVD22,"
                       "I#39=XTAL32M_Ready,"
                       //"I#40=CLK_CALIBRATION,"
                       "I#41=ADC,"
                       //"I#42=ADC2,"
                       //"I#43=Crypto,"
                       //"I#44=CAPTIMER1,"
                       //"I#45=RFDIAG,"
                       //"I#46=Timer3,"
                       //"I#47=Timer4,"
                       //"I#48=RTC_Event,"
                       //"I#49=GPIO_P0,"
                       //"I#50=GPIO_P1",
                       "I#51=RSVD"
                       ;


          SEGGER_SYSVIEW_SendSysDesc(sys_desc);
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void)
{
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

uint64_t sys_timer_get_timestamp_fromCPM(uint32_t* timer_value);

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetTimestamp()
{
        uint32_t timestamp;
        uint32_t timer_value;

        SEGGER_RTT_LOCK();
        timestamp = (uint32_t) sys_timer_get_timestamp_fromCPM(&timer_value);
        SEGGER_RTT_UNLOCK();

        return timestamp;
}

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetInterruptId(void)
{
        return ((*(U32 *)(0xE000ED04)) & 0x1FF);
}
#endif /* DEVICE_FAMILY */
#endif /* (dg_configSYSTEMVIEW == 1) */

/*************************** End of file ****************************/
