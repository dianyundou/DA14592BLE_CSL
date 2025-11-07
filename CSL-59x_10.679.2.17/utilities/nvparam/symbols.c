/*
 * Copyright (C) 2016-2020 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdint.h>

#define NVPARAM_PARAM_VALUE(TAG, TYPE, ...) \
        TYPE param_ ## TAG[] __attribute__((section("section_" #TAG))) __attribute__((aligned(1))) = { __VA_ARGS__ }; \
        uint16_t param_ ## TAG ## _size __attribute__((section("section_" #TAG "_size"))) __attribute__((aligned(1))) = sizeof(param_ ##TAG);

#if (dg_configNVPARAM_APP_AREA == 1)
#include "app_nvparam_values.h"
#else
#include "platform_nvparam_values.h"
#endif

#define NVPARAM_AREA(NAME, PARTITION, OFFSET)

#define NVPARAM_PARAM(TAG, OFFSET, LENGTH) \
                char sizeofcheck_ ## TAG[LENGTH - sizeof(param_ ## TAG)];

#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH) \
                char sizeofcheck_ ## TAG[LENGTH - sizeof(param_ ## TAG) - 2];

#define NVPARAM_AREA_END()

// define this so preprocessor does not try to include ad_nvparam_defs.h
#define AD_NVPARAM_DEFS_H_

#if (dg_configNVPARAM_APP_AREA == 1)
#include "app_nvparam.h"
#else
#include "platform_nvparam.h"
#endif

// dummy main
int main() { }

