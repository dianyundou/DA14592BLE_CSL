/**
 ****************************************************************************************
 *
 * @file ad_nvparam_defs.h
 *
 * @brief NV-Parameters definitions
 *
 * Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
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

/*
 * DO NOT MODIFY THIS FILE!!!
 *
 * NV-Parameters configuration shall be done in platform_nvparam.h!!!
 *
 */

#ifndef AD_NVPARAM_DEFS_H_
#define AD_NVPARAM_DEFS_H_

#ifndef IN_AD_NVPARAM_C

/*
 * If this is included outside of ad_nvparam.c, we just define empty macros so nothing but tag
 * definitions are created from platform_nvparam.h
 */

#define NVPARAM_AREA(NAME, PARTITION, OFFSET)
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_AREA_END()

#else

/*
 * If this is included inside of ad_nvparam.c, we will create proper configuration structure.
 */

/*
 * First we create configurations of each area - this will create "area_XXX" symbol for each defined
 * area in platform_nvparam.h. It contains all parameters defined for given area.
 */

#define NVPARAM_AREA(NAME, PARTITION, OFFSET) \
        static const parameter_t area_ ## NAME[] = {
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH) \
                { \
                        .tag = TAG, \
                        .offset = OFFSET, \
                        .length = LENGTH, \
                },
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH) \
                { \
                        .tag = TAG, \
                        .flags = FLAG_VARIABLE_LEN, \
                        .offset = OFFSET, \
                        .length = LENGTH, \
                },
#define NVPARAM_AREA_END() \
        };

#undef PLATFORM_NVPARAM_H_
#if (dg_configNVPARAM_APP_AREA == 1)
 #undef APP_NVPARAM_H_
 #include "app_nvparam.h"
#else
 #include "platform_nvparam.h"
#endif

/*
 * Next, using the same platform_nvparam.h, we define list of areas. Each has proper attributes set
 * and pointer to corresponding area structure.
 */

#undef NVPARAM_AREA
#undef NVPARAM_PARAM
#undef NVPARAM_VARPARAM
#undef NVPARAM_AREA_END
#define NVPARAM_AREA(NAME, PARTITION, OFFSET) \
        { \
                .name = #NAME, \
                .partition = PARTITION, \
                .offset = OFFSET, \
                .parameters = area_ ## NAME, \
                .num_parameters = sizeof(area_ ## NAME) / sizeof(area_ ## NAME[0]), \
        },
#define NVPARAM_PARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_VARPARAM(TAG, OFFSET, LENGTH)
#define NVPARAM_AREA_END()

static const area_t areas[] = {
#undef PLATFORM_NVPARAM_H_
#if (dg_configNVPARAM_APP_AREA == 1)
 #undef APP_NVPARAM_H_
 #include "app_nvparam.h"
#else
 #include "platform_nvparam.h"
#endif
};

#define num_areas (sizeof(areas) / sizeof(areas[0]))

#endif /* IN_AD_NVPARAM_C */

#endif /* AD_NVPARAM_DEFS_H_ */
