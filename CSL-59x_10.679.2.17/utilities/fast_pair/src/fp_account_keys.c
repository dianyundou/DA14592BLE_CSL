/**
 ****************************************************************************************
 *
 * @file fp_account_keys.c
 *
 * @brief Google Fast Pair account keys module implementation
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
#include "ble_gap.h"
#include "fp_crypto.h"
#include "fp_core.h"
#include "fp_conn_params.h"

#include "fp_account_keys.h"

/* Maximum length of bloom filter */
#define BLOOM_FILTER_MAX_LENGTH         (FP_ACCOUNT_KEYS_COUNT + 3 + FP_ACCOUNT_KEYS_COUNT / 5)
/* Account key data payload length */
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
#define ACCOUNT_KEY_DATA_LEN            (4 + BLOOM_FILTER_MAX_LENGTH + 3 + 1 + FP_BATTERIES_COUNT)
#else
#define ACCOUNT_KEY_DATA_LEN            (4 + BLOOM_FILTER_MAX_LENGTH + 3)
#endif

/* Account key data */
#define ACCOUNT_KEY_DATA_SHOW_UI_INDICATION     (0)
#define ACCOUNT_KEY_DATA_HIDE_UI_INDICATION     (1 << 1)
#define ACCOUNT_KEY_DATA_SALT                   (1)

/* First byte of valid account key */
#define ACCOUNT_KEY_MAGIC_NUMBER                (0x04)

#define BATT_SHOW_UI_INDICATION                 (0x03)
#define BATT_HIDE_UI_INDICATION                 (0x04)
#define BATT_CHARGING                           (0x80)
#define BATT_NOT_CHARGING                       (0)
#define BATT_UNKNOWN_VALUE                      (0x7f)
#define BATT_NUM(A)                             ((A[0] & 0xF0) >> 4)

/* List of account keys */
typedef uint8_t account_key_list_t[FP_ACCOUNT_KEYS_COUNT][FP_ACC_KEYS_ACCOUNT_KEY_LENGTH];

/* Advertise structure used when no account keys are available */
static const gap_adv_ad_struct_t adv_struct_no_account_keys[] = {
        GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_SVC_DATA,
                                0x2C, 0xFE,       /* 0xFE2C (Fast Pair Service UUID) */
                                0x00,             /* no flags */
                                0x00)             /* account key list is empty */
};

/* Google Fast Pair account keys module context */
__FP_RETAINED static struct {
        gap_adv_ad_struct_t adv_struct;
        uint8_t account_key_payload[ACCOUNT_KEY_DATA_LEN];
        bool regenerate_adv_struct;
        account_key_list_t account_key_list;
        uint8_t account_key_list_count;
        uint16_t salt;
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
        uint8_t battery_information[4];
#endif
#if (FP_FMDN == 1)
        uint8_t ephemeral_identity_key[FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH];
        bool is_ephemeral_identity_key_valid;
#endif
} acc_keys_ctx;

/* Swap byte order */
static uint32_t swap_endian(uint32_t in)
{
        uint32_t b[4];

        b[0] = (in & 0x000000ff) << 24u;
        b[1] = (in & 0x0000ff00) << 8u;
        b[2] = (in & 0x00ff0000) >> 8u;
        b[3] = (in & 0xff000000) >> 24u;

        return (b[0] | b[1] | b[2] | b[3]);
}

/* Get bloom filter length */
static uint8_t get_bloom_filter_length(uint8_t keys_count)
{
        return keys_count + 3 + (keys_count / 5); /* = 1.2 * keys_count + 3 */
}

#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
/* Prepare battery information for advertise payload */
static void prepare_battery_information(void)
{
        uint8_t i;
        fp_battery_info_t *info = fp_get_battery_information();

        acc_keys_ctx.battery_information[0] = (FP_BATTERIES_COUNT << 4 ) |
                (fp_get_battery_ui_indication() ? BATT_SHOW_UI_INDICATION : BATT_HIDE_UI_INDICATION);
        for (i = 0; i < FP_BATTERIES_COUNT; i++) {
                acc_keys_ctx.battery_information[i + 1] =
                        (info[i].level == 0xFF ? BATT_UNKNOWN_VALUE : info[i].level);
                acc_keys_ctx.battery_information[i + 1] +=
                        (info[i].is_charging ? BATT_CHARGING : BATT_NOT_CHARGING);
        }
}
#endif /* FP_BATTERY_NOTIFICATION && FP_BATTERIES_COUNT */

