/**
 ****************************************************************************************
 *
 * @file fp_procedure.c
 *
 * @brief Google Fast Pair procedure module implementation
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
#include <string.h>
#include FAST_PAIR_CONFIG_FILE
#include "fp_defaults.h"
#include "sdk_defs.h"
#include "osal.h"
#include "ble_stack_config.h"
#include "ad_ble.h"
#include "gfps.h"
#include "fp_crypto.h"
#include "fp_core.h"
#include "fast_pair.h"
#include "fp_notifications.h"
#include "fp_account_keys.h"
#include "fp_conn_params.h"
#include "fp_utils.h"
#include "uECC.h"

#include "fp_procedure.h"

#define PAIRING_NOT_ALLOWED_TIME                (5 * 60 * 1000)  /* default is 5 minutes */
#define KEY_TMO                                 (10000)          /* 10 seconds */

#define AES_KEY_LENGTH                          (32)
#define KEY_LENGTH                              (AES_KEY_LENGTH / 2)

#define MSG_LENGTH                              (16)
#define PASSKEY_LENGTH                          (6)
#define ADD_DATA_MAX_LENGTH                     (64)
#define ADD_DATA_PACKET_LENGTH                  (FP_CRYPTO_AES_BLOCK_LENGTH + ADD_DATA_MAX_LENGTH)

#define SEEKER_PASSKEY_TYPE                     (0x2)
#define PROVIDER_PASSKEY_TYPE                   (0x3)

#define KEY_BASED_PAIRING_RESPONE_MSG_TYPE      (0x01)

#define ADD_DATA_HMAC_SHA_SIZE                  (8)

/* Key-based pairing request flags */
#define REQ_FLAG_PROVIDER_STARTS_PAIRING        (0x40)
#define REQ_FLAG_RETROACTIVE_ACCOUNT_KEY_WRITE  (0x10)
#define REQ_FLAG_PROVIDER_NOTIFY_NAME           (0x20)

#define USED_SALT_SET_SIZE                      (10)
#define MAX_SALT_LENGTH                         (15)

/* Action request flags */
#define REQ_FLAG_ADDITIONAL_DATA                (0x40)

/* Additional data ID */
#define DATA_ID_PERSONALIZED_NAME               (0x01)

#define MSG_TYPE_ACTION_REQ                     (0x10)
#define MSG_TYPE_KEY_BASED_PAIRING_REQ          (0x00)

#if (FP_TEST_KEY_PAIRING == 1)
/* Google's test vectors for pairing request */
static const uint8_t test_output[] = { 0xF3, 0x0F, 0x4E, 0x78, 0x6C, 0x59, 0xA7, 0xBB,
                                       0xF3, 0x87, 0x3B, 0x5A, 0x49, 0xBA, 0x97, 0xEA };
static const uint8_t test_key[] = { 0xA0, 0xBA, 0xF0, 0xBB, 0x95, 0x1F, 0xF7, 0xB6,
                                    0xCF, 0x5E, 0x3F, 0x45, 0x61, 0xC3, 0x32, 0x1D };
static const uint8_t test_request[] = { 0xAC, 0x9A, 0x16, 0xF0, 0x95, 0x3A, 0x3F, 0x22,
                                        0x3D, 0xD1, 0x0C, 0xF5, 0x36, 0xE0, 0x9E, 0x9C} ;
#endif /* FP_TEST_KEY_PAIRING */

/* Fast Pair pairing procedure state */
typedef enum  {
        PROCEDURE_STATE_NONE,
        PROCEDURE_STATE_IDLE,
        PROCEDURE_STATE_ACTION_ADDITIONAL_DATA,
        PROCEDURE_STATE_KEY_BASED_PAIRING
} PROCEDURE_STATE;

