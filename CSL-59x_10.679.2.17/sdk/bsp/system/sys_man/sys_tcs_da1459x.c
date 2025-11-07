/**
****************************************************************************************
*
* @file sys_tcs_da1459x.c
*
* @brief TCS Handler
*
* Copyright (C) 2020-2024 Renesas Electronics Corporation and/or its affiliates.
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
#if (dg_configUSE_SYS_TCS == 1)

#include "sys_tcs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if (dg_configUSE_HW_GPADC == 1)
#include "hw_gpadc.h"
#endif

#define CS_START_CMD            0xA5A5A5A5
#define CS_SDK_VAL              0x90000000
#define CS_STOP_CMD             0x00000000
#define CS_EMPTY_VAL            0xFFFFFFFF
#define EFLASH_CS_MAIN_ADDR     0x00000000 // 1984 bytes
#define EFLASH_CS_INFO_ADDR     0x00040400

#define CS_INFO_MAX_SIZE        256 * 4 //256 entries of 4 bytes
#define CS_REST_MAX_SIZE        496 * 4 //496 entries of 4 bytes

#define MAX_REG_ADDR            0x50060F9C

typedef struct {
        uint32_t reg_address;
        bool trimmed;
} reg_trimmed_t;

/* Static allocation for tcs_data */
__RETAINED_CMI static uint32_t __tcs_data[TCS_DATA_SIZE];
__RETAINED static uint32_t* tcs_data;
__RETAINED static uint32_t tcs_data_size;

__RETAINED_CMI static sys_tcs_attr_t tcs_attributes[SYS_TCS_GROUP_MAX];
        /* Section ID=0 (GID 0) may not be used. It is used in the production
        test to indicate trim pairs to be placed in the BOOT section */
      /*{SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYS (0x01)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_COMM (0x02)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_MEM (0x03)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_TMR (0x04)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_PER (0x05)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD (0x06)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH (0x07)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_AUD (0x08)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x09)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // unknown to SDK TCS group (0x0A)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_BD_ADDR (0x0B)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PROD_INFO (0x0C)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_CHIP_ID (0x0D)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PROD_WAFER (0x0E)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TESTPROGRAM_VERSION (0x0F)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_SD_ADC_SINGLE_MODE (0x10)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_SD_ADC_DIFF_MODE (0x11)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_GP_ADC_SINGLE_MODE (0x12)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_GP_ADC_DIFF_MODE (0x13)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TEMP_SENS_25C (0x14)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_CCOEFF_LP (0x15)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_CCOEFF_HP (0x16)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_LP (0x17)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_LP (0x18)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_HP (0x19)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_HP (0x1A)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_LP_0DBM (0x1B)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_LP_6DBM (0x1C)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_HP_0DBM (0x1D)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_RAD_MODE_HP_6DBM (0x1E)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_LP_105C (0x1F)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_LP_N40C (0x20)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_HP_105C (0x21)
        {SYS_TCS_TYPE_REG_PAIR, GID_EMPTY, 0},      // SYS_TCS_GROUP_PD_SYNTH_MODE_HP_N40C (0x22)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TEMP_SENS_105C (0x23)
        {SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0},      // SYS_TCS_GROUP_TEMP_SENS_N40C (0x24)
         remaining GIDs from 0x25 to 0x7F are unknown to SDK TCS group IDs initialized as
        { SYS_TCS_TYPE_TRIM_VAL, GID_EMPTY, 0} */

static void init_tcs_attributes_array()
{
        // below array contains the GID with type SYS_TCS_TYPE_REG_PAIR
        uint8_t reg_pair_gids[] = {
                0x00,   /* reserved GID (0x0) */
                0x01,   /* SYS_TCS_GROUP_PD_SYS (0x01) */
                0x02,   /* SYS_TCS_GROUP_PD_COMM (0x02) */
                0x03,   /* SYS_TCS_GROUP_PD_MEM (0x03) */
                0x04,   /* SYS_TCS_GROUP_PD_TMR (0x04) */
                0x05,   /* SYS_TCS_GROUP_PD_PER (0x05) */
                0x06,   /* SYS_TCS_GROUP_PD_RAD (0x06) */
                0x07,   /* SYS_TCS_GROUP_PD_SYNTH (0x07) */
                0x08,   /* SYS_TCS_GROUP_PD_AUD (0x08) */
                0x10,   /* SYS_TCS_GROUP_SD_ADC_SINGLE_MODE (0x10) */
                0x11,   /* SYS_TCS_GROUP_SD_ADC_DIFF_MODE (0x11) */
        };

        uint32_t i = 0;
        for (i = 0; i < SYS_TCS_GROUP_MAX; i++) {
                //Initialize start to point to not configured GID
                tcs_attributes[i].start = GID_EMPTY;
        }

        for (i = 0; i < ARRAY_LENGTH(reg_pair_gids); i++) {
                // configure register address value pair type.
                tcs_attributes[reg_pair_gids[i]].value_type = SYS_TCS_TYPE_REG_PAIR;
        }

}

