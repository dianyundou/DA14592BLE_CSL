/**************************************************************************//**
 * @file     system_DA1459x.h
 * @brief    CMSIS Device System Header File for DA1459x Device
 * @version  V5.9.0
 * @date     15. November 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* Copyright (c) 2023 Modified by Renesas Electronics Corporation and/or its affiliates. */


#ifndef SYSTEM_DA1459x_H
#define SYSTEM_DA1459x_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
  \brief System Clock Frequency (Core Clock)
*/
extern uint32_t SystemCoreClock;     /*!< System Clock Frequency (Core Clock) */


/**
  \brief Setup the microcontroller system.

   Initialize the System and update the SystemCoreClock variable.
 */
extern void SystemInit (void);


/**
  \brief  Update SystemCoreClock variable.
   Updates the SystemCoreClock with current core Clock retrieved from cpu registers.
 */
extern void SystemCoreClockUpdate (void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_DA1459x_H */