/* Fast Pair pairing procedure error */
typedef enum  {
        PROCEDURE_ERR_NONE,
        PROCEDURE_ERR_STOPPED,
        PROCEDURE_ERR_PASSKEY_DECRYPT_FAILED,
        PROCEDURE_ERR_UNSUPPORTED_MSG_TYPE,
        PROCEDURE_ERR_PERSONALIZED_NAME_INVALID,
        PROCEDURE_ERR_ACC_KEY_DECRYPT_FAILED,
        PROCEDURE_ERR_CONN_FAILED
} PROCEDURE_ERR;

/* Fast Pair pairing procedure module context */
__FP_RETAINED static struct {
        bool is_pairing_mode;
        uint8_t pairing_failures;
        uint8_t bd_address[BD_ADDR_LEN]; /* big-endian */
        OS_TIMER failures_tim;
        OS_TIMER key_tim;
        uint8_t key[KEY_LEN];
#if (FP_LOCATOR_TAG != 1)
        bool is_passkey_received;
#endif
        PROCEDURE_STATE proc_state;
        uint8_t additional_data_id;
#if (FP_LOCATOR_TAG != 1)
        uint32_t passkey;
#endif
        uint16_t conn_idx;
#ifdef FP_PERSONALIZED_NAME
        char personalized_name[FP_PERSONALIZED_NAME_MAX_LENGTH];
#endif
        fp_procedure_status_cb_t procedure_status_cb;
} proc_ctx;

/* Structure to track used salts */
__FP_RETAINED static struct {
        uint8_t salts[USED_SALT_SET_SIZE][MAX_SALT_LENGTH];
        uint8_t lengths[USED_SALT_SET_SIZE];
        uint8_t last_idx;
} used_salt_set;

/* Anti-Spoofing key for Model ID  */
static const uint8_t fp_anti_spoofing_private_key[] = FP_ANTI_SPOOFING_PRIVATE_KEY;

/* Verifies if the received request is correct */
static bool verify_request(uint8_t *request)
{
        own_address_t rpa_address;
        uint8_t public_address[BD_ADDR_LEN];

        ble_gap_address_get(&rpa_address);
        reverse_byte_order(rpa_address.addr, BD_ADDR_LEN);
        ad_ble_get_public_address(public_address);
        reverse_byte_order(public_address, BD_ADDR_LEN);

        /* Octet 2-7 Provider's current BD address
         * Check against RPA or public address */
        if (memcmp(rpa_address.addr, &(request[2]), BD_ADDR_LEN) == 0) {
                /* RPA will be used in response (a fix for some phones) */
                memcpy(proc_ctx.bd_address, rpa_address.addr, BD_ADDR_LEN);
                return true;
        } else if (memcmp(public_address, &(request[2]), BD_ADDR_LEN) == 0) {
                /* Public will be used in response (Fast Pair requirement) */
                memcpy(proc_ctx.bd_address, public_address, BD_ADDR_LEN);
                return true;
        }

        return false;
}

/* Pairing failures timeout timer callback function */
static void failures_tim_cb(OS_TIMER timer)
{
        proc_ctx.pairing_failures = 0;
}

/* Key timeout timer callback function */
static void key_tim_cb(OS_TIMER timer)
{
        fp_send_notification(KEY_TMO_NOTIF);
}

/* Produces key based paring response */
static void produce_response(uint8_t *response)
{
        uint8_t i;

        /* Msg type */
        response[0] = KEY_BASED_PAIRING_RESPONE_MSG_TYPE;
        /* It should be Provider's public address
         * but as problems some phones an address from request is used */
        memcpy(&(response[1]), proc_ctx.bd_address, BD_ADDR_LEN);
        /* Salt remainder */
        response[7] = fp_crypto_get_salt();
        for (i = 8; i < 16; i += 2) {
                uint16_t rand_val = fp_crypto_get_salt();
                response[i] = rand_val;
                response[i + 1] = rand_val >> 8;
        }
}

