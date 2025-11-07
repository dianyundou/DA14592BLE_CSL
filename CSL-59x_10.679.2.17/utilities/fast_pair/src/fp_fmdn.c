/**
 ****************************************************************************************
 *
 * @file fp_fmdn.c
 *
 * @brief Google Fast Pair Find My Device Network (FMDN) extension module implementation
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "osal.h"
#include "fp_core.h"
#include "fast_pair.h"
#include "fp_notifications.h"
#include "fp_crypto.h"
#include "fp_account_keys.h"
#include "sys_timer.h"
#include "fp_utils.h"
#include "gfps.h"
#include "fp_procedure.h"
#include "fp_ano.h"
#include "uECC_vli.h"
#include "fp_ring_comp.h"
#include "fp_motion_detection.h"
#include "fp_ble.h"

#include "fp_fmdn.h"

#if (FP_FMDN == 1)

#define ELLIPTIC_CURVE                                  (uECC_secp160r1())
#define E2EE_EID_LENGTH                                 (20)

#define PROTOCOL_VERSION                                (1)
#define ROTATION_PERIOD_EXPONENT                        (10)
#define ID_ROTATION_TIME_SEC                            (1024)  /* sec */
#define ID_ROTATION_RAND_OFFSET_MIN                     (1)     /* sec */
#define ID_ROTATION_RAND_OFFSET_MAX                     (204)   /* sec */
#define UTP_MODE_TIMEOUT_SEC                            (24 * 60 * 60)  /* 24h */
#define PROVISIONING_TIMEOUT_MS                         (5 * 60 * 1000) /* 5 min */
#define MAX_RINGING_TIMEOUT_DS                          (10 * 60 * 10)  /* 10 min */

#define ONE_TIME_AUTHENTICATION_KEY_LENGTH              (8)
#define NONCE_BYTES_NUM                                 (8)
#define BEACON_MINIMUM_REQ_RES_LENGTH                   (10)  /* without additional data */
#define BEACON_SET_EIK_REQ_LENGTH                       (BEACON_MINIMUM_REQ_RES_LENGTH + \
                                                         FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH)
#define BEACON_SET_EIK_WITH_HASH_REQ_LENGTH             (BEACON_SET_EIK_REQ_LENGTH + 8)
#define RECOVERY_KEY_LENGTH                             (ONE_TIME_AUTHENTICATION_KEY_LENGTH)
#define RING_KEY_LENGTH                                 (ONE_TIME_AUTHENTICATION_KEY_LENGTH)
#define UTPM_KEY_LENGTH                                 (ONE_TIME_AUTHENTICATION_KEY_LENGTH)
#define RING_ALL                                        (0xFF)
#define RING_COMPONENTS_MASK                            ((1 << FP_FMDN_RING_COMPONENTS_NUM) - 1)
#define E2EE_EID_FRAME_TYPE                             (0x40)
#define E2EE_EID_FRAME_UTPM_TYPE                        (0x41)  /* unwanted tracking protection mode on */
#define CURVE_MAX_WORDS                                 (8)
#define PROTOCOL_VERSION_LENGTH                         (1)
#define DATA_ID_LENGTH                                  (1)
#define DATA_LENGTH_LENGTH                              (1)
#define ADDITIONAL_DATA_MAX_LENGTH                      (40)
#define HASH_FLAGS_UTPM_MASK                            (0x01)
#define HASH_FLAGS_BATT_LEVEL_MASK                      (0x03 << 1)

#define LOCATION_ENABLED_ADV_FRAME                      (0)

/* Field getters for beacon request/response messages */
#define GET_ONE_TIME_AUTH_KEY(msg)                      (&(msg[2]))
#define GET_DATA_ID(msg)                                (msg[0])
#define GET_DATA_LENGTH(msg)                            (msg[1])
#define GET_ADD_DATA(msg)                               (&(msg[10]))
/* Field getters for ringing request message */
#define GET_RING_OPERATION(msg)                         (msg[BEACON_MINIMUM_REQ_RES_LENGTH])
#define GET_RING_TIMEOUT_HIGH(msg)                      (msg[BEACON_MINIMUM_REQ_RES_LENGTH + 1])
#define GET_RING_TIMEOUT_LOW(msg)                       (msg[BEACON_MINIMUM_REQ_RES_LENGTH + 2])
#define GET_RING_VOLUME(msg)                            (msg[BEACON_MINIMUM_REQ_RES_LENGTH + 3])
/* Field getters for unwanted tracking protection mode request message */
#define GET_UTPM_CONTROL_FLAGS(msg)                     (msg[BEACON_MINIMUM_REQ_RES_LENGTH])
#define SKIP_RINGING_AUTH_FLAG                          (0x01)

/* Data IDs for beacon actions */
typedef enum {
        DATA_ID_READ_BEACON_PARAMETERS,
        DATA_ID_READ_PROVISIONING_STATE,
        DATA_ID_SET_EPHEMERAL_IDENTITY_KEY,
        DATA_ID_CLEAR_EPHEMERAL_IDENTITY_KEY,
        DATA_ID_READ_EPHEMERAL_IDENTITY_KEY,
        DATA_ID_RING,
        DATA_ID_READ_RINGING_STATE,
        DATA_ID_ACTIVATE_UNWANTED_TRACKING_PROTECTION,
        DATA_ID_DEACTIVATE_UNWANTED_TRACKING_PROTECTION,
} data_id_t;

/* Battery levels type */
typedef enum {
        BATTERY_LEVEL_UNSUPPORTED,
        BATTERY_LEVEL_NORMAL,
        BATTERY_LEVEL_LOW,
        BATTERY_LEVEL_CRITICAL
} battery_level_t;

/* ID rotation status */
typedef enum {
        ID_ROTATION_NONE,
        ID_ROTATION_PENDING,
        ID_ROTATION_IN_PROGRESS
} id_rotation_stat_t;

/* 8-byte key type */
typedef enum {
        RECOVERY_KEY = 0x01,
        RING_KEY = 0x02,
        UTPM_KEY = 0x03
} rru_key_t;

/* Ringing setup type */
typedef struct {
        uint8_t op; /* components that have been requested to ring */
        FP_RING_COMP_VOLUME volume;
        uint16_t timeout_ds;
} ring_setup_t;

/* Ringing state type */
typedef struct {
        ring_setup_t setup;
        uint32_t start_time_ms;
} ringing_state_t;

/* Google FMDN module context */
__FP_RETAINED static struct {
        uint8_t last_nonce[NONCE_BYTES_NUM];
        uint8_t E2EE_EID[E2EE_EID_LENGTH];
        uint8_t hashed_flags;
        uint8_t sha256_r_lsb;
        ble_service_t *svc;
        uint16_t ring_conn_idx; /* connection index the ringing request of which currently being served */
        gap_adv_ad_struct_t adv_struct;
        bool regenerate_adv_struct;
        bool utpm; /* unwanted tracking prevention mode */
        bool skip_ringing_authentication;
        bool utpm_addr_rotation_pending;
        ACCURATE_OS_TIMER provisioning_tim;
        ACCURATE_OS_TIMER utpm_tim;
        OS_TIMER user_consent_tim;
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        OS_TIMER ring_tim;
#endif
        ACCURATE_OS_TIMER id_rotation_tim;
        id_rotation_stat_t id_rotation_stat;
        bool user_consent;
        bool eid_cleared;
        ringing_state_t ring_state;
        fp_beacon_time_cb_t beacon_time_cb;
        fp_fmdn_ringing_complete_cb_t ringing_complete_cb;
} fmdn_ctx;

