/**
 ****************************************************************************************
 *
 * @file fp_ano.c
 *
 * @brief Accessory Non-Owner service module implementation
 *
 * Copyright (C) 2024-2025 Renesas Electronics Corporation and/or its affiliates.
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

#include <stdio.h>
#include "sdk_defs.h"
#include "fp_core.h"
#include "anos.h"
#include "fp_crypto.h"
#include "fp_fmdn.h"
#include "fp_account_keys.h"

#include "fp_ano.h"

#if (FP_FMDN == 1)

#define MAX_RESPONSE_LENGTH                                       ( 70 )
#define MAX_MANUFACTURER_NAME_LENGTH                              ( 64 )
#define MAX_MODEL_NAME_LENGTH                                     ( 64 )
#define ACCESSORY_CATEGORY_OPERAND_LENGTH                         ( 8 )
#define RECOVERY_KEY_LENGTH                                       ( 8 )

#define PROTOCOL_IMPLEMENTATION_VERSION_MAJOR                     ( 1 )
#define PROTOCOL_IMPLEMENTATION_VERSION_MINOR                     ( 0 )
#define PROTOCOL_IMPLEMENTATION_VERSION_REVISION                  ( 0 )

#define RESPONSE_STATUS_SUCCESS                                   ( 0x0000 )
#define RESPONSE_STATUS_INVALID_STATE                             ( 0x0001 )
#define RESPONSE_STATUS_INVALID_CONFIGURATION                     ( 0x0002 )
#define RESPONSE_STATUS_INVALID_LENGTH                            ( 0x0003 )
#define RESPONSE_STATUS_INVALID_PARAM                             ( 0x0004 )
#define RESPONSE_STATUS_INVALID_COMMAND                           ( 0x0005 )

/* Accessory Information request opcodes */
#define GET_PRODUCT_DATA_OPCODE                                   ( 0x003 )
#define GET_PRODUCT_DATA_RESPONSE_OPCODE                          ( 0x803 )
#define GET_MANUFACTURER_NAME_OPCODE                              ( 0x004 )
#define GET_MANUFACTURER_NAME_RESPONSE_OPCODE                     ( 0x804 )
#define GET_MODEL_NAME_OPCODE                                     ( 0x005 )
#define GET_MODEL_NAME_RESPONSE_OPCODE                            ( 0x805 )
#define GET_ACCESSORY_CATEGORY_OPCODE                             ( 0x006 )
#define GET_ACCESSORY_CATEGORY_RESPONSE_OPCODE                    ( 0x806 )
#define GET_PROTOCOL_IMPLEMENTATION_VERSION_OPCODE                ( 0x007 )
#define GET_PROTOCOL_IMPLEMENTATION_VERSION_RESPONSE_OPCODE       ( 0x807 )
#define GET_ACCESSORY_CAPABILITIES_OPCODE                         ( 0x008 )
#define GET_ACCESSORY_CAPABILITIES_RESPONSE_OPCODE                ( 0x808 )
#define GET_NETWORK_ID_OPCODE                                     ( 0x009 )
#define GET_NETWORK_ID_RESPONSE_OPCODE                            ( 0x809 )
#define GET_FIRMWARE_VERSION_OPCODE                               ( 0x00A )
#define GET_FIRMWARE_VERSION_RESPONSE_OPCODE                      ( 0x80A )
#define GET_BATTERY_TYPE_OPCODE                                   ( 0x00B )
#define GET_BATTERY_TYPE_RESPONSE_OPCODE                          ( 0x80B )
#define GET_BATTERY_LEVEL_OPCODE                                  ( 0x00C )
#define GET_BATTERY_LEVEL_RESPONSE_OPCODE                         ( 0x80C )

/* Accessory Non-Owner Controls request opcodes */
#define CONTROL_SOUND_START_OPCODE                                ( 0x300 )
#define CONTROL_SOUND_STOP_OPCODE                                 ( 0x301 )
#define CONTROL_COMMAND_RESPONSE_OPCODE                           ( 0x302 )
#define CONTROL_SOUND_COMPLETED_OPCODE                            ( 0x303 )
#define CONTROL_GET_OPCODE_IDENTIFIER_OPCODE                      ( 0x404 )
#define CONTROL_GET_OPCODE_IDENTIFIER_RESPONSE_OPCODE             ( 0x405 )