/* Calculate bloom filter on account key list */
static void bloom_filter(account_key_list_t key_list, uint8_t key_list_count, uint16_t salt,
        uint8_t *filter, uint8_t *filter_len, const uint8_t *battery_info)
{
        uint8_t i, j;
        uint8_t input[FP_ACC_KEYS_ACCOUNT_KEY_LENGTH + 2 + 4];
        uint32_t hash[FP_CRYPTO_SHA256_BYTES_LEN / sizeof(uint32_t)];
        uint32_t M;
        uint32_t inputLength;

        *filter_len = get_bloom_filter_length(key_list_count);
        memset(filter, 0, *filter_len);

        for (i = 0; i < key_list_count; i++) {
                /* Prepare buffer for sha256 (account key + salt + battery info) */
                memcpy(input, key_list[i], FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
                input[FP_ACC_KEYS_ACCOUNT_KEY_LENGTH] = (uint8_t)(salt >> 8);
                input[FP_ACC_KEYS_ACCOUNT_KEY_LENGTH + 1 ] = (uint8_t)salt;
                inputLength = FP_ACC_KEYS_ACCOUNT_KEY_LENGTH + sizeof(salt);
                if (battery_info) {
                        input[inputLength] = battery_info[0];
                        for (j = 0; j < BATT_NUM(battery_info); j++) {
                                input[inputLength + 1 + j] = battery_info[j + 1];
                        }
                        inputLength += (1 + BATT_NUM(battery_info));
                }

                /* Calculate sha256 on inputBuffer */
                fp_crypto_sha256((uint8_t*)hash, input, inputLength);

                /* Swap words endianess */
                for (j = 0; j < ARRAY_LENGTH(hash); j++) {
                        hash[j] = swap_endian(hash[j]);
                }

                /* Calculate filter */
                for (j = 0; j < ARRAY_LENGTH(hash); j++) {
                        M = hash[j] % (*filter_len * 8);
                        filter[M/8] |= (1 << (M % 8));
                }
        }
}

#if (FP_TEST_BLOOM_FILTER == 1)
static void check_result(char *test_vector, uint8_t filter[], uint8_t filter_len, uint8_t result[], uint8_t result_len)
{
        if (filter_len != result_len) {
                FP_LOG_PRINTF("%s: incorrect bloom filter length: %d\r\n", test_vector, result_len);
        }
        for (uint8_t i = 0; i < result_len; i++) {
                if (filter[i] != result[i]) {
                        FP_LOG_PRINTF("%s: incorrect bloom filter, index: %d, %x\r\n", test_vector, i, result[i]);
                }
        }

}

/* Test bloom filter implementation with Google's test vector */
static void test_bloom_filter(void)
{
        uint16_t salt = 0xC7C8;
        uint8_t ak1[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                          0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
        uint8_t ak2[] = { 0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44,
                          0x55, 0x55, 0x66, 0x66, 0x77, 0x77, 0x88, 0x88 };
        uint8_t battery_data[] = { 0x33, 0x40, 0x40, 0x40 };
        uint8_t filter_1ak[] = { 0x02, 0x0C, 0x80, 0x2A };
        uint8_t filter_1ak_with_battery[] = { 0x01, 0x01, 0x46, 0x0A };
        uint8_t filter_2ak[] = { 0x84, 0x4A, 0x62, 0x20, 0x8B };
        uint8_t filter_2ak_with_battery[] = { 0x46, 0x15, 0x24, 0xD0, 0x08 };
        uint8_t result[sizeof(filter_2ak_with_battery)];
        uint8_t result_len;
        account_key_list_t account_key_list;

        memcpy(account_key_list[0], ak1, sizeof(ak1));
        memcpy(account_key_list[1], ak2, sizeof(ak2));

        /* TV1: single account key */
        bloom_filter(account_key_list, 1, salt, result, &result_len, NULL);
        check_result("TV1", filter_1ak, sizeof(filter_1ak), result, result_len);

        /* TV2: single account key with battery data */
        bloom_filter(account_key_list, 1, salt, result, &result_len, battery_data);
        check_result("TV2", filter_1ak_with_battery, sizeof(filter_1ak_with_battery), result, result_len);

        /* TV3: 2 account keys */
        bloom_filter(account_key_list, 2, salt, result, &result_len, NULL);
        check_result("TV3", filter_2ak, sizeof(filter_2ak), result, result_len);

        /* TV4: 2 account keys and battery data */
        bloom_filter(account_key_list, 2, salt, result, &result_len, battery_data);
        check_result("TV4", filter_2ak_with_battery, sizeof(filter_2ak_with_battery), result, result_len);
}
#endif /* FP_TEST_BLOOM_FILTER */

/* Read account key list from NVM to acc_keys_ctx structure */
static void read_account_key_list(void)
{
        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_ACCOUNT_KEY_LIST,
                  .data = acc_keys_ctx.account_key_list,
                  .len = sizeof(acc_keys_ctx.account_key_list) }
        };
        fp_conn_params_get_params(params, ARRAY_LENGTH(params));

        uint8_t num_of_acc_keys = params[0].ret_len / FP_ACC_KEYS_ACCOUNT_KEY_LENGTH;
        if (num_of_acc_keys < FP_ACCOUNT_KEYS_COUNT) {
                memset(&acc_keys_ctx.account_key_list[num_of_acc_keys], 0xff,
                        (FP_ACCOUNT_KEYS_COUNT - num_of_acc_keys) * FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
        }
}

#if (FP_FMDN == 1)
/* Verify if ephemeral identity key is set */
static bool is_ephemeral_identity_key_set(void)
{
        uint8_t i;

        for (i = 0; i < FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH; i++) {
                if (acc_keys_ctx.ephemeral_identity_key[i] != 0xFF) {
                        break;
                }
        }

        return ((i == FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH) ? false : true);
}

/* Read ephemeral identity key from NVM and set it in acc_keys_ctx structure */
static void read_ephemeral_identity_key(void)
{
        uint16_t read_len;

        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_EPHEMERAL_IDENTITY_KEY,
                  .data = acc_keys_ctx.ephemeral_identity_key,
                  .len = FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH },
        };
        read_len = fp_conn_params_get_params(params, ARRAY_LENGTH(params));

        if (read_len != FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH) {
                memset(acc_keys_ctx.ephemeral_identity_key, 0xff,
                        FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);
        }

        acc_keys_ctx.is_ephemeral_identity_key_valid = is_ephemeral_identity_key_set();
}