static void compute_e2ee_eid(uint32_t beacon_time_counter, const uint8_t *identity);
static bool verify_provisioning_and_key_matching(const uint8_t *request, rru_key_t key_type,
        uint8_t *key);
static void deactivate_utpm(void);
static void control_id_rotation(bool enable);

#if (FP_FMDN_LOG_ENABLE == 1)
#define FMDN_PRINT_HEX(name, data, length) print_hex_buffer(name, data, length)

/* Prints the buffer as hex numbers */
static void print_hex_buffer(char *name, const uint8_t *data, uint8_t length)
{
        uint8_t i;

        FP_FMDN_LOG_PRINTF("%s: ", name);
        for (i = 0; i < length; i++) {
                FP_FMDN_LOG_PRINTF("%02X", data[i]);
        }
        FP_FMDN_LOG_PRINTF("\r\n");
}
#else
#define FMDN_PRINT_HEX(name, data, length)
#endif /* FP_FMDN_LOG_ENABLE */

/* Sends notification as beacon actions response */
static int notify_beacon_actions_response(ble_service_t *svc, uint16_t conn_idx, uint8_t *response)
{
        uint8_t length_field = response[1];

        return gfps_notify_beacon_actions(svc, conn_idx, response, length_field + 2 /* data ID */);
}

/* Prepares input for calculating one-time authentication key */
static uint8_t prepare_input_for_key(const uint8_t *msg, uint8_t *input)
{
        uint8_t input_length = 0;
        uint8_t add_data_length = GET_DATA_LENGTH(msg) - ONE_TIME_AUTHENTICATION_KEY_LENGTH;

        input[0] = PROTOCOL_VERSION;
        input_length++;
        memcpy(input + input_length, fmdn_ctx.last_nonce, NONCE_BYTES_NUM);
        input_length += NONCE_BYTES_NUM;
        input[input_length] = GET_DATA_ID(msg);
        input_length++;
        input[input_length] = GET_DATA_LENGTH(msg);
        input_length++;

        if (add_data_length) {
                memcpy(&(input[input_length]), GET_ADD_DATA(msg), add_data_length);
                input_length += add_data_length;
        }
        return input_length;
}