typedef uint8_t (*handler_t)(uint8_t *output);

/* Accessory control handling structure */
typedef struct {
        uint16_t request_opcode;
        handler_t control_handler;
} control_handler_t;

/* Battery levels type */
typedef enum {
        ACCESSORY_BATTERY_LEVEL_FULL,
        ACCESSORY_BATTERY_LEVEL_MEDIUM,
        ACCESSORY_BATTERY_LEVEL_LOW,
        ACCESSORY_BATTERY_LEVEL_CRITICAL
} accessory_battery_level_t;

/* Accessory Non-Owner service module context */
__RETAINED static struct {
        uint16_t conn_idx;
        bool accessory_is_ringing;  /* Accessory ringing was initiated by non-owner device */
} ano_ctx;

/* Instance for Accessory Non-Owner service */
__RETAINED static ble_service_t *fp_anos;

/* Accessory Information response operand handlers */
static uint8_t get_product_data_operand(uint8_t *operand);
static uint8_t get_manufacturer_name_operand(uint8_t *operand);
static uint8_t get_model_name_operand(uint8_t *operand);
static uint8_t get_accessory_category_operand(uint8_t *operand);
static uint8_t get_protocol_implementation_version_operand(uint8_t *operand);
static uint8_t get_accessory_capabitlities_operand(uint8_t *operand);
static uint8_t get_network_id_operand(uint8_t *operand);
static uint8_t get_firmware_version_operand(uint8_t *operand);
static uint8_t get_battery_type_operand(uint8_t *operand);
static uint8_t get_battery_level_operand(uint8_t *operand);

/* Accessory Non-Owner Control response handlers */
static uint8_t control_get_identifier_response(uint8_t *response);
static uint8_t control_start_sound_response(uint8_t *response);
static uint8_t control_stop_sound_response(uint8_t *response);

/* Array indexed with Accessory Information request opcodes */
static const handler_t accessory_information_handlers[] =
{
        NULL,                                           /* opcode 0x000 unavailable */
        NULL,                                           /* opcode 0x001 unavailable */
        NULL,                                           /* opcode 0x002 unavailable */
        get_product_data_operand,                       /* GET_PRODUCT_DATA_OPCODE */
        get_manufacturer_name_operand,                  /* GET_MANUFACTURER_NAME_OPCODE */
        get_model_name_operand,                         /* GET_MODEL_NAME_OPCODE */
        get_accessory_category_operand,                 /* GET_ACCESSORY_CATEGORY_OPCODE */
        get_protocol_implementation_version_operand,    /* GET_PROTOCOL_IMPLEMENTATION_VERSION_OPCODE */
        get_accessory_capabitlities_operand,            /* GET_ACCESSORY_CAPABILITIES_OPCODE */
        get_network_id_operand,                         /* GET_NETWORK_ID_OPCODE */
        get_firmware_version_operand,                   /* GET_FIRMWARE_VERSION_OPCODE */
        get_battery_type_operand,                       /* GET_BATTERY_TYPE_OPCODE */
        get_battery_level_operand,                      /* GET_BATTERY_LEVEL_OPCODE */
};

/* Array indexed with Accessory Non-Owner Control request opcodes */
static const control_handler_t accessory_control_handlers[] =
{
        { CONTROL_GET_OPCODE_IDENTIFIER_OPCODE, control_get_identifier_response },
        { CONTROL_SOUND_START_OPCODE, control_start_sound_response },
        { CONTROL_SOUND_STOP_OPCODE, control_stop_sound_response }
};

/* Gets product data */
static uint8_t get_product_data_operand(uint8_t *operand)
{
        /* Product data is an 8-byte value: Fast Pair model ID padded with zeros */
        memset(operand, 0, 8);

        operand[7] = (uint8_t) FP_FMDN_PRODUCT_DATA;
        operand[6] = (uint8_t) (FP_FMDN_PRODUCT_DATA >> 1);
        operand[5] = (uint8_t) (FP_FMDN_PRODUCT_DATA >> 2);

        return 8;
}