#if (FP_TEST_KEY_PAIRING == 1)
/* Google's test vector for AES counter mode */
static void test_aes_ctr_crypt(void)
{
        uint8_t input[] = {
                0x53, 0x6F, 0x6D, 0x65, 0x6F, 0x6E, 0x65, 0x27, 0x73, 0x20, 0x47, 0x6F, 0x6F,
                0x67, 0x6C, 0x65, 0x20, 0x48, 0x65, 0x61, 0x64, 0x70, 0x68, 0x6F, 0x6E, 0x65
        };
        uint8_t nonce[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
        uint8_t secret[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                             0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
        uint8_t result[] = {
                0xEE, 0x4A, 0x24, 0x83, 0x73, 0x80, 0x52, 0xE4, 0x4E, 0x9B, 0x2A, 0x14, 0x5E,
                0x5D, 0xDF, 0xAA, 0x44, 0xB9, 0xE5, 0x53, 0x6A, 0xF4, 0x38, 0xE1, 0xE5, 0xC6};
        uint8_t output[sizeof(result)];
        uint8_t i = 0;

        fp_crypto_aes_ctr(input, sizeof(input), nonce, secret, FP_CRYPTO_OP_ENCRYPT, output);
        for (i = 0; i < sizeof(result); i++) {
                if (result[i] != output[i]) {
                        FP_LOG_PRINTF("aes_ctr_crypt() error at %d\r\n", i);
                        break;
                }
        }
}

/* Google's test vector for HMAC-SHA256 */
static void test_hmac_sha256(void)
{
        uint8_t input[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xEE, 0x4A, 0x24, 0x83,
                            0x73, 0x80, 0x52, 0xE4, 0x4E, 0x9B, 0x2A, 0x14, 0x5E, 0x5D, 0xDF, 0xAA,
                            0x44, 0xB9, 0xE5, 0x53, 0x6A, 0xF4, 0x38, 0xE1, 0xE5, 0xC6 };
        uint8_t secret[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67,
                             0x89, 0xAB, 0xCD, 0xEF };
        uint8_t result[] = { 0x55, 0xEC, 0x5E, 0x60, 0x55, 0xAF, 0x6E, 0x92, 0x61, 0x8B, 0x7D, 0x87,
                             0x10, 0xD4, 0x41, 0x37, 0x09, 0xAB, 0x5D, 0xA2, 0x7C, 0xA2, 0x6A, 0x66,
                             0xF5, 0x2E, 0x5A, 0xD4, 0xE8, 0x20, 0x90, 0x52 };
        uint8_t output[sizeof(result)];
        uint8_t i = 0;

        fp_crypto_hmac_sha256(input, sizeof(input), secret, FP_ACC_KEYS_ACCOUNT_KEY_LENGTH, output);
        for (i = 0; i < sizeof(result); i++) {
                if (result[i] != output[i]) {
                        FP_LOG_PRINTF("hmac_sha256() error at %d\r\n", i);
                        break;
                }
        }
}
#endif /* FP_TEST_KEY_PAIRING */

#ifdef FP_PERSONALIZED_NAME
/* Notifies Seeker with additional data containing personalized name */
static ble_error_t notify_additional_data_name(ble_service_t *svc, uint16_t conn_idx)
{
        uint8_t nonce[FP_CRYPTO_AES_BLOCK_LENGTH] = { 0 };
        uint8_t data_packet[ADD_DATA_PACKET_LENGTH];
        uint8_t hash[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t i;
        uint8_t personalized_name_len = strlen(proc_ctx.personalized_name);

        for (i = 8; i < 16; i += 2) {
                uint16_t rand_val = fp_crypto_get_salt();
                nonce[i] = rand_val;
                nonce[i + 1] = rand_val >> 8;
        }

        /* Encrypt personalized name directly in data packet starting of data_packet[16] */
        fp_crypto_aes_ctr((uint8_t *) proc_ctx.personalized_name, personalized_name_len, nonce,
                proc_ctx.key, FP_CRYPTO_OP_ENCRYPT, &data_packet[FP_CRYPTO_AES_BLOCK_LENGTH]);
        /* Copy nonce random bytes to data packet */
        memcpy(&data_packet[8], &nonce[8], 8);
        /* Calculate HMAC-SHA256 on encrypted nonce + personalized name */
        fp_crypto_hmac_sha256(&data_packet[ADD_DATA_HMAC_SHA_SIZE], 8 + personalized_name_len,
                proc_ctx.key, FP_ACC_KEYS_ACCOUNT_KEY_LENGTH, hash);
        /* First 8 bytes in data packet are starting bytes of hmac_sha256 */
        memcpy(data_packet, hash, 8);

        return gfps_notify_additional_data(svc, conn_idx, data_packet,
                FP_CRYPTO_AES_BLOCK_LENGTH + personalized_name_len);
}
#endif /* FP_PERSONALIZED_NAME */

/* Checks if salts are valid */
static bool check_salt_validity(const uint8_t *salt, const uint8_t len)
{
        for (int i = 0; i < USED_SALT_SET_SIZE; ++i) {
                if (len == used_salt_set.lengths[i]) {
                        if (0 == memcmp(salt, used_salt_set.salts[i], len)) {
                                return false;
                        }
                }
        }

        return true;
}

/* Remembers salts for validation */
static void add_salt_to_set(const uint8_t *salt, const uint8_t len)
{
        if (len > MAX_SALT_LENGTH) {
                return;
        }
        memcpy(used_salt_set.salts[used_salt_set.last_idx], salt, len);
        used_salt_set.lengths[used_salt_set.last_idx] = len;
        used_salt_set.last_idx = (used_salt_set.last_idx + 1) % USED_SALT_SET_SIZE;
}

/* Validates the request against salt values tracking */
static bool validate_request_against_salt_tracking(const uint8_t *request)
{
        uint8_t flags = request[1];
        /* Salt tracking - works only for raw request of type 0x00 */
        const bool seeker_addr_present = (flags & REQ_FLAG_PROVIDER_STARTS_PAIRING) ||
                (flags & REQ_FLAG_RETROACTIVE_ACCOUNT_KEY_WRITE);
        const uint8_t *salt = NULL;
        uint8_t length = 0;

        if (seeker_addr_present) {
                salt = request + 14;
                length = 16 - 14;
        } else {
                salt = request + 8;
                length = 16 - 8;
        }

        if (check_salt_validity(salt, length)) {
                add_salt_to_set(salt, length);
                return true;
        } else {
                /* Ignore the write request */
                return false;
        }
}

#ifdef FP_PERSONALIZED_NAME
/* Reads Provider personalized name. The name is read from NVM storage and stored in module context. */
static void read_personalized_name(void)
{
        uint16_t read_len = 0;

        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_PERSONALIZED_NAME,
                  .data = proc_ctx.personalized_name,
                  .len = FP_PERSONALIZED_NAME_MAX_LENGTH }
        };
        read_len = fp_conn_params_get_params(params, ARRAY_LENGTH(params));

        if (read_len == 0) {
                strcpy(proc_ctx.personalized_name, FP_DEFAULT_PERSONALIZED_NAME);
                return;
        }

        proc_ctx.personalized_name[read_len] = '\0';
}

/* Write Provider personalized name to NVM storage */
static void write_personalized_name(void)
{
        uint16_t write_len;

        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_PERSONALIZED_NAME,
                  .data = proc_ctx.personalized_name,
                  .len = strlen(proc_ctx.personalized_name) }
        };
        write_len = fp_conn_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != params[0].len) {
                FP_LOG_PRINTF("Problem in writing personalized name\r\n");
                FP_CHECK_ERROR(1);
        }
}
#endif /* FP_PERSONALIZED_NAME */