typedef enum {
        CS_EFLASH_INFO = 0,     /* CS in eFlash info block*/
        CS_EFLASH_MAIN ,        /* CS in eFlash main block*/
        CS_QSPI                 /* CS in QSPI FLASH */
} SYS_TCS_SOURCE;

static uint32_t fetch_tcs_entry(SYS_TCS_SOURCE source, uint32_t address)
{
        uint32_t cs_value = CS_EMPTY_VAL;

        if (source == CS_EFLASH_INFO) {
                cs_value = *(volatile uint32_t*)(MEMORY_EFLASH_S_BASE + EFLASH_CS_INFO_ADDR + address);
        }
        else if (source == CS_EFLASH_MAIN) {
                cs_value = *(volatile uint32_t*)(MEMORY_EFLASH_S_BASE + EFLASH_CS_MAIN_ADDR + address);
        }
        else if (source == CS_QSPI) {
                cs_value =  *(volatile uint32_t*)(MEMORY_QSPIF_S_BASE + address);
        }

        return cs_value;
}

static void store_tcs(uint32_t address, uint8_t gid_len, SYS_TCS_SOURCE source)
{
        int i = 0;
        uint16_t index;
        ASSERT_ERROR(tcs_data);

        //address --> GID header
        uint32_t value = fetch_tcs_entry(source, address);
        SYS_TCS_GID gid = (uint8_t)(value & 0x000000FF);

        if (gid >= SYS_TCS_GROUP_MAX) {
                return;
        }

        SYS_TCS_TYPE type = sys_tcs_get_value_type(gid);

        if (type == SYS_TCS_TYPE_TRIM_VAL) {
                //SYS_TCS_TYPE_TRIM_VAL could have different sizes during parsing
                //it is acceptable multiple instances with same GID to have different sizes,
                //only the newest will be finally stored. We cannot identify the newest but
                //at least the ones that fit will be stored until the newest is parsed
                if (gid_len != tcs_attributes[gid].size) {
                        return;
                }
        }

        /* start of storing TCS entries */

        index = tcs_attributes[gid].start;

        //for SYS_TCS_TYPE_REG_PAIR search tcs_data to find empty slot
        //for SYS_TCS_TYPE_TRIM_VAL fragmentation is not supported and always the newest entries are stored
        if (type == SYS_TCS_TYPE_REG_PAIR) {
                uint16_t gid_start = index;
                uint8_t gid_size = tcs_attributes[gid].size;

                //search tcs_data to find empty slot
                while (tcs_data[index] != 0) {
                        /* check if the index is inside the allocated space in tcs_data[] for this GID */
                        if (index >= (gid_start + gid_size)) {
                                /* Block in development mode */
                                ASSERT_WARNING(0);
                                return;
                        }
                        //go to next register address
                        index += 2;
                }
        }

        while (i < gid_len) { //4 bytes entries
                address += 4;
                tcs_data[index] = fetch_tcs_entry(source, address);
                index++;
                i++;

        }
}

/* The calculated size in bytes, that is the number
 * of entries * sizeof(int)
 * returns the size in bytes
 */
static uint16_t get_size_of_cs(SYS_TCS_SOURCE source)
{
        uint32_t value = 0;
        uint32_t address = 0;
        SYS_TCS_GID gid;
        uint16_t max_size = (source == CS_EFLASH_INFO ? CS_INFO_MAX_SIZE : CS_REST_MAX_SIZE);
        uint16_t size = 0;

        value = fetch_tcs_entry(source, address);
        if (value != CS_START_CMD) {
                return 0;
        }

        //check next entry
        address += 4;
        while (address < max_size) {
                value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MAX_REG_ADDR) { //address value pair parsed by bootrom, skip this value
                        address += 0x4;
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        uint8_t tcs_len = (value & 0x0000FF00) >> 8;
                        gid = (uint8_t)(value & 0x000000FF);
                        address += tcs_len * 4; //skip next tcs values.

                        if (gid >= SYS_TCS_GROUP_MAX) {
                                address += 0x4; //skip this entry
                                continue;
                        }

                        if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_TRIM_VAL) {
                                /*always keep the last found size */
                                if (tcs_attributes[gid].size != tcs_len) {
                                        /* update with new size */
                                        size -= 4 * tcs_attributes[gid].size;
                                        size += 4 * tcs_len;
                                        tcs_attributes[gid].size = tcs_len;
                                }
                        } else {
                                //check that SYS_TCS_TYPE_REG_PAIR values are of an even number
                                if (sys_tcs_get_value_type(gid) == SYS_TCS_TYPE_REG_PAIR) {
                                        ASSERT_ERROR((tcs_len & 0x01) == 0);
                                }

                                size += 4 * tcs_len; //size should be in bytes
                                tcs_attributes[gid].size += tcs_len;
                        }
                }
                // go to next word this value is not related to TCS
                address += 0x4;
        }
        return size;
}

