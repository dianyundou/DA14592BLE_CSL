/**
 ****************************************************************************************
 *
 * @file Xtal_TRIM.h
 *
 * @brief Xtal trim API
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

#ifndef XTAL_TRIM_H_
#define XTAL_TRIM_H_

#define AUTO_XTAL_TEST_DBG_EN           0                       // Enable/Disable debug parameters.

// Status codes
#define XTAL_OPERATION_SUCCESS          (0)             // XTAL calibration success.
#define PULSE_OUT_OF_RANGE_ERROR        (-1)            // Pulse found in the pulse pin assigned GPIO was out of acceptable range
#define NO_PULSE_ERROR                  (-2)            // No pulse found, or pulse > 740 ms (measure_pulse aborts)
#define WRITING_VAL_TO_OTP_ERROR        (-3)            // Failed to write value in OTP.
#define INVALID_GPIO_ERROR              (-4)            // Wrong GPIO configuration.
#define WRONG_XTAL_SOURCE_ERROR         (-5)            // Incorrect pulse detected.
#define XTAL_CALIBRATION_ERROR          (-6)            // XTAL calibration error.

int16_t auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin);

uint16_t run_xtal32m_block_calib(void);

void delay(uint32_t dd);
void Setting_Trim(uint32_t Trim_Value);

uint32_t pulse_counter(void);                                   // counting pulses during 500 msec
uint32_t MEASURE_PULSE(int32_t datareg1, int32_t shift_bit1);   // see assembly code

#if AUTO_XTAL_TEST_DBG_EN
void TRIM_test (int S1, int S2);                                // testing
#endif

#endif /* XTAL_TRIM_H_ */