/* Handles key-based pairing Seeker request */
static ble_error_t key_based_pairing_req(ble_service_t *svc, const uint8_t *req)
{
        uint8_t response[MSG_LENGTH];
        uint8_t response_encrypted[MSG_LENGTH];
        uint8_t flags __UNUSED;
        ble_error_t ret = BLE_STATUS_OK;

        /* Prepare response */
        produce_response(response);
        fp_crypto_aes_ecb(proc_ctx.key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_ENCRYPT, 1,
                response, response_encrypted);
        ret = gfps_notify_pairing(svc, proc_ctx.conn_idx, response_encrypted);
        if (ret != BLE_STATUS_OK) {
                goto done;
        }

        /* Check request flags */
        flags = req[1];
        FP_LOG_PRINTF("Key-based pairing request flags: 0x%x\r\n", flags);

#if (FP_LOCATOR_TAG != 1)
        if (flags & REQ_FLAG_PROVIDER_STARTS_PAIRING) {
                ble_gap_set_io_cap(GAP_IO_CAP_DISP_YES_NO);
                ret = ble_gap_pair(proc_ctx.conn_idx, true);
                if (ret != BLE_STATUS_OK) {
                        goto done;
                }
        }
#endif /* !FP_LOCATOR_TAG */
#ifdef FP_PERSONALIZED_NAME
        if (flags & REQ_FLAG_PROVIDER_NOTIFY_NAME) {
                read_personalized_name();
                ret = notify_additional_data_name(svc, proc_ctx.conn_idx);
                if (ret != BLE_STATUS_OK) {
                        goto done;
                }
        }
#endif /* FP_PERSONALIZED_NAME */

done:
        proc_ctx.proc_state = PROCEDURE_STATE_KEY_BASED_PAIRING;

        return ret;
}