/* Calculates one-time authentication key */
static void calculate_one_time_key(const uint8_t *msg, const uint8_t *key,
        uint8_t key_length, uint8_t *one_time_key, bool for_response)
{
        uint8_t output[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t input[PROTOCOL_VERSION_LENGTH + NONCE_BYTES_NUM + DATA_ID_LENGTH +
                      DATA_LENGTH_LENGTH + ADDITIONAL_DATA_MAX_LENGTH + 1];
        uint8_t input_length = prepare_input_for_key(msg, input);

        if (for_response) {
                input[input_length] = 0x01;
                input_length++;
        }

        fp_crypto_hmac_sha256(input, input_length, key, key_length, output);

        memcpy(one_time_key, output, ONE_TIME_AUTHENTICATION_KEY_LENGTH);
}

/* Calculates one-time authentication key from request */
static void calculate_one_time_key_from_request(const uint8_t *request, const uint8_t *key,
        uint8_t key_length, uint8_t *one_time_key)
{
        calculate_one_time_key(request, key, key_length, one_time_key, false);
}

/* Calculates one-time authentication key for response */
static void calculate_one_time_key_for_response(uint8_t *response, const uint8_t *key,
        uint8_t key_length)
{
        calculate_one_time_key(response, key, key_length, GET_ONE_TIME_AUTH_KEY(response), true);
}

/* Compares one-time authentication key from request with given key */
static bool compare_keys(const uint8_t *request, const uint8_t *key, uint8_t key_length)
{
        uint8_t one_time_key[ONE_TIME_AUTHENTICATION_KEY_LENGTH];

        calculate_one_time_key_from_request(request, key, key_length, one_time_key);
        /* One-time authentication keys match? */
        return ((memcmp(GET_ONE_TIME_AUTH_KEY(request), one_time_key,
                        ONE_TIME_AUTHENTICATION_KEY_LENGTH) == 0) ? true : false);
}

/* Verifies one-time authentication key with proper stored key */
static bool verify_one_time_authentication_key(const uint8_t *request, bool use_owner_key,
        uint8_t *key_index)
{
        uint8_t i;

        if (use_owner_key) {
                *key_index = 0;
                return compare_keys(request, fp_acc_keys_get_owner_key(),
                        FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
        }

        for (i = 0; i < fp_acc_keys_get_keys_count(); i++) {
                if (compare_keys(request, fp_acc_keys_get_key(i), FP_ACC_KEYS_ACCOUNT_KEY_LENGTH)) {
                        break;
                }
        }
        if (key_index) {
                *key_index = i;
        }

        if (i == fp_acc_keys_get_keys_count()) {
                return false;
        }

        return true;
}

/* Updates last nonce, as required for each read operation */
static void update_last_nonce(void)
{
        uint8_t i = 0;

        for (i = 0; i < NONCE_BYTES_NUM; i += 2) {
                uint16_t rand_val = fp_crypto_get_salt();
                fmdn_ctx.last_nonce[i] = rand_val;
                fmdn_ctx.last_nonce[i + 1] = rand_val >> 8;
        }

        FMDN_PRINT_HEX("Nonce", fmdn_ctx.last_nonce, NONCE_BYTES_NUM);
}

/* Produces beacon state response */
static void produce_beacon_state_response(uint8_t *response, uint8_t key_index)
{
        uint8_t add_data[FP_CRYPTO_AES_BLOCK_LENGTH];
        uint32_t current_time = fmdn_ctx.beacon_time_cb();

        FP_FMDN_LOG_PRINTF("State response Time: %lu\r\n", current_time);
        /* Clock value is in big endian */
        reverse_byte_order((uint8_t *) &current_time, sizeof(uint32_t));

        response[0] = DATA_ID_READ_BEACON_PARAMETERS;
        response[1] = ONE_TIME_AUTHENTICATION_KEY_LENGTH + FP_CRYPTO_AES_BLOCK_LENGTH;
        add_data[0] = FP_FMDN_CALIBRATED_TX_POWER_LEVEL;
        memcpy(&(add_data[1]), &current_time, sizeof(current_time));
        add_data[5] = FP_FMDN_ELLIPTIC_CURVE_SECP160R1;  /* Only SECP160R1 is currently supported */
        add_data[6] = FP_FMDN_RING_COMPONENTS_NUM;
        add_data[7] = FP_FMDN_RINGING_CAPABILITIES;
        memset(&(add_data[8]), 0, 8);

        fp_crypto_aes_ecb(fp_acc_keys_get_key(key_index), FP_CRYPTO_AES_BLOCK_LENGTH,
                FP_CRYPTO_OP_ENCRYPT, 1, add_data, &response[ONE_TIME_AUTHENTICATION_KEY_LENGTH + 2]);
        calculate_one_time_key_for_response(response, fp_acc_keys_get_key(key_index),
                FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
}

/* Produces provisioning state response */
static void produce_provisioning_state_response(const uint8_t *request, uint8_t *response,
        uint8_t key_index)
{
        uint8_t acc_key_index;

        response[0] = DATA_ID_READ_PROVISIONING_STATE;
        response[1] = 1 + ONE_TIME_AUTHENTICATION_KEY_LENGTH;
        response[10] = (verify_one_time_authentication_key(request, true, &acc_key_index) ? 0x02 : 0x00);
        if (fp_acc_keys_get_ephemeral_identity_key()) {
                response[1] += E2EE_EID_LENGTH;
                response[10] += 0x01;
                FMDN_PRINT_HEX("EID:", fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH);
                memcpy(&(response[11]), fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH);
        }
        calculate_one_time_key_for_response(response, fp_acc_keys_get_key(key_index),
                FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
}

/* Produces generic beacon action response */
static void produce_generic_response(uint8_t* response, const uint8_t *key, uint8_t key_length,
        data_id_t data_id)
{
        response[0] = data_id;
        response[1] = ONE_TIME_AUTHENTICATION_KEY_LENGTH;
        calculate_one_time_key_for_response(response, key, key_length);
}

/* Verifies if received ephemeral identity key matches the stored one */
static bool verify_eik_match(const uint8_t *eik_hash)
{
        uint8_t hash[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t *identity_key;
        uint8_t EIK_nonce[FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH + NONCE_BYTES_NUM];

        identity_key = fp_acc_keys_get_ephemeral_identity_key();
        if (identity_key == NULL) {
                return false;
        }
        memcpy(EIK_nonce, identity_key, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);
        memcpy(EIK_nonce + FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH, fmdn_ctx.last_nonce,
                NONCE_BYTES_NUM);
        /* SHA256 (Ephemeral identity key || the last nonce) */
        fp_crypto_sha256(hash, EIK_nonce, sizeof(EIK_nonce));
        /* EIK hash match? */
        return (memcmp(eik_hash, hash, 8) ? false : true);
}

/* Rotates ephemeral ID */
static void rotate_ephemeral_id(void)
{
        uint8_t *identity_key = fp_acc_keys_get_ephemeral_identity_key();

        if (identity_key) {
                compute_e2ee_eid(fmdn_ctx.beacon_time_cb(), identity_key);
        }
}

/* Handles read beacon state request */
static fast_pair_error_t handle_read_beacon_state(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH + FP_CRYPTO_AES_BLOCK_LENGTH];
        uint8_t acc_key_index;

        if (length != BEACON_MINIMUM_REQ_RES_LENGTH) {
                return ATT_ERROR_INVALID_VALUE;
        }

        if (!verify_one_time_authentication_key(request, false, &acc_key_index)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

#if (FP_LOCATOR_TAG == 1)
        /* After first invocation of read beacon parameters, provisioning is considered as started */
        if (fp_get_fmdn_provisioning_state() == FP_FMDN_PROVISIONING_INITIATING) {
                fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STARTED);
        }
#endif

        fp_set_authenticated_conn(conn_idx);

        /* Key match, so notify */
        produce_beacon_state_response(response, acc_key_index);
        gfps_notify_beacon_actions(svc, conn_idx, response, sizeof(response));

        return ATT_ERROR_OK;
}

/* Handles read provisioning state request */
static fast_pair_error_t handle_read_provisioning_state(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH + 1 + E2EE_EID_LENGTH];
        uint8_t acc_key_index;

        if (length != BEACON_MINIMUM_REQ_RES_LENGTH) {
                return ATT_ERROR_INVALID_VALUE;
        }

        if (!verify_one_time_authentication_key(request, false, &acc_key_index)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        /* Key match, so notify */
        produce_provisioning_state_response(request, response, acc_key_index);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Handles set ephemeral identity key request */
static fast_pair_error_t handle_set_ephemeral_identity_key(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t identity_key[FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH];
        uint8_t acc_key_index;
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH];
        int ret;

        if ((length != BEACON_SET_EIK_REQ_LENGTH) &&
            (length != BEACON_SET_EIK_WITH_HASH_REQ_LENGTH)) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!verify_one_time_authentication_key(request, true, &acc_key_index)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }
        if (length == BEACON_SET_EIK_WITH_HASH_REQ_LENGTH) {
                if (!verify_eik_match(GET_ADD_DATA(request) + 32)) {
                        return ATT_ERROR_UNAUTHENTICATED;
                }
        } else if (fp_acc_keys_get_ephemeral_identity_key()) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_crypto_aes_ecb(fp_acc_keys_get_owner_key(), FP_CRYPTO_AES_BLOCK_LENGTH,
                FP_CRYPTO_OP_DECRYPT, 2, GET_ADD_DATA(request), identity_key);
        ret = fp_acc_keys_set_ephemeral_identity_key(identity_key);
        FP_CHECK_ERROR(ret);

        fp_set_authenticated_conn(conn_idx);

        rotate_ephemeral_id();

        /* Restart periodic ID rotation so that EID is always new in each ID rotation */
        control_id_rotation(true);

        fp_restart_advertise();

        /* Key match, so notify */
        produce_generic_response(response, fp_acc_keys_get_key(acc_key_index),
                FP_ACC_KEYS_ACCOUNT_KEY_LENGTH, DATA_ID_SET_EPHEMERAL_IDENTITY_KEY);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Handles clear ephemeral identity key request */
static fast_pair_error_t handle_clear_ephemeral_identity_key(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t acc_key_index;
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH];
        int ret;

        if (length != (BEACON_MINIMUM_REQ_RES_LENGTH + 8)) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!verify_one_time_authentication_key(request, true, &acc_key_index)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }
        if (!verify_eik_match(&(request[BEACON_MINIMUM_REQ_RES_LENGTH]))) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        /* Deactivate unwanted tracking protection mode if it is active */
        deactivate_utpm();

        ret = fp_acc_keys_clear_ephemeral_identity_key();
        FP_CHECK_ERROR(ret);

        fmdn_ctx.eid_cleared = true;

        /* Stop FMDN advertise */
        fp_restart_advertise();
        produce_generic_response(response, fp_acc_keys_get_key(acc_key_index),
                FP_ACC_KEYS_ACCOUNT_KEY_LENGTH, DATA_ID_CLEAR_EPHEMERAL_IDENTITY_KEY);
        /* Notify */
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Calculates ring, recovery or unwanted tracking protection key */
static bool calc_key(rru_key_t key_type, uint8_t *key)
{
        uint8_t *identity_key;
        uint8_t hash[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t input[FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH + 1];

        identity_key = fp_acc_keys_get_ephemeral_identity_key();
        if (identity_key == NULL) {
                return false;
        }

        memcpy(input, identity_key, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);
        input[FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH] = key_type;

        /* SHA256 (Ephemeral identity key || order) */
        fp_crypto_sha256(hash, input, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH + 1);
        memcpy(key, hash, ONE_TIME_AUTHENTICATION_KEY_LENGTH);

        return true;
}

/* Produces read ephemeral identity key response for the Seeker */
static void produce_read_ephemeral_identity_key_response(uint8_t *response, uint8_t *recovery_key)
{
        uint8_t *identity_key;

        response[0] = DATA_ID_READ_EPHEMERAL_IDENTITY_KEY;
        response[1] = ONE_TIME_AUTHENTICATION_KEY_LENGTH + FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH;
        identity_key = fp_acc_keys_get_ephemeral_identity_key();
        if (identity_key == NULL) {
                return;
        }

        fp_crypto_aes_ecb(fp_acc_keys_get_owner_key(), FP_ACC_KEYS_ACCOUNT_KEY_LENGTH,
                FP_CRYPTO_OP_ENCRYPT, 2, identity_key, &response[10]);
        /* Add one-time key */
        calculate_one_time_key_for_response(response, recovery_key, RECOVERY_KEY_LENGTH);
}

/* Handles read ephemeral identity key request */
static fast_pair_error_t handle_read_ephemeral_identity_key(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t recovery_key[RECOVERY_KEY_LENGTH];
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH + FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH];

        if (length != (BEACON_MINIMUM_REQ_RES_LENGTH)) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!fp_procedure_is_pairing_mode() && !fmdn_ctx.user_consent) {
                return ATT_ERROR_NO_USER_CONSENT;
        }
        if (!verify_provisioning_and_key_matching(request, RECOVERY_KEY, recovery_key)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        produce_read_ephemeral_identity_key_response(response, recovery_key);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Notifies with ringing state */
static int notify_ringing_state(FP_FMDN_RING_STATE state)
{
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH + 4];
        uint8_t ring_key[RING_KEY_LENGTH];

        response[0] = DATA_ID_RING;
        response[1] = 4 + ONE_TIME_AUTHENTICATION_KEY_LENGTH;
        response[ONE_TIME_AUTHENTICATION_KEY_LENGTH + 2] = state;
        response[ONE_TIME_AUTHENTICATION_KEY_LENGTH + 3] = fmdn_ctx.ring_state.setup.op;
        response[ONE_TIME_AUTHENTICATION_KEY_LENGTH + 4] = fmdn_ctx.ring_state.setup.timeout_ds >> 8;
        response[ONE_TIME_AUTHENTICATION_KEY_LENGTH + 5] = fmdn_ctx.ring_state.setup.timeout_ds & 0xFF;

        /* Add one-time key */
        if (!calc_key(RING_KEY, ring_key)) {
                return 1;
        }
        calculate_one_time_key_for_response(response, ring_key, RING_KEY_LENGTH);
        return notify_beacon_actions_response(fmdn_ctx.svc, fmdn_ctx.ring_conn_idx, response);
}

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/* Start ringing (only GATT write request) */
static FP_FMDN_RING_STATE start_ringing(const ring_setup_t *ring_setup)
{
        FP_FMDN_RING_STATE notify_state;

        if (fp_ring_comp_set_state(ring_setup->op, ring_setup->volume) != 0) {
                FP_FMDN_LOG_PRINTF("Ringing failed to start\r\n");

                notify_state = FP_FMDN_RING_FAILED;
        } else {
                uint32_t timeout_ms;

                FP_FMDN_LOG_PRINTF("Ringing started successfully\r\n");

                fmdn_ctx.ring_state.start_time_ms = (uint32_t) (sys_timer_get_uptime_usec() / 1000);
                memcpy(&fmdn_ctx.ring_state.setup, ring_setup, sizeof(ring_setup_t));

                timeout_ms = (uint32_t) ring_setup->timeout_ds * 100;
                OS_TIMER_CHANGE_PERIOD(fmdn_ctx.ring_tim, OS_MS_2_TICKS(timeout_ms), OS_TIMER_FOREVER);

                notify_state = FP_FMDN_RING_STARTED;
        }

        return notify_state;
}

/* Stop ringing */
static int stop_ringing(FP_FMDN_RING_STATE state)
{
        int err = 0;
        FP_FMDN_RING_STATE notif_state;

        if (fp_ring_comp_set_state(0x0, 0 /* don't care */) != 0) {
                FP_FMDN_LOG_PRINTF("Ringing failed to stop\r\n");

                notif_state = FP_FMDN_RING_FAILED;
        } else {
#if (FP_FMDN_LOG_ENABLE == 1)
                switch (state) {
                case FP_FMDN_RING_TIMEOUT_STOPPED:
                        FP_FMDN_LOG_PRINTF("Ringing stopped successfully after timeout, Time: %lu\r\n",
                                fmdn_ctx.beacon_time_cb());
                        break;
                case FP_FMDN_RING_BUTTON_STOPPED:
                case FP_FMDN_RING_REQUESTED_STOPPED:
                        FP_FMDN_LOG_PRINTF("Ringing stopped successfully\r\n");
                        break;
                default:
                        break;
                }
#endif
                fmdn_ctx.ring_state.setup.op = 0x0;
                fmdn_ctx.ring_state.setup.timeout_ds = 0;

                OS_TIMER_STOP(fmdn_ctx.ring_tim, OS_TIMER_FOREVER);

                notif_state = state;
        }

        if (fmdn_ctx.ring_conn_idx != BLE_CONN_IDX_INVALID) {
                err = notify_ringing_state(notif_state);
        }

        return err;
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

/* Gets current ringing state */
static void get_ringing_state(ring_setup_t *ring_setup_out)
{
        memcpy(ring_setup_out, &fmdn_ctx.ring_state.setup, sizeof(ring_setup_t));

        if (ring_setup_out->op) {
                /* Elapsed time in deciseconds */
                uint16_t elapsed_time_ds =
                        (uint16_t) (((uint32_t) (sys_timer_get_uptime_usec() / 1000) -
                                     fmdn_ctx.ring_state.start_time_ms) / 100);

                if (elapsed_time_ds > ring_setup_out->timeout_ds) {
                        ring_setup_out->op = 0x0;
                        ring_setup_out->timeout_ds = 0;
                } else {
                        ring_setup_out->timeout_ds -= elapsed_time_ds;
                }
        }
}

/*
 * Verifies that the Provider is provisioned and the provided in the request One-Time
 * Authentication Key matches the given ring, recovery or unwanted tracking protection key
 * (all are 8 bytes length)
 */
static bool verify_provisioning_and_key_matching(const uint8_t *request, rru_key_t key_type,
        uint8_t *key)
{
        /* If Provider is not provisioned, return false */
        if (!calc_key(key_type, key)) {
                return false;
        }

        /* If One-Time Authentication Key does not match the ring key, return false */
        return compare_keys(request, key, ONE_TIME_AUTHENTICATION_KEY_LENGTH);
}

/* Handles ringing write request */
static fast_pair_error_t handle_ring(ble_service_t *svc, uint16_t conn_idx, const uint8_t *request,
        uint8_t length)
{
        ring_setup_t ring_setup;
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        FP_FMDN_RING_STATE notify_state;
#endif

        if ((length != (BEACON_MINIMUM_REQ_RES_LENGTH + 1)) &&
            (length != (BEACON_MINIMUM_REQ_RES_LENGTH + 4))) {
                return ATT_ERROR_INVALID_VALUE;
        }

        ring_setup.op = GET_RING_OPERATION(request);
        FP_FMDN_LOG_PRINTF("RING OP: %u\r\n", ring_setup.op);
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (ring_setup.op == RING_ALL) {
                ring_setup.op = RING_COMPONENTS_MASK;
        }
#endif

        /* Skip ringing authentication if corresponding flag is set */
        if (!fmdn_ctx.skip_ringing_authentication) {
                uint8_t ring_key[RING_KEY_LENGTH];
                if (!verify_provisioning_and_key_matching(request, RING_KEY, ring_key)) {
                        return ATT_ERROR_UNAUTHENTICATED;
                }

                /* Verify the requested state matches the number of components that can ring */
                if (ring_setup.op != (ring_setup.op & RING_COMPONENTS_MASK)) {
                        return ATT_ERROR_UNAUTHENTICATED;
                }

                fp_set_authenticated_conn(conn_idx);
        }

        /* Get requested ringing state on available ringing components */
        ring_setup.op = (ring_setup.op & RING_COMPONENTS_MASK);

        /* Get ringing timeout and volume */
        if (ring_setup.op) {
                ring_setup.timeout_ds = ((uint16_t)(GET_RING_TIMEOUT_HIGH(request)) << 8) +
                                        GET_RING_TIMEOUT_LOW(request);
                FP_FMDN_LOG_PRINTF("Ringing TO:%d[ds], Time: %lu\r\n", ring_setup.timeout_ds,
                        fmdn_ctx.beacon_time_cb());

                if ((ring_setup.timeout_ds == 0) ||
                    (ring_setup.timeout_ds > MAX_RINGING_TIMEOUT_DS)) {
                        return ATT_ERROR_INVALID_VALUE;
                }

                ring_setup.volume = GET_RING_VOLUME(request);

                if (ring_setup.volume > FP_RING_COMP_VOLUME_HIGH) {
                        return ATT_ERROR_INVALID_VALUE;
                }
        } else {
                ring_setup.timeout_ds = 0;
        }

        fmdn_ctx.svc = svc;
        fmdn_ctx.ring_conn_idx = conn_idx;

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        if (ring_setup.op) {
                notify_state = start_ringing(&ring_setup);
                notify_ringing_state(notify_state);
        } else {
                stop_ringing(FP_FMDN_RING_REQUESTED_STOPPED);
        }
#else
        notify_ringing_state(FP_FMDN_RING_FAILED);
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

        return ATT_ERROR_OK;
}

/* Produces ringing state response for the Seeker */
static void produce_ringing_state_response(uint8_t *response, uint8_t *ring_key)
{
        ring_setup_t ring_setup;

        memset(response, 0, 5);
        response[0] = DATA_ID_READ_RINGING_STATE;
        response[1] = 3 + ONE_TIME_AUTHENTICATION_KEY_LENGTH;

        get_ringing_state(&ring_setup);

        response[2 + ONE_TIME_AUTHENTICATION_KEY_LENGTH] = ring_setup.op;
        response[3 + ONE_TIME_AUTHENTICATION_KEY_LENGTH] = ring_setup.timeout_ds >> 8;
        response[4 + ONE_TIME_AUTHENTICATION_KEY_LENGTH] = ring_setup.timeout_ds & 0xFF;
        /* Add one-time key */
        calculate_one_time_key_for_response(response, ring_key, RING_KEY_LENGTH);
}

/* Handles ringing state read request */
static fast_pair_error_t handle_read_ringing_state(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        uint8_t ring_key[RING_KEY_LENGTH];
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH + 3];

        if (length != BEACON_MINIMUM_REQ_RES_LENGTH) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!verify_provisioning_and_key_matching(request, RING_KEY, ring_key)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        /*
         * Set connection as current connection requiring ringing state change notifications if
         * there is no active connection
         */
        if (fmdn_ctx.ring_conn_idx == BLE_CONN_IDX_INVALID) {
                fmdn_ctx.ring_conn_idx = conn_idx;
        }

        produce_ringing_state_response(response, ring_key);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Generates a randomized timer interval in seconds */
static uint32_t randomize_interval(uint32_t intv_sec)
{
        return (intv_sec + ((uint32_t) rand()) % ID_ROTATION_RAND_OFFSET_MAX +
                ID_ROTATION_RAND_OFFSET_MIN);
}

/* Helper function to control rotation */
static void control_rotation(OS_TIMER rot_tim, uint32_t intv_sec, bool enable)
{
        if (enable) {
                /* Generate a randomized time interval for next rotation */
                uint32_t rotation_intv_ms = randomize_interval(intv_sec) * 1000;

                /* Start rotation timer (for next rotation to take place) */
                ACCURATE_OS_TIMER_CHANGE_PERIOD(rot_tim, rotation_intv_ms);
        } else {
                ACCURATE_OS_TIMER_STOP(rot_tim);
        }
}

/* Controls address rotation for unwanted tracking protection mode */
static void control_utpm_address_rotation(bool enable)
{
        control_rotation(fmdn_ctx.utpm_tim, UTP_MODE_TIMEOUT_SEC, enable);
}

/* Controls ID rotation */
static void control_id_rotation(bool enable)
{
        control_rotation(fmdn_ctx.id_rotation_tim, ID_ROTATION_TIME_SEC, enable);
}

/* Activates unwanted tracking protection mode */
static void activate_utpm(uint8_t control_flags)
{
        FP_FMDN_LOG_PRINTF("Unwanted tracking prevention mode: ON\r\n");

        fmdn_ctx.utpm = true;
        fmdn_ctx.regenerate_adv_struct = true;
        if (control_flags & SKIP_RINGING_AUTH_FLAG) {
                fmdn_ctx.skip_ringing_authentication = true;
        }

        /* Flags changed so EID and hashed flags need to be re-computed */
        compute_e2ee_eid(fmdn_ctx.beacon_time_cb(), fp_acc_keys_get_ephemeral_identity_key());
        /* Start address rotation timer (for next rotation to take place) */
        control_utpm_address_rotation(true);
        /* Motion detector will enabled after some separation time */
        fp_motion_detection_start();
}

/* Handles activate unwanted tracking protection mode request */
static fast_pair_error_t handle_activate_utpm(ble_service_t *svc,
        uint16_t conn_idx, const uint8_t *request, uint8_t length)
{
        uint8_t utpm_key[UTPM_KEY_LENGTH];
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH];
        uint8_t control_flags = 0;
        int ret;

        if ((length != BEACON_MINIMUM_REQ_RES_LENGTH) &&
            (length != (BEACON_MINIMUM_REQ_RES_LENGTH + 1))) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!verify_provisioning_and_key_matching(request, UTPM_KEY, utpm_key)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        if (length == (BEACON_MINIMUM_REQ_RES_LENGTH + 1)) {
                control_flags = GET_UTPM_CONTROL_FLAGS(request);
        }

        activate_utpm(control_flags);
        ret = fp_update_advertise_data(false);
        FP_CHECK_ERROR(ret);

        produce_generic_response(response, utpm_key, UTPM_KEY_LENGTH,
                DATA_ID_ACTIVATE_UNWANTED_TRACKING_PROTECTION);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

/* Deactivates unwanted tracking protection mode */
static void deactivate_utpm(void)
{
        FP_FMDN_LOG_PRINTF("Unwanted tracking prevention mode: OFF\r\n");

        fmdn_ctx.utpm = false;
        fmdn_ctx.regenerate_adv_struct = true;
        fmdn_ctx.skip_ringing_authentication = false;
        /* Flags changed so EID and hashed flags need to be re-computed */
        compute_e2ee_eid(fmdn_ctx.beacon_time_cb(), fp_acc_keys_get_ephemeral_identity_key());

        /* Stop address rotation timer */
        control_utpm_address_rotation(false);
        /* No motion detection in near-onwer state */
        fp_motion_detection_stop();
}

/* Handles deactivate unwanted tracking protection mode request */
static fast_pair_error_t handle_deactivate_utpm(ble_service_t *svc,
        uint16_t conn_idx, const uint8_t *request, uint8_t length)
{
        uint8_t utpm_key[UTPM_KEY_LENGTH];
        uint8_t response[BEACON_MINIMUM_REQ_RES_LENGTH];
        int ret;

        if (length != (BEACON_MINIMUM_REQ_RES_LENGTH + 8)) {
                return ATT_ERROR_INVALID_VALUE;
        }
        if (!verify_provisioning_and_key_matching(request, UTPM_KEY, utpm_key)) {
                return ATT_ERROR_UNAUTHENTICATED;
        }
        if (!verify_eik_match(GET_ADD_DATA(request))) {
                return ATT_ERROR_UNAUTHENTICATED;
        }

        fp_set_authenticated_conn(conn_idx);

        deactivate_utpm();
        ret = fp_update_advertise_data(false);
        FP_CHECK_ERROR(ret);

        produce_generic_response(response, utpm_key, UTPM_KEY_LENGTH,
                DATA_ID_DEACTIVATE_UNWANTED_TRACKING_PROTECTION);
        notify_beacon_actions_response(svc, conn_idx, response);

        return ATT_ERROR_OK;
}

#if (FP_BATTERIES_COUNT != 0)
/* Gets level of the main battery */
static battery_level_t get_battery_level(void)
{
        battery_level_t battery_level = BATTERY_LEVEL_UNSUPPORTED;
        fp_battery_info_t *info = fp_get_battery_information();

        if (info[0].level <= FP_FMDN_BATTERY_LEVEL_CRITICAL) {
                battery_level = BATTERY_LEVEL_CRITICAL;
        } else if (info[0].level <= FP_FMDN_BATTERY_LEVEL_LOW) {
                battery_level = BATTERY_LEVEL_LOW;
        } else {
                battery_level = BATTERY_LEVEL_NORMAL;
        }

        return battery_level;
}
#endif /* FP_BATTERIES_COUNT */

/* Returns hashed flags based on the initial computation of SHA256(r) */
static uint8_t get_hashed_flags(void)
{
        uint8_t flags = 0;

        /* Set unwanted tracking protection mode indication */
        if (fmdn_ctx.utpm) {
                flags |= HASH_FLAGS_UTPM_MASK;
        }
#if (FP_BATTERIES_COUNT != 0)
        /* Set battery level indication */
        flags |= ((get_battery_level() << 1) & HASH_FLAGS_BATT_LEVEL_MASK);
#endif
        flags ^= fmdn_ctx.sha256_r_lsb;

        /* Update hashed flags */
        fmdn_ctx.hashed_flags = flags;

        return flags;
}

/* Computes hashed flags for FMDN frame */
static uint8_t compute_hashed_flags(uECC_word_t *r)
{
        uint8_t hash[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t byte_buffer[32];  /* maximum size for SECP256R1 */

        /* Compute LSB of SHA256(r) */
        uECC_vli_nativeToBytes(byte_buffer, uECC_curve_num_bytes(ELLIPTIC_CURVE), r);
        fp_crypto_sha256(hash, byte_buffer, uECC_curve_num_bytes(ELLIPTIC_CURVE));
        fmdn_ctx.sha256_r_lsb = hash[FP_CRYPTO_SHA256_BYTES_LEN - 1];

        return get_hashed_flags();
}

/* Computes ephemeral ID (EID) */
static void compute_e2ee_eid(uint32_t beacon_time_counter, const uint8_t *identity)
{
        uint8_t data[32] = {
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xee, 0x11, 0x22, 0x33, 0x44,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0xee, 0x11, 0x22, 0x33, 0x44
        };
        uint8_t data_enc[32];
        uECC_word_t rp[CURVE_MAX_WORDS * 2] = { 0 };
        uECC_word_t r[CURVE_MAX_WORDS] = { 0 };
        uECC_word_t R[CURVE_MAX_WORDS * 2];

        OS_ASSERT(E2EE_EID_LENGTH == uECC_curve_num_bytes(ELLIPTIC_CURVE));

        FP_FMDN_LOG_PRINTF("New EID is computed\r\n");

        FP_FMDN_LOG_PRINTF("Time: %lu\r\n", beacon_time_counter);
        FMDN_PRINT_HEX("AK ", fp_acc_keys_get_owner_key(), 16);
        FMDN_PRINT_HEX("EIK", identity, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);

        for (int i = 0; i < ROTATION_PERIOD_EXPONENT; ++i) {
                beacon_time_counter &= ~(1 << i);
        }
        reverse_byte_order((uint8_t *) &beacon_time_counter, 4);

        data[11] = ROTATION_PERIOD_EXPONENT;
        memcpy(data + 12, (uint8_t *) &beacon_time_counter, 4);
        data[27] = ROTATION_PERIOD_EXPONENT;
        memcpy(data + 28, (uint8_t *) &beacon_time_counter, 4);
        FMDN_PRINT_HEX("data    ", data, sizeof(data));
        fp_crypto_aes_ecb(identity, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH, FP_CRYPTO_OP_ENCRYPT, 2,
                data, data_enc);
        FMDN_PRINT_HEX("data_enc", data_enc, sizeof(data_enc));

        uECC_vli_bytesToNative(rp, data_enc, sizeof(data_enc));
        FMDN_PRINT_HEX("r'", (uint8_t *) rp, sizeof(rp));

        /* r = r' mod n */
        uECC_vli_mmod(r, rp, uECC_curve_n(ELLIPTIC_CURVE), 6);
        FMDN_PRINT_HEX("r  ", (uint8_t *) r, sizeof(r));

        /* R = r * G */
        uECC_point_mult(R, uECC_curve_G(ELLIPTIC_CURVE), r, ELLIPTIC_CURVE);
        FMDN_PRINT_HEX("R  ", (uint8_t *) R, sizeof(R));

        /* Rx is the value to be advertised as an ephemeral identifier */
        uECC_vli_nativeToBytes(fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH, R);
        FMDN_PRINT_HEX("EID", fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH);

        /* Finally compute hashed flags */
        fmdn_ctx.hashed_flags = compute_hashed_flags(r);
        fmdn_ctx.regenerate_adv_struct = true;
}

/* Unwanted tracking protection timeout timer callback */
static void utpm_tim_cb(ACCURATE_OS_TIMER timer)
{
        fp_send_notification(UTPM_TMO_NOTIF);
}

/* Provisioning timeout timer callback */
static void provisioning_tim_cb(ACCURATE_OS_TIMER timer)
{
        fp_send_notification(FMDN_PROVISIONING_TMO_NOTIF);
}

/* User button timeout timer callback */
static void user_consent_tim_cb(OS_TIMER timer)
{
        fp_send_notification(USER_CONSENT_TMO_NOTIF);
}

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
/* Ringing timeout timer callback */
static void ring_tim_cb(OS_TIMER timer)
{
        fp_send_notification(RING_TMO_NOTIF);
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

/* ID Rotation timer callback function */
static void id_rotation_tim_cb(ACCURATE_OS_TIMER timer)
{
        fp_send_notification(ID_ROTATION_TMO_NOTIF);
}

int fp_fmdn_init(const fp_fmdn_cfg_t *cfg)
{
        memset(&fmdn_ctx, 0, sizeof(fmdn_ctx));

        fmdn_ctx.beacon_time_cb = cfg->beacon_time_cb;
        /* EID needs to be calculated for ADV */
        rotate_ephemeral_id();

        fmdn_ctx.provisioning_tim = ACCURATE_OS_TIMER_CREATE(PROVISIONING_TIMEOUT_MS, false,
                                        provisioning_tim_cb);
        OS_ASSERT(fmdn_ctx.provisioning_tim);
        fmdn_ctx.user_consent_tim = OS_TIMER_CREATE("consent_tim",
                                        OS_MS_2_TICKS(FP_FMDN_USER_CONSENT_TIMEOUT_MS),
                                        OS_TIMER_ONCE, NULL, user_consent_tim_cb);
        OS_ASSERT(fmdn_ctx.user_consent_tim);
        fmdn_ctx.utpm_tim = ACCURATE_OS_TIMER_CREATE(UTP_MODE_TIMEOUT_SEC * 1000, false,
                                utpm_tim_cb);
        OS_ASSERT(fmdn_ctx.utpm_tim);
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        fmdn_ctx.ring_tim = OS_TIMER_CREATE("ring_tim", 1, OS_TIMER_ONCE, NULL, ring_tim_cb);
        OS_ASSERT(fmdn_ctx.ring_tim);
#endif
        fmdn_ctx.id_rotation_tim = ACCURATE_OS_TIMER_CREATE(ID_ROTATION_TIME_SEC * 1000, false,
                                        id_rotation_tim_cb);
        OS_ASSERT(fmdn_ctx.id_rotation_tim);

        /* Set provisioning state in Google Fast Pair framework core module */
        fp_set_fmdn_provisioning_state((fp_acc_keys_get_ephemeral_identity_key() != NULL) ?
                FP_FMDN_PROVISIONING_INITIATING : FP_FMDN_PROVISIONING_STOPPED);

        /* Enable periodic ID rotation */
        control_id_rotation(true);

        fmdn_ctx.ring_conn_idx = BLE_CONN_IDX_INVALID;
        fmdn_ctx.regenerate_adv_struct = true;

        return 0;
}

void fp_fmdn_deinit(void)
{
        ACCURATE_OS_TIMER_DELETE(fmdn_ctx.id_rotation_tim);
#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
        OS_TIMER_DELETE(fmdn_ctx.ring_tim, OS_TIMER_FOREVER);
#endif
        ACCURATE_OS_TIMER_DELETE(fmdn_ctx.utpm_tim);
        OS_TIMER_DELETE(fmdn_ctx.user_consent_tim, OS_TIMER_FOREVER);
        ACCURATE_OS_TIMER_DELETE(fmdn_ctx.provisioning_tim);
}

bool fp_fmdn_is_utpm_active(void)
{
        return fmdn_ctx.utpm;
}

fast_pair_error_t fp_fmdn_beacon_actions_write_cb(ble_service_t *svc, uint16_t conn_idx,
        const uint8_t *request, uint8_t length)
{
        data_id_t data_id = request[0];
        fast_pair_error_t status = ATT_ERROR_INVALID_VALUE;

        switch (data_id) {
        case DATA_ID_READ_BEACON_PARAMETERS:
                FP_FMDN_LOG_PRINTF("DATA_ID_READ_BEACON_PARAMETERS\r\n");
                status = handle_read_beacon_state(svc, conn_idx, request, length);
                update_last_nonce();
                break;
        case DATA_ID_READ_PROVISIONING_STATE:
                FP_FMDN_LOG_PRINTF("DATA_ID_READ_PROVISIONING_STATE\r\n");
                status = handle_read_provisioning_state(svc, conn_idx, request, length);
                update_last_nonce();
                break;
        case DATA_ID_SET_EPHEMERAL_IDENTITY_KEY:
                FP_FMDN_LOG_PRINTF("DATA_ID_SET_EPHEMERAL_IDENTITY_KEY\r\n");
                status = handle_set_ephemeral_identity_key(svc, conn_idx, request, length);
                break;
        case DATA_ID_CLEAR_EPHEMERAL_IDENTITY_KEY:
                FP_FMDN_LOG_PRINTF("DATA_ID_CLEAR_EPHEMERAL_IDENTITY_KEY\r\n");
                status = handle_clear_ephemeral_identity_key(svc, conn_idx, request, length);
                break;
        case DATA_ID_READ_EPHEMERAL_IDENTITY_KEY:
                FP_FMDN_LOG_PRINTF("DATA_ID_READ_EPHEMERAL_IDENTITY_KEY\r\n");
                status = handle_read_ephemeral_identity_key(svc, conn_idx, request, length);
                update_last_nonce();
                break;
        case DATA_ID_RING:
                FP_FMDN_LOG_PRINTF("DATA_ID_RING\r\n");
                status = handle_ring(svc, conn_idx, request, length);
                break;
        case DATA_ID_READ_RINGING_STATE:
                FP_FMDN_LOG_PRINTF("DATA_ID_READ_RINGING_STATE\r\n");
                status = handle_read_ringing_state(svc, conn_idx, request, length);
                update_last_nonce();
                break;
        case DATA_ID_ACTIVATE_UNWANTED_TRACKING_PROTECTION:
                FP_FMDN_LOG_PRINTF("DATA_ID_ACTIVATE_UNWANTED_TRACKING_PROTECTION\r\n");
                status = handle_activate_utpm(svc, conn_idx, request, length);
                break;
        case DATA_ID_DEACTIVATE_UNWANTED_TRACKING_PROTECTION:
                FP_FMDN_LOG_PRINTF("DATA_ID_DEACTIVATE_UNWANTED_TRACKING_PROTECTION\r\n");
                status = handle_deactivate_utpm(svc, conn_idx, request, length);
                break;
        default:
                break;
        }

        return status;
}

void fp_fmdn_beacon_actions_read_cb(ble_service_t *svc, uint16_t conn_idx, uint8_t *response)
{
        update_last_nonce();

        response[0] = PROTOCOL_VERSION;
        memcpy(&(response[1]), fmdn_ctx.last_nonce, NONCE_BYTES_NUM);
}

void fp_fmdn_schedule_new_advertise_struct(void)
{
        fmdn_ctx.regenerate_adv_struct = true;
}

#if (LOCATION_ENABLED_ADV_FRAME == 1)
static void prepare_google_adv_payload_data(uint8_t *payload)
{
        payload[0] = (fmdn_ctx.utpm ? E2EE_EID_FRAME_UTPM_TYPE : E2EE_EID_FRAME_TYPE);
        /* Ephemeral identifier */
        memcpy(&(payload[1]), fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH);
        payload[1 + E2EE_EID_LENGTH] = get_hashed_flags();
}

const gap_adv_ad_struct_t *fp_fmdn_advertise_struct(void)
{
        __FP_RETAINED static uint8_t payload[4 + (E2EE_EID_LENGTH + 2)];

        if (fmdn_ctx.regenerate_adv_struct) {
                /* Prepare location-enabled advertise payload */
                /* UUID field, 0xFCB2 is for location-enabled bluetooth advertise  */
                payload[0] = 0xB2;
                payload[1] = 0xFC;
                payload[2] = FP_FMDN_NETWORK_ID;
                payload[3] = (fmdn_ctx.utpm ? 0x00 : 0x01);
                /* Add proprietary company payload data */
                prepare_google_adv_payload_data(&payload[4]);

                fmdn_ctx.adv_struct.type = GAP_DATA_TYPE_UUID16_SVC_DATA;
                fmdn_ctx.adv_struct.len = sizeof(payload);
                fmdn_ctx.adv_struct.data = payload;
                fmdn_ctx.regenerate_adv_struct = false;
        }

        return &(fmdn_ctx.adv_struct);
}
#else
const gap_adv_ad_struct_t *fp_fmdn_advertise_struct(void)
{
        __FP_RETAINED static uint8_t payload[E2EE_EID_LENGTH + 3 + 1];

        if (fmdn_ctx.regenerate_adv_struct) {
                /* Prepare advertise struct with E2EE-EID Frame */
                /* Set FMDN Service UUID field */
                payload[0] = 0xAA;
                payload[1] = 0xFE;
                /* Frame type */
                payload[2] = (fmdn_ctx.utpm ? E2EE_EID_FRAME_UTPM_TYPE : E2EE_EID_FRAME_TYPE);
                /* Ephemeral identifier */
                memcpy(&(payload[3]), fmdn_ctx.E2EE_EID, E2EE_EID_LENGTH);
                payload[3 + E2EE_EID_LENGTH] = get_hashed_flags();

                fmdn_ctx.adv_struct.type = GAP_DATA_TYPE_UUID16_SVC_DATA;
                fmdn_ctx.adv_struct.len = sizeof(payload);
                fmdn_ctx.adv_struct.data = payload;
                fmdn_ctx.regenerate_adv_struct = false;
        }

        return &(fmdn_ctx.adv_struct);
}
#endif /* LOCATION_ENABLED_ADV_FRAME */

int fp_fmdn_rotate_utpm_address(void)
{
        int err;

        fmdn_ctx.utpm_addr_rotation_pending = false;

        if (fmdn_ctx.utpm) {
                /* No need to change BD address while advertising is stopped */
                fmdn_ctx.utpm_addr_rotation_pending = fp_is_advertise_stopped();

                /* The BD address should not change if pairing mode is enabled */
                fmdn_ctx.utpm_addr_rotation_pending = fmdn_ctx.utpm_addr_rotation_pending ||
                        fp_procedure_is_pairing_mode();

                if (fmdn_ctx.utpm_addr_rotation_pending) {
                        return 0;
                }

                /* Trigger random address renewal */
                err += fp_set_rand_addr_renewal(true);
                /* Salt rotation */
                fp_acc_keys_generate_new_salt();

                /* Start address rotation timer (for next rotation to take place) */
                control_utpm_address_rotation(true);
        }

        return err;
}

int fp_fmdn_rotate_id(void)
{
        int err = 0;
        bool id_rot_pending;

        /* No need to change BD address while advertising is stopped */
        id_rot_pending = fp_is_advertise_stopped();

#if (FP_LOCATOR_TAG == 1)
        /*
         * After the Provider was paired, the BD address should not change till FMDN is
         * provisioned or till provisioning timer expires
         */
        id_rot_pending = id_rot_pending || ((fp_acc_keys_get_ephemeral_identity_key() == NULL) &&
                                            ACCURATE_OS_TIMER_IS_ACTIVE(fmdn_ctx.provisioning_tim));
#endif /* FP_LOCATOR_TAG */

        /* The BD address should not change if pairing mode is enabled */
        id_rot_pending = id_rot_pending || fp_procedure_is_pairing_mode();

        /* Update ID rotation status */
        fmdn_ctx.id_rotation_stat = (id_rot_pending) ? ID_ROTATION_PENDING : ID_ROTATION_NONE;

        /* Postpone ID rotation if marked as pending */
        if (fmdn_ctx.id_rotation_stat == ID_ROTATION_PENDING) {
                return 0;
        }

        /* The BD address should not change if unwanted tracking protection mode is enabled */
        if (!fmdn_ctx.utpm) {
                /*
                 * Stop advertise before address rotation. It will be restarted with renewed
                 * BD address.
                 */
                ble_error_t ret = BLE_ERROR_NOT_ALLOWED;

                if (fp_ble_adv_stop() == BLE_ERROR_NOT_ALLOWED) {
                        ret = fp_ble_adv_stop_all();
                }

                /* Set ID rotation in progress */
                fmdn_ctx.id_rotation_stat = ID_ROTATION_IN_PROGRESS;

                /* Trigger random address renewal */
                err += fp_set_rand_addr_renewal(true);
                /* Salt rotation */
                fp_acc_keys_generate_new_salt();

                if (ret == BLE_STATUS_OK) {
                        return BLE_STATUS_OK;
                }

                err += (ret != BLE_ERROR_NOT_ALLOWED);
        }

        /* Rotate ephemeral ID */
        rotate_ephemeral_id();

        /* Start rotation timer (for next rotation to take place) */
        control_id_rotation(true);

        /* Clear indication for ID rotation in progress */
        fmdn_ctx.id_rotation_stat = ID_ROTATION_NONE;

        /* Restart advertising if unwanted tracking protection mode is enabled */
        if (fmdn_ctx.utpm) {
                err += fp_restart_advertise();
        }

        return err;
}

void fp_fmdn_perform_id_rotation_in_progress(void)
{
        /* Perform ID rotation if it is in progress */
        if (fmdn_ctx.id_rotation_stat == ID_ROTATION_IN_PROGRESS) {
                /* Rotate ephemeral ID */
                rotate_ephemeral_id();

                /* Start rotation timer (for next rotation to take place) */
                control_id_rotation(true);

                /* Clear indication for ID rotation in progress */
                fmdn_ctx.id_rotation_stat = ID_ROTATION_NONE;
        }
}

bool fp_fmdn_is_id_rotation_pending(void)
{
        return (fmdn_ctx.id_rotation_stat == ID_ROTATION_PENDING);
}

bool fp_fmdn_is_utpm_address_rotation_pending(void)
{
        return fmdn_ctx.utpm_addr_rotation_pending;
}

void fp_fmdn_set_user_consent(bool enable)
{
        fmdn_ctx.user_consent = enable;

        FP_FMDN_LOG_PRINTF("User consent mode: %s\r\n", enable ? "ON" : "OFF");

        if (enable) {
                OS_TIMER_START(fmdn_ctx.user_consent_tim, OS_TIMER_FOREVER);
        } else {
                OS_TIMER_STOP(fmdn_ctx.user_consent_tim, OS_TIMER_FOREVER);
        }
}

bool fp_fmdn_is_user_consent_mode(void)
{
        return fmdn_ctx.user_consent;
}

void fp_fmdn_wipe_out_account_keys(void)
{
        int ret;

        if (fmdn_ctx.eid_cleared) {
                fmdn_ctx.eid_cleared = false;
                /* Account keys can only be cleared when GAP disconnects */
                ret = fp_acc_keys_clean();
                FP_CHECK_ERROR(ret);
        }
}

#if (FP_LOCATOR_TAG == 1)
void fp_fmdn_set_provisioning_timer(bool enable)
{
        if (enable) {
                ACCURATE_OS_TIMER_CHANGE_PERIOD(fmdn_ctx.provisioning_tim,
                        (fp_get_fmdn_provisioning_state() == FP_FMDN_PROVISIONING_INITIATING) ?
                                FP_FMDN_PROVISIONING_INIT_TIMEOUT_MS : PROVISIONING_TIMEOUT_MS);
        } else {
                ACCURATE_OS_TIMER_STOP(fmdn_ctx.provisioning_tim);
        }
}
#endif /* FP_LOCATOR_TAG */

#if (FP_FMDN_RING_COMPONENTS_NUM > 0)
int fp_fmdn_stop_ringing(FP_FMDN_RING_STATE state)
{
        int err = 0;

        err = stop_ringing(state);
        if (fmdn_ctx.ringing_complete_cb) {
                fmdn_ctx.ringing_complete_cb();
        }

        return err;
}

int fp_fmdn_start_ringing(uint32_t dur_ms, FP_RING_COMP_VOLUME volume,
        fp_fmdn_ringing_complete_cb_t cb)
{
        ring_setup_t ring_setup;

        if (fp_fmdn_is_ringing()) {
                /* Another ringing request is currently processed, ignore this one */
                return 2;
        }

        fmdn_ctx.ringing_complete_cb = cb;

        ring_setup.op = RING_COMPONENTS_MASK;
        ring_setup.volume = volume;
        ring_setup.timeout_ds = dur_ms / 100;
        return (start_ringing(&ring_setup) != FP_FMDN_RING_STARTED);
}

bool fp_fmdn_is_ringing(void)
{
        return (fmdn_ctx.ring_state.setup.op > 0x0);
}
#endif /* FP_FMDN_RING_COMPONENTS_NUM */

void fp_fmdn_remove_ring_connection(uint16_t conn_idx)
{
        if (fmdn_ctx.ring_conn_idx == conn_idx) {
                fmdn_ctx.ring_conn_idx = BLE_CONN_IDX_INVALID;
        }
}

uint8_t *fp_fmdn_get_eid(void)
{
        return fmdn_ctx.E2EE_EID;
}

bool fp_fmdn_get_recovery_key(uint8_t *recovery_key)
{
        /* If Provider is not provisioned, return false */
        return calc_key(RECOVERY_KEY, recovery_key);
}
#endif /* FP_FMDN */
