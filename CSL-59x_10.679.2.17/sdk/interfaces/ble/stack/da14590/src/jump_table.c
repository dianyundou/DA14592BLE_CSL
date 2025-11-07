/**
 ****************************************************************************************
 *
 * @file jump_table.c
 *
 * @brief Heaps and configuration table setup.
 *
 * Copyright (C) 2017-2023 Renesas Electronics Corporation and/or its affiliates.
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
#ifdef CONFIG_USE_BLE

/*
 * INCLUDES
 ****************************************************************************************
 */
#include <string.h>
#include "co_version.h"
#include "ble_config.h"
#include "sdk_defs.h"
#include "cmsis_compiler.h"
#include "co_version.h"
#include "rwip.h"
#include "arch.h"
#include "ble_stack_config_tables.h"
#include "cmac_config_tables.h"
#include "gapc.h"

#define BLE_STACK_INT_HEAP_POS                  ( 0x40108000 )          // Address of cache RAM
#define BLE_STACK_INT_HEAP_SIZE                 ( 8192 )                // Size of heap in cache RAM
#define BLE_STACK_HEAP_SIZE_MIN                 ( 12 )                  // Minimum allowed size
#ifndef BLE_STACK_EXT_HEAP_SIZE
        /* Calculate the BLE stack total heap space according to the maximum number of connections
           (the calculation formula is equivalent to the calculation previously done internally). */
        #if (dg_configBLE_CONNECTIONS_MAX == 1)
        #define BLE_STACK_EXT_HEAP_SIZE         ( BLE_STACK_HEAP_SIZE_MIN )
        #else
        #define BLE_STACK_EXT_HEAP_SIZE         ( (1460 * ( dg_configBLE_CONNECTIONS_MAX - 1 )) + BLE_STACK_HEAP_SIZE_MIN )
        #endif
#elif ( BLE_STACK_EXT_HEAP_SIZE < BLE_STACK_HEAP_SIZE_MIN )
        #undef BLE_STACK_EXT_HEAP_SIZE
        #define BLE_STACK_EXT_HEAP_SIZE         ( BLE_STACK_HEAP_SIZE_MIN )
#endif

uint32_t rwip_heap_env[BLE_STACK_EXT_HEAP_SIZE / sizeof(uint32_t)] __attribute__((section(".cmi_data_ext_area")));
uint32_t rwip_heap_db[BLE_STACK_HEAP_SIZE_MIN / sizeof(uint32_t)] __attribute__((section(".cmi_data_ext_area")));
uint32_t rwip_heap_non_ret[BLE_STACK_HEAP_SIZE_MIN / sizeof(uint32_t)] __attribute__((section(".cmi_data_ext_area")));

const uint32_t rom_cfg_table_const[] =
{
    [rwip_heap_env_addr_pos]                        = (uint32_t) &rwip_heap_env[0],
    [rwip_heap_env_size_pos]                        = (uint32_t) BLE_STACK_EXT_HEAP_SIZE,
    [rwip_heap_msg_addr_pos]                        = (uint32_t) BLE_STACK_INT_HEAP_POS,
    [rwip_heap_msg_size_pos]                        = (uint32_t) BLE_STACK_INT_HEAP_SIZE,
    [rwip_heap_non_ret_addr_pos]                    = (uint32_t) &rwip_heap_non_ret[0],
    [rwip_heap_non_ret_size_pos]                    = (uint32_t) BLE_STACK_HEAP_SIZE_MIN,
#if (BLE_HOST_PRESENT)
    [rwip_heap_db_addr_pos]                         = (uint32_t) &rwip_heap_db[0],
    [rwip_heap_db_size_pos]                         = (uint32_t) BLE_STACK_HEAP_SIZE_MIN,
#endif /* (BLE_HOST_PRESENT) */

#if (BLE_EMB_PRESENT)
    [man_id_pos]                                    = 0x00D2, // Dialog Semi Id
    [ea_timer_prog_delay_pos]                       = 1,
    [ea_clock_corr_lat_pos]                         = 2,
    [ea_be_used_dft_pos]                            = 2,
    [start_margin_pos]                              = 2,
    [test_mode_margin_pos]                          = 4,
    [bw_used_slave_dft_pos]                         = 3,
    [bw_used_adv_dft_pos]                           = 6,
    [rwble_prog_latency_dft_pos]                    = 1,
    [rwble_asap_latency_pos]                        = 2,
    [rwble_priority_adv_ldc_pos]                    = 0,
    [rwble_priority_scan_pos]                       = 0,
    [rwble_priority_mconnect_pos]                   = 3,
    [rwble_priority_sconnect_pos]                   = 3,
    [rwble_priority_adv_hdc_pos]                    = 5,
    [rwble_priority_init_pos]                       = 5,
    [rwble_priority_max_pos]                        = 6,
    [lld_evt_abort_cnt_duration_pos]                = 485,
    [ea_check_halfslot_boundary_pos]                = 624,
    [ea_check_slot_boundary_pos]                    = 614,
    [lld_rx_irq_thres_pos]                          = (BLE_RX_BUFFER_CNT / 2),
    [llm_adv_interval_min_noncon_disc_pos]          = 32,
    [hci_acl_data_packet_num_pos]                   = 8,
    [hci_acl_data_packet_size_pos]                  = 251,
    [hci_lmp_ll_vers_pos]                           = RWBLE_SW_VERSION_MAJOR,
    [hci_lmp_ll_subversion_pos]                     = CO_SUBVERSION_BUILD(RWBLE_SW_VERSION_MINOR, RWBLE_SW_VERSION_BUILD),
#endif /* (BLE_EMB_PRESENT) */

#if (BLE_HOST_PRESENT)
    #if (BLE_APP_PRESENT)
        [app_main_task_pos]                         = TASK_APP,
    #else
        #if (GTL_ITF)
        [app_main_task_pos]                         = TASK_GTL,
        #endif /* (GTL_ITF) */
    #endif /* (BLE_APP_PRESENT) */

    [gap_lecb_cnx_max_pos]                          = 10,
    [gapm_scan_filter_size_pos]                     = 10,
    [smpc_rep_attempts_timer_def_val_pos]           = 200,
    [smpc_rep_attempts_timer_max_val_pos]           = 3000,
    [smpc_rep_attempts_timer_mult_pos]              = 2,
    [smpc_timeout_timer_duration_pos]               = 3000,
    [att_trans_rtx_pos]                             = 0x0BB8,
    [att_sec_enc_key_size_pos]                      = 0x10,
#endif /* (BLE_HOST_PRESENT) */

#if ((BLE_HOST_PRESENT) || (BLE_EMB_PRESENT))
    [nb_links_user_pos]                             = dg_configBLE_CONNECTIONS_MAX,
#endif /* ((BLE_HOST_PRESENT) || (BLE_EMB_PRESENT)) */

#if ((GTL_ITF) || (TL_ITF))
    [max_tl_pending_packets_adv_pos]                = 50,
    [max_tl_pending_packets_pos]                    = 60,
#endif /* ((GTL_ITF) || (TL_ITF)) */
};

uint32_t rom_cfg_table[sizeof(rom_cfg_table_const) / sizeof(uint32_t)] __attribute__((section(".cmi_data_ext_area")));

uint32_t jump_table_init(void)
{
        memcpy((uint8_t *) rom_cfg_table, (uint8_t *) rom_cfg_table_const, sizeof(rom_cfg_table));
        return ((uint32_t) rom_cfg_table);
}


#endif /* CONFIG_USE_BLE */