/* Handles action Seeker request */
static void action_req(ble_service_t *svc, const uint8_t *req)
{
        uint8_t flags;

        flags = req[1];
        FP_LOG_PRINTF("Action request flags: 0x%x\r\n", flags);

        if (flags & REQ_FLAG_ADDITIONAL_DATA) {
                proc_ctx.proc_state = PROCEDURE_STATE_ACTION_ADDITIONAL_DATA;
                proc_ctx.additional_data_id = req[10];
        }
}

#if (FP_LOCATOR_TAG != 1)
/* Prepares passkey block */
static void produce_passkey_block(uint8_t *block)
{
        uint8_t i;

        /* Msg type */
        block[0] = PROVIDER_PASSKEY_TYPE;
        /* block[1-3] already contains passkey */
        /* Salt remainder */
        for (i = 4; i < 16; i += 2) {
                uint16_t rand_val = fp_crypto_get_salt();
                block[i] = rand_val;
                block[i + 1] = rand_val >> 8;
        }
}
#endif /* !FP_LOCATOR_TAG */

/* Initiate procedure */
void initiate_procedure(uint16_t conn_idx)
{
        /* Start key timer to clear key after 10s */
        OS_TIMER_START(proc_ctx.key_tim, OS_TIMER_FOREVER);

        /* Set procedure as idle */
        proc_ctx.proc_state = PROCEDURE_STATE_IDLE;

        /* Remember connection index for key removal at disconnection */
        proc_ctx.conn_idx = conn_idx;
}

/* Complete procedure */
static void complete_procedure(PROCEDURE_ERR err)
{
        uint16_t conn_idx = proc_ctx.conn_idx;
        PROCEDURE_STATE proc_state = proc_ctx.proc_state;

        if (proc_ctx.conn_idx == BLE_CONN_IDX_INVALID) {
                return;
        }

        proc_ctx.conn_idx = BLE_CONN_IDX_INVALID;

        OS_TIMER_STOP(proc_ctx.key_tim, OS_TIMER_FOREVER);

        /* Discard key */
        memset(proc_ctx.key, 0, KEY_LENGTH);

        /* Set procedure as completed (i.e. none) */
        proc_ctx.proc_state = PROCEDURE_STATE_NONE;

        FP_LOG_PRINTF("Key discarded\r\n");

        FP_LOG_PRINTF("Procedure %s\r\n", err ? "failed" : "succeeded");

        if (proc_state == PROCEDURE_STATE_KEY_BASED_PAIRING) {
                if (proc_ctx.procedure_status_cb) {
                        proc_ctx.procedure_status_cb(conn_idx, FP_PAIR_REQ_STAT_COMPLETED, err);
                }
        }
}

