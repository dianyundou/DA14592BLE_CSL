/**
 ****************************************************************************************
 *
 * @file ble_gattc_util.c
 *
 * @brief BLE GATT Client utilities API
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

#include <stdint.h>
#include <string.h>
#include "osal.h"
#include "ble_bufops.h"
#include "ble_common.h"
#include "ble_gattc.h"
#include "ble_uuid.h"

struct find_item_state {
        uint16_t index;
        bool has_uuid;
        att_uuid_t uuid;
};

__RETAINED static struct {
        const ble_evt_gattc_browse_svc_t *evt;

        struct find_item_state c_state; /* state for 'find_characteristic' */
        struct find_item_state d_state; /* state for 'find_descriptor' */
} find_state;

static void reset_find_state(void)
{
        find_state.c_state.index = 0;
        find_state.c_state.has_uuid = false;
        find_state.d_state.index = 0;
        find_state.d_state.has_uuid = false;
}

void ble_gattc_util_find_init(const ble_evt_gattc_browse_svc_t *evt)
{
        find_state.evt = evt;

        reset_find_state();
}

const gattc_item_t *ble_gattc_util_find_characteristic(const att_uuid_t *uuid)
{
        if (!find_state.evt) {
                return NULL;
        }

        /* Reset state if parameter has changed */
        if (((uuid != NULL) != find_state.c_state.has_uuid) ||
                                        (uuid && !ble_uuid_equal(uuid, &find_state.c_state.uuid))) {
                reset_find_state();

                find_state.c_state.has_uuid = uuid;
                if (find_state.c_state.has_uuid) {
                        memcpy(&find_state.c_state.uuid, uuid, sizeof(att_uuid_t));
                }
        }

        for (; find_state.c_state.index < find_state.evt->num_items; find_state.c_state.index++) {
                const gattc_item_t *item = &find_state.evt->items[find_state.c_state.index];

                if (item->type != GATTC_ITEM_TYPE_CHARACTERISTIC) {
                        continue;
                }

                if (!uuid || ble_uuid_equal(uuid, &item->uuid)) {
                        /* Move to next item, we'll start next search from there */
                        find_state.c_state.index++;
                        find_state.d_state.index = find_state.c_state.index;

                        return item;
                }
        }

        return NULL;
}

const gattc_item_t *ble_gattc_util_find_descriptor(const att_uuid_t *uuid)
{
        if (!find_state.evt) {
                return NULL;
        }

        /* Reset state if parameter has changed */
        if (((uuid != NULL) != find_state.d_state.has_uuid) ||
                                        (uuid && !ble_uuid_equal(uuid, &find_state.d_state.uuid))) {
                /* we start for last found characteristic item index */
                find_state.d_state.index = find_state.c_state.index;

                find_state.d_state.has_uuid = uuid;
                if (find_state.d_state.has_uuid) {
                        memcpy(&find_state.d_state.uuid, uuid, sizeof(att_uuid_t));
                }
        }

        for (; find_state.d_state.index < find_state.evt->num_items; find_state.d_state.index++) {
                const gattc_item_t *item = &find_state.evt->items[find_state.d_state.index];

                if (item->type == GATTC_ITEM_TYPE_CHARACTERISTIC) {
                        /* characteristic found - no more descriptors! */
                        return NULL;
                }

                if (!uuid || ble_uuid_equal(uuid, &item->uuid)) {
                        /* Move to next item, we'll start next search from there */
                        find_state.d_state.index++;

                        return item;
                }
        }

        return NULL;
}
