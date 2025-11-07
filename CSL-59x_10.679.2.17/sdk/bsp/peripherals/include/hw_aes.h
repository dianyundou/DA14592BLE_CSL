/**
 * \addtogroup PLA_DRI_CRYPTO
 * \{
 * \addtogroup AES
 * \{
 * \brief AES Engine LLD API
 */

/**
 ****************************************************************************************
 *
 * @file hw_aes.h
 *
 * @brief Definition of API for the AES Engine Low Level Driver.
 *
 * Copyright (C) 2023 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef HW_AES_H_
#define HW_AES_H_

#if dg_configUSE_HW_AES

#define HW_AES_USER_DATA_KEYS_MAX_ENTRIES       ((MEMORY_EFLASH_USER_DATA_KEYS_END  - \
                                                MEMORY_EFLASH_USER_DATA_KEYS_BASE) \
                                               / MEMORY_EFLASH_USER_DATA_KEY_SIZE)
#include <stdbool.h>
#include "hw_aes_hash.h"

/**
 * \brief       AES engine error codes
 */
typedef enum {
        HW_AES_ERROR_INVALID_INPUT_DATA_LEN = -2,
        HW_AES_ERROR_CRYPTO_ENGINE_LOCKED = -1,
        HW_AES_ERROR_NONE = 0
} HW_AES_ERROR;

/**
 * \brief       AES Mode
 */
typedef enum {
        HW_AES_MODE_ECB = 0,            /**< AES Mode ECB */
        HW_AES_MODE_CTR = 2,            /**< AES Mode CTR */
        HW_AES_MODE_CBC = 3,            /**< AES Mode CBC */
} HW_AES_MODE;

/**
 * \brief       AES operation
 */
typedef enum {
        HW_AES_OPERATION_DECRYPT = 0,   /**< Perform AES Decryption */
        HW_AES_OPERATION_ENCRYPT = 1    /**< Perform AES Encryption */
} HW_AES_OPERATION;

/**
 * \brief       AES key size
 */
typedef enum {
        HW_AES_KEY_SIZE_128 = 0,        /**< AES Key 128-bit */
        HW_AES_KEY_SIZE_192 = 1,        /**< AES Key 192-bit */
        HW_AES_KEY_SIZE_256 = 2         /**< AES Key 256-bit */
} HW_AES_KEY_SIZE;

/**
 * \brief       AES key expansion modes
 */
typedef enum {
        HW_AES_KEY_EXPAND_BY_SW = 0,    /**< The key expansion is performed by the software */
        HW_AES_KEY_EXPAND_BY_HW = 1     /**< The key expansion is performed by the hardware accelerator */
} HW_AES_KEY_EXPAND;

/**
 * \brief       AES Output Mode
 */
typedef enum {
        HW_AES_OUTPUT_DATA_MODE_ALL = 0,         /**< Write back to the memory all the output data */
        HW_AES_OUTPUT_DATA_MODE_FINAL_BLOCK = 1, /**< Write back to the memory only the final block of the output data */
} HW_AES_OUTPUT_DATA_MODE;

/**
 * \brief       AES engine configuration structure.
 *
 * \note        When executing from XiP Flash the output data address can explicitly reside in SYSRAM
 *              while execution from SYSRAM allows to set this address either in SYSRAM (remapped or
 *              not) or in CACHERAM. Moreover, there are some restrictions in terms of the acceptable
 *              values of the number of the input data to be processed (input_data_len) with regards
 *              to the AES mode (mode) and the input data mode (wait_more_input), indicated by the
 *              next table:
 *
 * mode        | wait_more_input = true | wait_more_input = false |
 * ----------- | ---------------------- | ----------------------- |
 * HW_AES_ECB  | multiple of 16         | multiple of 16          |
 * HW_AES_CBC  | multiple of 16         | no restrictions         |
 * HW_AES_CTR  | multiple of 16         | no restrictions         |
 *
 * \sa          HW_AES_MODE
 * \sa          HW_AES_OPERATION
 * \sa          HW_AES_KEY_SIZE
 * \sa          HW_AES_KEY_EXPAND
 * \sa          HW_AES_OUTPUT_DATA_MODE
 * \sa          hw_aes_hash_cb
 */