void fp_procedure_init(fp_procedure_status_cb_t procedure_status_cb)
{
        memset(&proc_ctx, 0, sizeof(proc_ctx));

        proc_ctx.procedure_status_cb = procedure_status_cb;
        proc_ctx.conn_idx = BLE_CONN_IDX_INVALID;

        /* Create timers for failure in pairing and key timeout */
        proc_ctx.failures_tim = OS_TIMER_CREATE("failures", OS_MS_2_TICKS(PAIRING_NOT_ALLOWED_TIME),
                                OS_TIMER_ONCE, NULL, failures_tim_cb);
        OS_ASSERT(proc_ctx.failures_tim);
        proc_ctx.key_tim = OS_TIMER_CREATE("key_tmo", OS_MS_2_TICKS(KEY_TMO),
                                OS_TIMER_ONCE, NULL, key_tim_cb);
        OS_ASSERT(proc_ctx.key_tim);

#if (FP_TEST_KEY_PAIRING == 1)
        test_aes_ctr_crypt();
        test_hmac_sha256();
        fp_procedure_pairing_cb(NULL, 0, NULL, NULL);
#endif
}

void fp_procedure_deinit(void)
{
        OS_TIMER_DELETE(proc_ctx.key_tim, OS_TIMER_FOREVER);
        OS_TIMER_DELETE(proc_ctx.failures_tim, OS_TIMER_FOREVER);
}

void fp_procedure_stop(uint16_t conn_idx)
{
        if ((conn_idx == BLE_CONN_IDX_INVALID) || (fp_procedure_is_pairing_connection(conn_idx))) {
                complete_procedure(PROCEDURE_ERR_STOPPED);
        }
}

void fp_procedure_set_pairing_mode(bool enable)
{
        proc_ctx.is_pairing_mode = enable;
}

bool fp_procedure_is_pairing_mode(void)
{
        return proc_ctx.is_pairing_mode;
}

void fp_procedure_account_key_cb(uint16_t conn_idx, const uint8_t *keybuffer)
{
        /*
         * Description of the procedure:
         * https://developers.google.com/nearby/fast-pair/specifications/characteristics#AccountKey
         */

        uint8_t account_key_decrypted[MSG_LENGTH];

        if (!fp_procedure_is_pairing_connection(conn_idx)) {
                return;
        }

#if (FP_LOCATOR_TAG != 1)
        if (!proc_ctx.is_passkey_received) {
                return;
        }
#endif
        fp_crypto_aes_ecb(proc_ctx.key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_DECRYPT, 1,
                keybuffer, account_key_decrypted);
        if (account_key_decrypted[0] != 0x04) {
                /* Procedure is completed */
                complete_procedure(PROCEDURE_ERR_ACC_KEY_DECRYPT_FAILED);
                return;
        }

        /* Add account key to persistent storage */
        fp_acc_keys_add_key(account_key_decrypted);

        /* Procedure is completed */
        complete_procedure(PROCEDURE_ERR_NONE);
#if (FP_FMDN == 1)
        /* Wait for FMDN provisioning after AK is set */
        fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_WAITING);
#endif
        fp_restart_advertise();
}