/* Gets manufacturer name */
static uint8_t get_manufacturer_name_operand(uint8_t *operand)
{
        uint8_t manuf_name_len = (sizeof(FP_FMDN_MANUFACTURER_NAME) > MAX_MANUFACTURER_NAME_LENGTH) ?
                                 MAX_MANUFACTURER_NAME_LENGTH : sizeof(FP_FMDN_MANUFACTURER_NAME);

        memcpy(operand, FP_FMDN_MANUFACTURER_NAME, manuf_name_len);
        return manuf_name_len;
}

/* Gets model name */
static uint8_t get_model_name_operand(uint8_t *operand)
{
        uint8_t model_name_len = (sizeof(FP_FMDN_MODEL_NAME) > MAX_MODEL_NAME_LENGTH) ?
                                 MAX_MODEL_NAME_LENGTH : sizeof(FP_FMDN_MODEL_NAME);

        memcpy(operand, FP_FMDN_MODEL_NAME, model_name_len);
        return model_name_len;
}

/* Gets accessory category */
static uint8_t get_accessory_category_operand(uint8_t *operand)
{
        memset(operand, 0, ACCESSORY_CATEGORY_OPERAND_LENGTH);
        operand[0] = FP_FMDN_ACCESSORY_CATEGORY;
        return ACCESSORY_CATEGORY_OPERAND_LENGTH;
}

/* Gets protocol implementation version */
static uint8_t get_protocol_implementation_version_operand(uint8_t *operand)
{
        uint16_t major = PROTOCOL_IMPLEMENTATION_VERSION_MAJOR;

        operand[0] = PROTOCOL_IMPLEMENTATION_VERSION_REVISION;
        operand[1] = PROTOCOL_IMPLEMENTATION_VERSION_MINOR;
        memcpy(operand + 2, &major, sizeof(uint16_t));
        return 4;
}

/* Gets accessory capabilities */
static uint8_t get_accessory_capabitlities_operand(uint8_t *operand)
{
        uint32_t capabilities = FP_FMDN_ACCESSORY_CAPABILITIES;

        memcpy(operand, &capabilities, sizeof(capabilities));
        return sizeof(capabilities);
}

/* Gets network ID */
static uint8_t get_network_id_operand(uint8_t *operand)
{
        operand[0] = FP_FMDN_NETWORK_ID;
        return sizeof(uint8_t);
}

/* Gets firmware version */
static uint8_t get_firmware_version_operand(uint8_t *operand)
{
        uint16_t major = FP_FMDN_FW_VERSION_MAJOR;

        operand[0] = FP_FMDN_FW_VERSION_REVISION;
        operand[1] = FP_FMDN_FW_VERSION_MINOR;
        memcpy(operand + 2, &major, sizeof(uint16_t));
        return 4;
}

/* Gets battery type */
static uint8_t get_battery_type_operand(uint8_t *operand)
{
        operand[0] = FP_FMDN_BATTERY_TYPE;
        return sizeof(uint8_t);
}

/* Gets level of the main battery */
static accessory_battery_level_t get_battery_level(void)
{
        accessory_battery_level_t battery_level;

#if (FP_FMDN_BATTERY_TYPE == FP_FMDN_BATTERY_TYPE_POWERED)
        battery_level = ACCESSORY_BATTERY_LEVEL_FULL;
#else
        fp_battery_info_t *info = fp_get_battery_information();
        if (info[0].level <= FP_FMDN_BATTERY_LEVEL_CRITICAL) {
                battery_level = ACCESSORY_BATTERY_LEVEL_CRITICAL;
        } else if (info[0].level <= FP_FMDN_BATTERY_LEVEL_LOW) {
                battery_level = ACCESSORY_BATTERY_LEVEL_LOW;
        } else if (info[0].level <= FP_FMDN_BATTERY_LEVEL_MEDIUM) {
                battery_level = ACCESSORY_BATTERY_LEVEL_MEDIUM;
        } else {
                battery_level = ACCESSORY_BATTERY_LEVEL_FULL;
        }
#endif
        return battery_level;
}