typedef struct {
        HW_AES_MODE             mode : 2;               /**< AES mode */
        HW_AES_OPERATION        operation : 1;          /**< AES operation, e.g. encryption or decryption */
        HW_AES_KEY_SIZE         key_size : 2;           /**< AES key size */
        HW_AES_KEY_EXPAND       key_expand : 1;         /**< AES key expansion mode */
        HW_AES_OUTPUT_DATA_MODE output_data_mode : 1;   /**< AES output data mode */
        bool                    wait_more_input;        /**< AES input data mode */
        hw_aes_hash_cb          callback;               /**< AES callback function */
        const uint8_t           *iv_cnt_ptr;            /**< Pointer of the initialization vector in
                                                             CBC mode or the initialization counter
                                                             in CTR Mode */
        uint32_t                keys_addr;              /**< The address of the AES Keys which can
                                                             reside either in OTP or in RAM. In the
                                                             former case use the hw_otpc_get_aes_key_address()
                                                             to get their address. */
        uint32_t                input_data_addr;        /**< AES input data address */
        uint32_t                output_data_addr;       /**< AES output data address */
        uint32_t                input_data_len;         /**< Bytes of input data to be processed */
} hw_aes_config_t;

/**
 * \brief       Set AES Mode.
 *
 * \param [in]  aes_mode AES Mode.
 *
 * \sa          HW_AES_MODE
 */
__STATIC_INLINE void hw_aes_set_mode(HW_AES_MODE aes_mode)
{
        ASSERT_WARNING((aes_mode == HW_AES_MODE_ECB) || (aes_mode == HW_AES_MODE_CTR) ||
                       (aes_mode == HW_AES_MODE_CBC));

        uint32_t crypto_ctrl_reg = AES_HASH->CRYPTO_CTRL_REG;

        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_HASH_SEL, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG, crypto_ctrl_reg, 0);
        REG_SET_FIELD(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD, crypto_ctrl_reg, aes_mode);

        AES_HASH->CRYPTO_CTRL_REG = crypto_ctrl_reg;
}

/**
 * \brief       Get AES Mode.
 *
 * \return      AES mode.
 *
 * \sa          HW_AES_MODE
 */
__STATIC_INLINE HW_AES_MODE hw_aes_get_mode(void)
{
        return REG_GETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ALG_MD);
}

/**
 * \brief       Set AES operation, e.g. encryption or decryption.
 *
 * \param [in]  operation AES operation.
 *
 * \sa          HW_AES_OPERATION
 */
__STATIC_INLINE void hw_aes_set_operation(HW_AES_OPERATION operation)
{
        ASSERT_WARNING((operation == HW_AES_OPERATION_DECRYPT) ||
                       (operation == HW_AES_OPERATION_ENCRYPT));

        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_ENCDEC, operation);
}

/**
 * \brief       Set AES key size.
 *
 * \param [in]  key_size AES key size.
 *
 * \sa          HW_AES_KEY_SIZE
 */
__STATIC_INLINE void hw_aes_set_key_size(HW_AES_KEY_SIZE key_size)
{
        ASSERT_WARNING((key_size == HW_AES_KEY_SIZE_128) || (key_size == HW_AES_KEY_SIZE_192) ||
                       (key_size == HW_AES_KEY_SIZE_256));

        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEY_SZ, key_size);
}

/**
 * \brief       Set AES key expansion mode.
 *
 * \param [in]  key_expand AES key expansion mode.
 *
 * \sa          HW_AES_KEY_EXPAND
 */
__STATIC_INLINE void hw_aes_set_key_expansion(HW_AES_KEY_EXPAND key_expand)
{
        ASSERT_WARNING((key_expand == HW_AES_KEY_EXPAND_BY_SW) ||
                       (key_expand == HW_AES_KEY_EXPAND_BY_HW));

        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_AES_KEXP, key_expand);
}

/**
 * \brief       Set AES engine output data mode.
 *
 * \param [in]  output_data_mode AES output data mode.
 *
 * \sa          HW_AES_OUTPUT_DATA_MODE
 */
__STATIC_INLINE void hw_aes_set_output_data_mode(HW_AES_OUTPUT_DATA_MODE output_data_mode)
{
        ASSERT_WARNING((output_data_mode == HW_AES_OUTPUT_DATA_MODE_ALL) ||
                       (output_data_mode == HW_AES_OUTPUT_DATA_MODE_FINAL_BLOCK));

        REG_SETF(AES_HASH, CRYPTO_CTRL_REG, CRYPTO_OUT_MD, output_data_mode);
}

/**
 * \brief       Start AES operation (Encryption/Decryption).
 *
 * \param [in]  aes_operation AES operation (Encryption or Decryption).
 *
 * \sa          HW_AES_OPERATION
 */