void fp_procedure_pairing_cb(ble_service_t *svc, uint16_t conn_idx, const uint8_t *request,
        const uint8_t *public_key)
{
        uint8_t aes_key[AES_KEY_LENGTH];
        uint8_t output[FP_CRYPTO_SHA256_BYTES_LEN];
        uint8_t request_decrypted[MSG_LENGTH];
        uint8_t i;
        bool decrypt_successful = false;

#if (FP_TEST_KEY_PAIRING == 1)
        if (request == NULL) {
                /* This should be a test call of procedure_pairing_cb() */
                uint8_t result[MSG_LENGTH];
                uint8_t k;

                fp_crypto_aes_ecb(test_key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_DECRYPT, 1,
                        test_request, result);
                for (k = 0; k < MSG_LENGTH; k++) {
                        if (test_output[k] != result[k]) {
                                FP_LOG_PRINTF("Error in decrypt_request(), index = %d\r\n", k);
                        }
                }

                /* Exit the test call */
                return;
        }
#endif /* FP_TEST_KEY_PAIRING */

        /* Check if a Fast Pair procedure is currently active */
        if (proc_ctx.proc_state != PROCEDURE_STATE_NONE) {
                return;
        }

#if (FP_LOCATOR_TAG != 1)
        /* Reset pass key status */
        proc_ctx.is_passkey_received = false;
#endif

        if (proc_ctx.pairing_failures == FP_MAX_PAIRING_FAILURES) {
                return;
        }

        if (public_key) {
                if (!proc_ctx.is_pairing_mode) {
                        /* Ignore the write request */
                        return;
                }
#if (FP_LOCATOR_TAG == 1)
                /* For tags, only one Fast Pair pairing can be performed */
                if (fp_acc_keys_get_keys_count() > 0) {
                        return;
                }
#endif
                uECC_shared_secret(public_key, fp_anti_spoofing_private_key, aes_key, uECC_secp256r1());
                fp_crypto_sha256(output, aes_key, AES_KEY_LENGTH);
                memcpy(proc_ctx.key, output, KEY_LENGTH);

                fp_crypto_aes_ecb(proc_ctx.key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_DECRYPT, 1,
                        request, request_decrypted);
                decrypt_successful = verify_request(request_decrypted);
        } else {
                /* Try to use any of the stored keys to decrypt */
                for (i = 0; i < fp_acc_keys_get_keys_count(); i++) {
                        fp_crypto_aes_ecb(fp_acc_keys_get_key(i), FP_CRYPTO_AES_BLOCK_LENGTH,
                                FP_CRYPTO_OP_DECRYPT, 1, request, request_decrypted);
                        decrypt_successful = verify_request(request_decrypted);
                        if (decrypt_successful) {
                                memcpy(proc_ctx.key, fp_acc_keys_get_key(i), KEY_LENGTH);
                                break;
                        }
                }
        }

        if (!decrypt_successful) {
                proc_ctx.pairing_failures++;
                if (proc_ctx.pairing_failures == FP_MAX_PAIRING_FAILURES) {
                        OS_TIMER_START(proc_ctx.failures_tim, OS_TIMER_FOREVER);
                }
                return;
        }

        if (!validate_request_against_salt_tracking(request_decrypted)) {
                return;
        }

        /* Procedure is initiated */
        initiate_procedure(conn_idx);

        /* Request decryption is successful */
        proc_ctx.pairing_failures = 0;

        if (request_decrypted[0] == MSG_TYPE_ACTION_REQ) {
                action_req(svc, request_decrypted);
        } else if (request_decrypted[0] == MSG_TYPE_KEY_BASED_PAIRING_REQ) {
                ble_error_t ret;

                /* Notify about procedure status */
                if (proc_ctx.procedure_status_cb) {
                        proc_ctx.procedure_status_cb(conn_idx, FP_PAIR_REQ_STAT_INITIATED,
                                PROCEDURE_ERR_NONE);
                }

                ret = key_based_pairing_req(svc, request_decrypted);
                if (ret != BLE_STATUS_OK) {
                        complete_procedure(PROCEDURE_ERR_CONN_FAILED);
                }
        } else {
                complete_procedure(PROCEDURE_ERR_UNSUPPORTED_MSG_TYPE);
        }
}