/* Write ephemeral identity key to NVM storage */
static int write_ephemeral_identity_key(const uint8_t *identity_key)
{
        uint16_t write_len;

        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_EPHEMERAL_IDENTITY_KEY,
                  .data = (uint8_t *) identity_key,
                  .len = FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH },
        };
        write_len = fp_conn_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH) {
                FP_LOG_PRINTF("Problem in writing Ephemeral Identity Key\r\n");
                return 1;
        }

        return 0;
}
#endif /* FP_FMDN */

/* Count stored account keys */
static uint8_t count_account_keys(void)
{
        uint8_t i;
        uint8_t keys_num = 0;

        for (i = 0; i < FP_ACCOUNT_KEYS_COUNT; i++) {
                if (acc_keys_ctx.account_key_list[i][0] == ACCOUNT_KEY_MAGIC_NUMBER) {
                        keys_num++;
                } else {
                        break;
                }
        }

        return keys_num;
}

int fp_acc_keys_init(void)
{
        memset(&acc_keys_ctx, 0, sizeof(acc_keys_ctx));

        /* Read account key list from NVM storage */
        read_account_key_list();

#if (FP_FMDN == 1)
        read_ephemeral_identity_key();
#endif

        /* Count account keys in the list */
        acc_keys_ctx.account_key_list_count = count_account_keys();

        /* Initialize salt */
        fp_acc_keys_generate_new_salt();

#if (FP_TEST_BLOOM_FILTER == 1)
        test_bloom_filter();
#endif
        acc_keys_ctx.regenerate_adv_struct = true;

        return 0;
}