__STATIC_INLINE void hw_aes_start_operation(HW_AES_OPERATION aes_operation)
{
        hw_aes_set_operation(aes_operation);
        hw_aes_hash_start();
}

/**
 * \brief       Check if the restrictions of the input data length are fulfilled.
 *
 * There are some restrictions in terms of the acceptable values of the data_len with regards to the
 * AES mode and the Input Data Mode (wait_more_input). If the rules indicated by the next table are
 * NOT fulfilled the function returns false, otherwise true.
 *
 * mode        | wait_more_input = true | wait_more_input = false |
 * ----------- | ---------------------- | ----------------------- |
 * HW_AES_ECB  | multiple of 16         | multiple of 16          |
 * HW_AES_CBC  | multiple of 16         | no restrictions         |
 * HW_AES_CTR  | multiple of 16         | no restrictions         |
 *
 * \return      True if the restrictions are fulfilled, otherwise false.
 */
bool hw_aes_check_input_data_len_restrictions(void);

/**
 * \brief       Set the Initialization Vector in CBC Mode or the Counter in CTR Mode.
 *
 * \param [in]  iv_cnt_ptr Pointer of the Initialization Vector in CBC Mode or
 *                         the Initialization Counter in CTR Mode.
 *
 * \note        Only applicable to AES CBC/CTR modes.
 */
void hw_aes_set_init_vector(const uint8_t *iv_cnt_ptr);
/**
 * \brief Check whether the Encryption Key is revoked or not.
 *
 * \param [in] idx eFlash memory cell_offset where the key is located
 *
 * \pre  Make sure that eFlash is available for reading
 *
 * \warning The acceptable idx range is from 0 to 15
 *
 * \return true if key is valid, otherwise false
 *
 */
bool hw_aes_is_key_valid(uint8_t idx);


/**
 * \brief Get User Data Encryption Key memory address
 *
 * Returns address of User Data Encryption Key for given key index within the memory that the keys
 * are stored
 *
 * \param [in] key_idx key index
 *
 * \warning The acceptable range for the key_idx is 0 - 7.
 *
 * \return key memory address or 0 if key has been revoked
 *
 */
uint32_t hw_aes_key_address_get(uint8_t key_idx);

/**
 * \brief       Load the AES keys from OTP/RAM to Crypto Engine
 *
 * \param [in]  key_src_addr The address of the AES Keys which can reside either in OTP or in RAM.
 *                           In the former case use the hw_otpc_get_aes_key_address() to get their
 *                           address.
 * \param [in]  key_size     The AES Key size.
 * \param [in]  key_exp      Select whether the keys will be expanded by software or by hardware.
 *
 * \sa          HW_AES_KEY_SIZE
 * \sa          HW_AES_KEY_EXPAND
 *
 */
void hw_aes_load_keys(uint32_t key_src_addr, HW_AES_KEY_SIZE key_size, HW_AES_KEY_EXPAND key_exp);

/**
 * \brief       AES engine initialization function.
 *
 * Configure the AES engine provided that the crypto engine is NOT locked by the HASH engine.
 * If the function returns HW_AES_ERROR_NONE, the operation can be started by calling the
 * hw_aes_start_operation().
 *
 * \param [in]  aes_cfg AES engine configuration structure
 *
 * \return      HW_AES_ERROR_NONE if the AES engine has been successfully initialized,
 *              otherwise an error code.
 *
 * \warning     When AES operation has been completed, the hw_aes_hash_deinit() should be called in
 *              order for the crypto engine to be unlocked from AES. This is mandatory in case that
 *              both AES and HASH are used by the same application. The two blocks make use of the
 *              same hardware accelerator, thus they are mutually exclusive and cannot be used
 *              simultaneously. The functions hw_aes_init(), hw_hash_init() and hw_aes_hash_deinit()
 *              incorporate a mechanism which ensures mutual exclusion and prevents race condition,
 *              provided that the user doesn't call the functions hw_aes_hash_disable_clock(),
 *              hw_aes_hash_enable_clock(), hw_aes_set_mode() and hw_hash_set_type(). The aforementioned
 *              functions affect some AES/HASH register fields which are used by this mechanism and
 *              might violate it. Therefore, it is highly recommended to use the corresponding
 *              init/deinit functions instead.
 *
 * \sa          hw_aes_config_t
 * \sa          HW_AES_ERROR
 */
HW_AES_ERROR hw_aes_init(const hw_aes_config_t *aes_cfg);

#endif /* dg_configUSE_HW_AES */

#endif /* HW_AES_H_ */
/**
 * \}
 * \}
 */