#if (FP_LOCATOR_TAG != 1)
void fp_procedure_passkey_cb(ble_service_t *svc, uint16_t conn_idx, const uint8_t *passkey_block)
{
        uint8_t block_decrypted[MSG_LENGTH];
        uint8_t block_encrypted[MSG_LENGTH];
        uint32_t passkey;
        ble_error_t ret;

        if (!fp_procedure_is_pairing_connection(conn_idx)) {
                return;
        }

        fp_crypto_aes_ecb(proc_ctx.key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_DECRYPT, 1,
                passkey_block, block_decrypted);

        if (block_decrypted[0] == SEEKER_PASSKEY_TYPE) {
                passkey = block_decrypted[1] << 16;
                passkey += block_decrypted[2] << 8;
                passkey += block_decrypted[3];
                proc_ctx.is_passkey_received = true;

                ret = ble_gap_numeric_reply(conn_idx, (passkey == proc_ctx.passkey) ? true : false);
                FP_CHECK_ERROR(ret);

                /* Prepare response */
                produce_passkey_block(block_decrypted);
                fp_crypto_aes_ecb(proc_ctx.key, FP_CRYPTO_AES_BLOCK_LENGTH, FP_CRYPTO_OP_ENCRYPT, 1,
                        block_decrypted, block_encrypted);
                ret = gfps_notify_passkey(svc, conn_idx, block_encrypted);
                FP_CHECK_ERROR((ret == BLE_ERROR_NOT_CONNECTED) ? BLE_STATUS_OK : ret);
        } else {
                /* Decryption fails, ignore the write. Procedure is completed. */
                complete_procedure(PROCEDURE_ERR_PASSKEY_DECRYPT_FAILED);
        }
}
#endif /* !FP_LOCATOR_TAG */

void fp_procedure_additional_data_cb(uint16_t conn_idx, const uint8_t *data, uint8_t length)
{
        PROCEDURE_ERR err = PROCEDURE_ERR_PERSONALIZED_NAME_INVALID;

        if (!fp_procedure_is_pairing_connection(conn_idx)) {
                return;
        }

        if (proc_ctx.proc_state != PROCEDURE_STATE_ACTION_ADDITIONAL_DATA) {
                goto done;
        }

#ifdef FP_PERSONALIZED_NAME
        if (proc_ctx.additional_data_id == DATA_ID_PERSONALIZED_NAME) {
                uint8_t nonce[FP_CRYPTO_AES_BLOCK_LENGTH] = {0};
                uint8_t hash[FP_CRYPTO_SHA256_BYTES_LEN];
                uint8_t personalized_name_len = length - FP_CRYPTO_AES_BLOCK_LENGTH;

                if (length > ADD_DATA_PACKET_LENGTH) {
                        FP_LOG_PRINTF("Additional data length exceeds %d\r\n",
                                ADD_DATA_PACKET_LENGTH);
                        goto done;
                }
                /* Calculate HMAC-SHA256 on encrypted personalized name */
                fp_crypto_hmac_sha256(&data[ADD_DATA_HMAC_SHA_SIZE], length - ADD_DATA_HMAC_SHA_SIZE,
                        proc_ctx.key, FP_ACC_KEYS_ACCOUNT_KEY_LENGTH, hash);
                if (memcmp(hash, data, 8)) {
                        FP_LOG_PRINTF("Incorrect HMAC-SHA256\r\n");
                        goto done;
                }
                /* Copy nonce */
                memcpy(&nonce[8], &data[8], 8);
                /* Decrypt personalized name */
                fp_crypto_aes_ctr(&data[FP_CRYPTO_AES_BLOCK_LENGTH], personalized_name_len,
                        nonce, proc_ctx.key, FP_CRYPTO_OP_DECRYPT,
                        (uint8_t *) proc_ctx.personalized_name);
                proc_ctx.personalized_name[personalized_name_len] = '\0';

                /* Write to NVM storage */
                write_personalized_name();
        }
#endif /* FP_PERSONALIZED_NAME */

        err = PROCEDURE_ERR_NONE;

done:
        /* Procedure is completed */
        complete_procedure(err);
}

#if (FP_LOCATOR_TAG != 1)
void fp_procedure_save_passkey(uint32_t passkey)
{
        proc_ctx.passkey = passkey;
}
#endif /* !FP_LOCATOR_TAG */

bool fp_procedure_is_pairing_connection(uint16_t conn_idx)
{
        return OS_TIMER_IS_ACTIVE(proc_ctx.key_tim) && (proc_ctx.conn_idx == conn_idx);
}