/* Gets battery level */
static uint8_t get_battery_level_operand(uint8_t *operand)
{
        operand[0] = get_battery_level();
        return sizeof(uint8_t);
}

/* Handles command response */
static uint8_t handle_command_response(uint16_t request_opcode, uint16_t request_status, uint8_t *response)
{
        uint8_t length;
        uint16_t command_response_opcode = CONTROL_COMMAND_RESPONSE_OPCODE;

        if (request_status != RESPONSE_STATUS_SUCCESS) {
                FP_FMDN_LOG_PRINTF("Invalid Command, status=%x\r\n", request_status);
        }
        memcpy(response, &command_response_opcode, sizeof(command_response_opcode));
        length = sizeof(command_response_opcode);
        memcpy(response + length, &request_opcode, sizeof(request_opcode));
        length += sizeof(request_opcode);
        memcpy(response + length, &request_status, sizeof(request_status));
        length += sizeof(request_status);

        return length;
}

/* Handles Accessory Information requested opcodes */
static uint8_t handle_accessory_information_opcodes(uint16_t request_opcode, uint8_t *response)
{
        uint16_t response_opcode;
        uint8_t length;

        if (accessory_information_handlers[request_opcode]) {
                response_opcode = request_opcode | 0x800;  /* opcode for response */
                memcpy(response, &response_opcode, sizeof(response_opcode));
                length = sizeof(response_opcode);
                length += accessory_information_handlers[request_opcode](response +
                                                                           sizeof(response_opcode));
        } else {
                length = handle_command_response(request_opcode, RESPONSE_STATUS_INVALID_COMMAND,
                                                                                          response);
        }

        return length;
}