int fp_acc_keys_clean(void)
{
        uint16_t write_len = 0;

        memset(acc_keys_ctx.account_key_list, 0xff, sizeof(acc_keys_ctx.account_key_list));
        acc_keys_ctx.account_key_list_count = 0;

        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_ACCOUNT_KEY_LIST,
                  .data = acc_keys_ctx.account_key_list,
                  .len = sizeof(acc_keys_ctx.account_key_list) }
        };
        write_len = fp_conn_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != sizeof(acc_keys_ctx.account_key_list)) {
                FP_LOG_PRINTF("Problem in cleaning account key list\r\n");
                return 1;
        }

        return 0;
}

void fp_acc_keys_schedule_new_advertise_struct(void)
{
        acc_keys_ctx.regenerate_adv_struct = true;
}

const gap_adv_ad_struct_t *fp_acc_keys_get_advertise_struct(void)
{
        if (acc_keys_ctx.account_key_list_count == 0) {
                return adv_struct_no_account_keys;
        }

        if (acc_keys_ctx.regenerate_adv_struct) {
                /* Prepare advertise struct with account key filter */
                uint8_t index;
                uint8_t filter_len;

                /* Set Fast Pair Service UUID field */
                acc_keys_ctx.account_key_payload[0] = 0x2C;
                acc_keys_ctx.account_key_payload[1] = 0xFE;

                /* Set reserved flags field */
                acc_keys_ctx.account_key_payload[2] = 0x00;

#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
                /* Prepare battery information */
                prepare_battery_information();
#endif
                /* Generate and add account key filter */
                bloom_filter(acc_keys_ctx.account_key_list, acc_keys_ctx.account_key_list_count,
                        acc_keys_ctx.salt, &(acc_keys_ctx.account_key_payload[4]), &filter_len,
#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
                        acc_keys_ctx.battery_information);
#else
                        NULL);
#endif
                acc_keys_ctx.account_key_payload[3] =
#if (FP_LOCATOR_TAG == 1)
                        (filter_len << 4) | ACCOUNT_KEY_DATA_HIDE_UI_INDICATION;
#else
                        (filter_len << 4) | (fp_get_acc_key_filter_ui_indication() ?
                                                ACCOUNT_KEY_DATA_SHOW_UI_INDICATION :
                                                ACCOUNT_KEY_DATA_HIDE_UI_INDICATION);
#endif /* FP_LOCATOR_TAG */

                /* Add salt field */
                index = 4 + filter_len;
                acc_keys_ctx.account_key_payload[index++] = (sizeof(acc_keys_ctx.salt) << 4) | ACCOUNT_KEY_DATA_SALT;
                acc_keys_ctx.account_key_payload[index] = (uint8_t)(acc_keys_ctx.salt >> 8);
                acc_keys_ctx.account_key_payload[index + 1] = (uint8_t)acc_keys_ctx.salt;
                index += sizeof(acc_keys_ctx.salt);

#if (FP_BATTERY_NOTIFICATION == 1) && (FP_BATTERIES_COUNT != 0)
                uint8_t i;
                /* Add battery information */
                acc_keys_ctx.account_key_payload[index++] = acc_keys_ctx.battery_information[0];
                for (i = 0; i < BATT_NUM(acc_keys_ctx.battery_information); i++) {
                        acc_keys_ctx.account_key_payload[index++] =
                                acc_keys_ctx.battery_information[i + 1];
                }
#endif
                acc_keys_ctx.adv_struct.type = GAP_DATA_TYPE_UUID16_SVC_DATA;
                acc_keys_ctx.adv_struct.len = index;
                acc_keys_ctx.adv_struct.data = acc_keys_ctx.account_key_payload;

                acc_keys_ctx.regenerate_adv_struct = false;
        }

        return &acc_keys_ctx.adv_struct;
}

void fp_acc_keys_generate_new_salt(void)
{
        acc_keys_ctx.salt = fp_crypto_get_salt();
        acc_keys_ctx.regenerate_adv_struct = true;
}

