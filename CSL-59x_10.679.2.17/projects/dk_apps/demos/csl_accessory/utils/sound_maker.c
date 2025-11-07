/**
 ****************************************************************************************
 *
 * @file sound_maker.c
 *
 * @brief Sound maker implementation
 *
 * Copyright (C) 2025 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdint.h>
#include "sdk_defs.h"
#include "osal.h"
#include "led_control.h"

#include "sound_maker.h"

/* Maximum number of sound maker instances that can be handled */
#ifndef SOUND_MAKERS_COUNT_MAX
#define SOUND_MAKERS_COUNT_MAX          (2)
#endif

/* Sound maker command types */
typedef enum {
        SOUND_MAKER_CMD_ENABLE,
        SOUND_MAKER_CMD_DISABLE,
        SOUND_MAKER_CMD_DELETE
} SOUND_MAKER_CMD;

/* Sound maker instances */
__RETAINED static struct sound_maker_t {
        bool valid;
        bool enabled;
        SOUND_MAKER_VOLUME vol;
} sound_makers[SOUND_MAKERS_COUNT_MAX];

/* Returns pointer to free sound maker instance entry */
static struct sound_maker_t *search_for_free_sound_maker(void)
{
        for (int i = 0; i < SOUND_MAKERS_COUNT_MAX; i++) {
                if (sound_makers[i].valid == false) {
                        return &sound_makers[i];
                }
        }

        return NULL;
}

/* Updates the state of sound maker */
static void update_sound_maker(sound_maker_handle_t sound_maker, SOUND_MAKER_CMD cmd,
        SOUND_MAKER_VOLUME vol)
{
        struct sound_maker_t *sound_mkr = (struct sound_maker_t *) sound_maker;
        bool sound_mkr_en = false;
        SOUND_MAKER_VOLUME sound_mkr_vol = SOUND_MAKER_VOLUME_LOW;

        if (sound_mkr->valid == false) {
                return;
        }

        switch (cmd) {
        case SOUND_MAKER_CMD_ENABLE:
                sound_mkr->enabled = true;
                sound_mkr->vol = vol;
                break;
        case SOUND_MAKER_CMD_DISABLE:
                sound_mkr->enabled = false;
                break;
        case SOUND_MAKER_CMD_DELETE:
                sound_mkr->valid = false;
                sound_mkr->enabled = false;
                break;
        }

        /* Enable sound maker if at least one instance is enabled and set the volume to max */
        for (int i = 0; i < SOUND_MAKERS_COUNT_MAX; i++) {
                sound_mkr = &sound_makers[i];
                if (sound_mkr->valid && sound_mkr->enabled) {
                        sound_mkr_en = true;
                        if (sound_mkr->vol > sound_mkr_vol) {
                                sound_mkr_vol = sound_mkr->vol;
                        }
                }
        }

        static const LED_CONTROL_BLINK_RATE led_blink_rates[SOUND_MAKER_VOLUME_HIGH + 1] = {
                LED_CONTROL_BLINK_RATE_DEFAULT,
                LED_CONTROL_BLINK_RATE_LOW,
                LED_CONTROL_BLINK_RATE_MEDIUM,
                LED_CONTROL_BLINK_RATE_HIGH
        };

        led_control_set_mode(sound_mkr_en ? LED_CONTROL_MODE_START_BLINK : LED_CONTROL_MODE_SET_OFF,
                led_blink_rates[vol], LED_CONTROL_BLINK_FOREVER);
}

sound_maker_handle_t sound_maker_init(void)
{
        struct sound_maker_t *sound_mkr;

        sound_mkr = search_for_free_sound_maker();

        if (sound_mkr == NULL) {
                return NULL;
        }

        sound_mkr->valid = true;
        sound_mkr->enabled = false;

        return sound_mkr;
}

void sound_maker_deinit(sound_maker_handle_t sound_maker)
{
        update_sound_maker(sound_maker, SOUND_MAKER_CMD_DELETE, 0);
}

void sound_maker_enable(sound_maker_handle_t sound_maker, SOUND_MAKER_VOLUME vol)
{
        update_sound_maker(sound_maker, SOUND_MAKER_CMD_ENABLE, vol);
}

void sound_maker_disable(sound_maker_handle_t sound_maker)
{
        update_sound_maker(sound_maker, SOUND_MAKER_CMD_DISABLE, 0);
}