static void store_cs_attributes(SYS_TCS_SOURCE source, uint16_t size)
{
        uint32_t address = 0x4; //skip CS_START_CMD

        while (address < size) {
                uint32_t value = fetch_tcs_entry(source, address);

                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MAX_REG_ADDR) { // address - value pair
                        address += 0x4;
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        uint8_t gid_len = (value & 0x00000FF00) >> 8;
                        store_tcs(address, gid_len, source);
                        address += gid_len*4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
        }
}

void sys_tcs_get_trim_values_from_cs(void)
{
        uint16_t total_size = 0;
        SYS_TCS_SOURCE source = CS_EFLASH_INFO;
        uint16_t tcs_location_sizes[3] = {0};

#if (dg_configUSE_SYS_TCS == 0)
        return;
#endif

        init_tcs_attributes_array();

        //first locate start of CS in INFO
        for (source = CS_EFLASH_INFO; source <= CS_QSPI; source++) {
                tcs_location_sizes[source] = get_size_of_cs(source);
                if (tcs_location_sizes[CS_EFLASH_MAIN]) {
                        //if main block has CS information exit for loop, no need to check
                        //next in priority CS block in QSPI.
                        break;
                }

                total_size += tcs_location_sizes[source];
        }

        if (total_size == 0) {
                /* Trimmed devices are only supported */
                ASSERT_ERROR(0);
                /* No CS information found*/
                return;
        }

        /* The calculated size in bytes, taking into account that one entry is 4 bytes */
        ASSERT_ERROR(total_size < CS_REST_MAX_SIZE + CS_INFO_MAX_SIZE);

        /* Static allocation for tcs_data */
        tcs_data = __tcs_data;

        //convert sizes to offsets in the tcs table and
        //set the sizes of the GID attributes
        uint8_t gid_offset = 0;
        for (uint8_t gid = 0; gid < SYS_TCS_GROUP_MAX; gid++) {
                if (tcs_attributes[gid].size != 0) {
                        tcs_attributes[gid].start = gid_offset;
                        gid_offset += tcs_attributes[gid].size;
                }
        }
        /* store the tcs_data used size*/
        tcs_data_size = gid_offset;
        //Store TCS values
        if (tcs_location_sizes[CS_EFLASH_INFO]) {
                store_cs_attributes(CS_EFLASH_INFO, CS_INFO_MAX_SIZE);
        }

        if (tcs_location_sizes[CS_EFLASH_MAIN]) {
                store_cs_attributes(CS_EFLASH_MAIN, CS_REST_MAX_SIZE);
        } else if (tcs_location_sizes[CS_QSPI]) {
                store_cs_attributes(CS_QSPI, CS_REST_MAX_SIZE);
        }
}

static bool parse_cs_for_booter_reg_pair(SYS_TCS_SOURCE source, uint8_t num, reg_trimmed_t *reg, uint16_t block_sz)
{
        uint32_t address = 0;
        uint32_t value = 0;

        value = fetch_tcs_entry(source, address);
        if (value != CS_START_CMD) {
                /* search for start command to continue parsing*/
                return false;
        }

        while (address < block_sz) {
                if ((value == CS_STOP_CMD) || (value == CS_EMPTY_VAL)) {
                        break; // End of CS
                } else if (value <= MAX_REG_ADDR) { // address - value pair
                        for (uint8_t i = 0; i < num; i++) {
                                if (value == reg[i].reg_address) {
                                        reg[i].trimmed = true;
                                        break;
                                }
                        }
                        address += 0x4; //skip value entry
                } else if ((value & 0xF0000000) == CS_SDK_VAL) {  // SDK value
                        address += ((value & 0x00000FF00) >> 8)*4; //skip next tcs values.
                }
                address += 0x4; //advance address by 4 bytes
                value = fetch_tcs_entry(source, address);
        }
        return true;
}

