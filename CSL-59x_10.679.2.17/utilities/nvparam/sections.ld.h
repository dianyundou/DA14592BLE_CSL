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

SECTIONS {
        .bss : {
                // just need to define this so linker does not complain
                __bss_start__ = .;
                __bss_end__ = .;
        }

        .nvparam : {
                FILL(0xFF);

#define NVPARAM_AREA(NAME, PARTITION, OFFSET) \
                s = OFFSET;

#define NVPARAM_PARAM(TAG, OFFSET, LENGTH) \
                . = s + OFFSET; \
                *(section_ ## TAG)

#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH) \
                . = s + OFFSET; \
                *(section_ ## TAG ## _size) \
                *(section_ ## TAG)

#define NVPARAM_AREA_END()

// define this so preprocessor does not try to include ad_nvparam_defs.h
#define AD_NVPARAM_DEFS_H_

#if (dg_configNVPARAM_APP_AREA == 1)
#include "app_nvparam.h"
#else
#include "platform_nvparam.h"
#endif

        }
}