/* Gets device identifier */
static uint8_t control_get_identifier_response(uint8_t *response)
{
        uint16_t response_opcode;
        uint8_t length;
        uint8_t recovery_key[RECOVERY_KEY_LENGTH];
        uint8_t output[FP_CRYPTO_SHA256_BYTES_LEN];

        if (!fp_fmdn_is_user_consent_mode() || (fp_acc_keys_get_ephemeral_identity_key() == NULL)) {
             /* No user consent or device not FMDN provisioned */
             FP_FMDN_LOG_PRINTF("No user consent or device not FMDN provisioned\r\n");
             return handle_command_response(CONTROL_GET_OPCODE_IDENTIFIER_OPCODE,
                                                         RESPONSE_STATUS_INVALID_COMMAND, response);
        }

        response_opcode = CONTROL_GET_OPCODE_IDENTIFIER_RESPONSE_OPCODE;  /* opcode for response */
        memcpy(response, &response_opcode, sizeof(response_opcode));
        length = sizeof(response_opcode);

        /* 10 bytes of current EID */
        memcpy(response + length, fp_fmdn_get_eid() , 10);
        length += 10;

        /* 8 bytes of HMAC-SHA256(recovery key, the 10-byte truncated current ephemeral identifier) */
        fp_fmdn_get_recovery_key(recovery_key);
        fp_crypto_hmac_sha256(fp_fmdn_get_eid(), 10, recovery_key, RECOVERY_KEY_LENGTH, output);
        memcpy(response + length, output , 8);
        length += 8;

        return length;
}

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/* Confirms the completion of the sound event */
static void play_sound_completed(void)
{
        uint16_t response_opcode = CONTROL_SOUND_COMPLETED_OPCODE;

        if (!ano_ctx.accessory_is_ringing) {
                return;
        }

        FP_FMDN_LOG_PRINTF("Sound event completed\r\n");

        ano_ctx.accessory_is_ringing = false;

        anos_indicate_response(fp_anos, ano_ctx.conn_idx, (uint8_t *) &response_opcode,
                sizeof(response_opcode));
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

/* Handles play sound action and starts playing sound */
static uint8_t control_start_sound_response(uint8_t *response)
{
        uint16_t response_status;

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (FP_FMDN_ACCESSORY_CAPABILITIES & FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND) {
                response_status = RESPONSE_STATUS_INVALID_STATE;
                if (!ano_ctx.accessory_is_ringing) {
                        int err = fp_fmdn_start_ringing(FP_FMDN_NON_OWNER_PLAY_SOUND_TIMEOUT_MS,
                                        FP_RING_COMP_VOLUME_HIGH, play_sound_completed);
                        if (err == 0) {
                                ano_ctx.accessory_is_ringing = true;
                                response_status = RESPONSE_STATUS_SUCCESS;
                        }
                }
        } else {
                response_status = RESPONSE_STATUS_INVALID_COMMAND;
        }
#else
        response_status = RESPONSE_STATUS_INVALID_COMMAND;
#endif

        return handle_command_response(CONTROL_SOUND_START_OPCODE, response_status, response);
}

/* Handles stop sound action and stops playing sound */
static uint8_t control_stop_sound_response(uint8_t *response)
{
        uint16_t response_status;

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (FP_FMDN_ACCESSORY_CAPABILITIES & FP_FMDN_ACCESSORY_CAPABILITY_PLAY_SOUND) {
                if (!fp_fmdn_is_ringing()) {
                        ano_ctx.accessory_is_ringing = false;
                }
                if (ano_ctx.accessory_is_ringing) {
                        uint16_t response_opcode;
                        uint8_t length;

                        ano_ctx.accessory_is_ringing = false;

                        fp_fmdn_stop_ringing(FP_FMDN_RING_REQUESTED_STOPPED);

                        response_opcode = CONTROL_SOUND_COMPLETED_OPCODE;
                        memcpy(response, &response_opcode, sizeof(response_opcode));
                        length = sizeof(response_opcode);

                        return length;
                } else {
                        response_status = RESPONSE_STATUS_INVALID_STATE;
                }
        } else {
                response_status = RESPONSE_STATUS_INVALID_COMMAND;
        }
#else
        response_status = RESPONSE_STATUS_INVALID_COMMAND;
#endif
        return handle_command_response(CONTROL_SOUND_STOP_OPCODE, response_status, response);
}

/* Handles Accessory Non-Owner Control requested opcodes */
static uint8_t handle_accessory_control_opcodes(uint16_t request_opcode, uint8_t *response)
{
        uint8_t length = 0, i;

        for (i = 0; i < ARRAY_LENGTH(accessory_control_handlers); i++) {
                if (accessory_control_handlers[i].request_opcode == request_opcode) {
                        if (accessory_control_handlers[i].control_handler) {
                                length = accessory_control_handlers[i].control_handler(response);
                        }
                        break;
                }
        }
        if (i == ARRAY_LENGTH(accessory_control_handlers)) {
                /* Unsupported opcode */
                length = handle_command_response(request_opcode, RESPONSE_STATUS_INVALID_COMMAND, response);
        }

        return length;
}

/* Handles write request to accessory */
static void write_request(ble_service_t *svc, uint16_t conn_idx, uint16_t opcode)
{
        uint8_t response[MAX_RESPONSE_LENGTH];
        uint8_t length;

        FP_FMDN_LOG_PRINTF("ano>>> opcode=%x availability=%d\r\n", opcode, fp_fmdn_is_utpm_active());

        ano_ctx.conn_idx = conn_idx;

        if (!fp_fmdn_is_utpm_active()) {
                /* Invalid command handling */
                length = handle_command_response(opcode, RESPONSE_STATUS_INVALID_COMMAND, response);
        } else if (opcode < ARRAY_LENGTH(accessory_information_handlers)) {
                length = handle_accessory_information_opcodes(opcode, response);
        } else {
                length = handle_accessory_control_opcodes(opcode, response);
        }

        /* Indicate the response */
        anos_indicate_response(svc, conn_idx, response, length);
}

void fp_ano_init(void)
{
        /* Initialize Accessory Non-Owner service */
        fp_anos = anos_init(write_request);
        ano_ctx.accessory_is_ringing = false;
}

void fp_ano_deinit(void)
{
        ble_service_remove(fp_anos);
        ble_service_cleanup(fp_anos);

        fp_anos = NULL;
}
#endif /* FP_FMDN */