bool sys_tcs_reg_pairs_in_cs(const uint32_t *reg_address, uint8_t num, bool *trimmed_reg)
{
        uint8_t i = 0;
        bool ret = true;

        reg_trimmed_t regs[num];

        ASSERT_ERROR(reg_address);
        ASSERT_ERROR(trimmed_reg);

        if (num == 0) {
                return false;
        }

        for (i = 0; i < num; i++ ) {
                regs[i].reg_address = reg_address[i];
                regs[i].trimmed = trimmed_reg[i];
        }

        /* always parse info block */
        parse_cs_for_booter_reg_pair(CS_EFLASH_INFO, num, regs, CS_INFO_MAX_SIZE);


        if (parse_cs_for_booter_reg_pair(CS_EFLASH_MAIN, num, regs, CS_REST_MAX_SIZE) == false) {
                /* parse qspi block if main block doesn't contain CS entries*/
                parse_cs_for_booter_reg_pair(CS_QSPI, num, regs, CS_REST_MAX_SIZE);
        }

        for (i = 0; i < num; i++ ) {
                trimmed_reg[i] = regs[i].trimmed;
                if (trimmed_reg[i] == false) {
                        ret = false;
                }
        }
        return ret;
}

__RETAINED_HOT_CODE void sys_tcs_apply_reg_pairs(SYS_TCS_GID gid)
{
        if (tcs_data == NULL) {
                return;
        }
        ASSERT_WARNING(gid < SYS_TCS_GROUP_MAX);
        ASSERT_WARNING(tcs_attributes[gid].value_type == SYS_TCS_TYPE_REG_PAIR);

        uint8_t start = tcs_attributes[gid].start;
        int size = (int)tcs_attributes[gid].size;

        while (size > 0) {
                *(uint32_t *)tcs_data[start] = tcs_data[start+1];
                size -= 2;
                start += 2;
        }
}


__RETAINED_CODE sys_tcs_attr_t* sys_tcs_get_tcs_attributes_ptr(void)
{
        return tcs_attributes;
}

__RETAINED_CODE uint32_t* sys_tcs_get_tcs_data_ptr(void)
{
        return tcs_data;
}

uint32_t sys_tcs_get_tcs_data_size()
{
        return tcs_data_size;
}

#if (dg_configUSE_HW_SDADC == 1)
#include "hw_sdadc.h"
void hw_sdadc_get_trimmed_values(HW_SDADC_INPUT_MODE mode, int16_t *gain, int16_t *offs)
{
        uint32_t *values;
        uint8_t size;

        switch (mode) {
        case HW_SDADC_INPUT_MODE_SINGLE_ENDED:
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_SD_ADC_SINGLE_MODE, &values, &size);
                break;
        case HW_SDADC_INPUT_MODE_DIFFERENTIAL:
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_SD_ADC_DIFF_MODE, &values, &size);
                break;
        default:
                values = NULL;
                break;
        }
        ASSERT_ERROR(values != NULL);

        for (int i = 0; i < size; i+=2) {
                if ((uint32_t *)values[i] == &SDADC->SDADC_GAIN_CORR_REG) {
                        *gain = values[i+1] & REG_MSK(SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR);
                } else if ((uint32_t *)values[i] == &SDADC->SDADC_OFFS_CORR_REG) {
                        *offs = values[i+1] & REG_MSK(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR);
                }
        }
}
#endif /* dg_configUSE_HW_SDADC */

uint32_t sys_tcs_get_testprogram_version()
{
        uint32_t *values;
        uint8_t size;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_TESTPROGRAM_VERSION, &values, &size);
        return *values;
}

#if (dg_configUSE_HW_GPADC == 1)
#include "hw_gpadc.h"
__WEAK bool hw_gpadc_get_trimmed_offsets_from_cs(uint8_t mode, uint16_t *offp, uint16_t *offn)
{
        uint32_t *values;
        uint8_t size;

        switch (mode) {
        case HW_GPADC_INPUT_MODE_SINGLE_ENDED:
                sys_tcs_get_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, &values, &size);
                break;
        case HW_GPADC_INPUT_MODE_DIFFERENTIAL:
                sys_tcs_get_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, &values, &size);
                break;
        default:
                return false;
        }

        if (size != 2) {
                return false;
        }

        *offn = ((*(values + 1) & 0xFFFF0000) >> 16) & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
        *offp = ( *(values + 1) & 0xFFFF) & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
        return true;
}
#endif /* dg_configUSE_HW_GPADC */
#endif /* (dg_configUSE_SYS_TCS == 1) */