uint8_t *fp_acc_keys_get_key(uint8_t index)
{
        if (index >= acc_keys_ctx.account_key_list_count) {
                return NULL;
        }

        return acc_keys_ctx.account_key_list[index];
}

uint8_t fp_acc_keys_get_keys_count(void)
{
        return acc_keys_ctx.account_key_list_count;
}

void fp_acc_keys_add_key(const uint8_t *key)
{
        uint8_t index = acc_keys_ctx.account_key_list_count;
        uint16_t write_len = 0;

        /* Check if there is space for a new key */
        if (index >= FP_ACCOUNT_KEYS_COUNT) {
                /* If not, replace the last item on the list */
                index = FP_ACCOUNT_KEYS_COUNT - 1;
        } else {
                acc_keys_ctx.account_key_list_count++;
        }
        /* Write a new key to the slot */
        memcpy(acc_keys_ctx.account_key_list[index], key, FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);

        /* Store the new account key list */
        fp_conn_params_t params[] = {
                { .param = FP_CONN_PARAMS_BLE_ACCOUNT_KEY_LIST,
                  .data = &acc_keys_ctx.account_key_list[0],
                  .len = FP_ACC_KEYS_ACCOUNT_KEY_LENGTH * acc_keys_ctx.account_key_list_count}
        };

        write_len = fp_conn_params_set_params(params, ARRAY_LENGTH(params));

        if (write_len != (FP_ACC_KEYS_ACCOUNT_KEY_LENGTH * acc_keys_ctx.account_key_list_count)) {
                FP_LOG_PRINTF("Problem in writing account key list\r\n");
                FP_CHECK_ERROR(1);
        }

        acc_keys_ctx.regenerate_adv_struct = true;
}

void fp_acc_keys_update_key_usage(uint8_t index)
{
        if (index == 0) {
                return;
        }

        if (index >= acc_keys_ctx.account_key_list_count) {
                return;
        }

        uint8_t buff[FP_ACC_KEYS_ACCOUNT_KEY_LENGTH];
        memcpy(buff, acc_keys_ctx.account_key_list[index], FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);

#if (FP_FMDN == 1)
        /*
         * The Owner Account Key (the first Account Key introduced after a factory reset)
         * must not be removed when we run out of free Account Key slots.
         */
#define RETAINED_ACCOUNT_KEYS   1
#else
#define RETAINED_ACCOUNT_KEYS   0
#endif /* FP_FMDN */

        for (int i = index - 1; i >= RETAINED_ACCOUNT_KEYS; --i) {
                memcpy(acc_keys_ctx.account_key_list[i + 1], acc_keys_ctx.account_key_list[i],
                        FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
        }
        memcpy(acc_keys_ctx.account_key_list[0], buff, FP_ACC_KEYS_ACCOUNT_KEY_LENGTH);
}

#if (FP_FMDN == 1)
uint8_t *fp_acc_keys_get_ephemeral_identity_key(void)
{
        return (acc_keys_ctx.is_ephemeral_identity_key_valid ?
                acc_keys_ctx.ephemeral_identity_key : NULL);
}

int fp_acc_keys_set_ephemeral_identity_key(const uint8_t *identity_key)
{
        acc_keys_ctx.is_ephemeral_identity_key_valid = true;
        memcpy(acc_keys_ctx.ephemeral_identity_key, identity_key,
                FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);

        fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STARTED);

        return write_ephemeral_identity_key(identity_key);
}

int fp_acc_keys_clear_ephemeral_identity_key(void)
{
        acc_keys_ctx.is_ephemeral_identity_key_valid = false;
        memset(acc_keys_ctx.ephemeral_identity_key, 0xFF, FP_FMDN_EPHEMERAL_IDENTITY_KEY_LENGTH);

        fp_set_fmdn_provisioning_state(FP_FMDN_PROVISIONING_STOPPED);

        return write_ephemeral_identity_key(acc_keys_ctx.ephemeral_identity_key);
}

uint8_t *fp_acc_keys_get_owner_key(void)
{
        if (acc_keys_ctx.account_key_list_count == 0) {
                return NULL;
        }

        return acc_keys_ctx.account_key_list[0];
}
#endif /* FP_FMDN */
