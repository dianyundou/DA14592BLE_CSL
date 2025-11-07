/**
 \addtogroup MID_INT_BLE_API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_bufops.h
 *
 * @brief Helpers to put and get data from BLE buffers
 *
 * Copyright (C) 2015-2018 Renesas Electronics Corporation and/or its affiliates.
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

#ifndef BLE_BUFOPS_H_
#define BLE_BUFOPS_H_

/**
 * \brief Get uint8 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint8_t get_u8(const uint8_t *buffer)
{
        return buffer[0];
}

/**
 * \brief Get uint16 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint16_t get_u16(const uint8_t *buffer)
{
        return (buffer[0]) | (buffer[1] << 8);
}

/**
 * \brief Get uint32 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint32_t get_u32(const uint8_t *buffer)
{
        return (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

/**
 * \brief Get uint8 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint8_t get_u8_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint8_t);

        return get_u8(b);
}

/**
 * \brief Get uint16 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint16_t get_u16_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint16_t);

        return get_u16(b);
}

/**
 * \brief Get uint32 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
__STATIC_INLINE uint32_t get_u32_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint32_t);

        return get_u32(b);
}

/**
 * \brief Put uint8 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u8(uint8_t *buffer, uint8_t value)
{
        buffer[0] = value;
}

/**
 * \brief Put uint16 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u16(uint8_t *buffer, uint16_t value)
{
        buffer[0] = value;
        buffer[1] = value >> 8;
}

/**
 * \brief Put uint32 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u32(uint8_t *buffer, uint32_t value)
{
        buffer[0] = value;
        buffer[1] = value >> 8;
        buffer[2] = value >> 16;
        buffer[3] = value >> 24;
}

/**
 * \brief Put uint8 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u8_inc(uint8_t **buffer, uint8_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint8_t);

        put_u8(b, value);
}

/**
 * \brief Put uint16 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u16_inc(uint8_t **buffer, uint16_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint16_t);

        put_u16(b, value);
}

/**
 * \brief Put uint32 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
__STATIC_INLINE void put_u32_inc(uint8_t **buffer, uint32_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint32_t);

        put_u32(b, value);
}

/**
 * \brief Put data to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     length length of data to put
 * \param [in]     data   data to put
 *
 */
__STATIC_INLINE void put_data_inc(uint8_t **buffer, uint16_t length, const void *data)
{
        memcpy(*buffer, data, length);

        (*buffer) += length;
}

/**
 * \brief Put string to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     str    string to put
 *
 */
__STATIC_INLINE void put_str_inc(uint8_t **buffer, const char *str)
{
        put_data_inc(buffer, strlen(str), str);
        put_u8_inc(buffer, '\0');
}

#endif /* BLE_BUFOPS_H_ */
/**
 \}
 */
