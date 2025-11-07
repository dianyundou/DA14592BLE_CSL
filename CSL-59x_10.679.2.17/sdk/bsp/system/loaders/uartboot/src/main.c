/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief UART bootloader
 *
 * Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
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

/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup Boot-loader
 * \{
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sdk_defs.h>
#include <hw_gpio.h>
#include <hw_uart.h>
#include <hw_timer.h>
#include <hw_qspi.h>
#include <hw_watchdog.h>
#include <hw_cpm.h>
#include <hw_clk.h>
#include <eflash_automode.h>
#include <ad_flash.h>
#include <ad_nvms.h>
#include <ad_nvms_ves.h>
#include <ad_nvms_direct.h>
#include <flash_partitions.h>
#include "sdk_crc16.h"
#include "uartboot_types.h"
#include "protocol.h"
#if dg_configUSE_SYS_TCS
#include <sys_tcs.h>
#include "../../peripherals/src/hw_sys_internal.h"
#endif

#define BOOTUART (HW_UART2)
#define BOOTUART_STEP                   3

#define glue(a,b) a##b
#define BAUDRATE_CONST(b) glue(HW_UART_BAUDRATE_, b)
#define BAUDRATE_CFG BAUDRATE_CONST(BAUDRATE)


#       define CFG_GPIO_BOOTUART_TX_PORT       HW_GPIO_PORT_0
#       define CFG_GPIO_BOOTUART_TX_PIN        HW_GPIO_PIN_13
#       define CFG_GPIO_BOOTUART_RX_PORT       HW_GPIO_PORT_0
#       define CFG_GPIO_BOOTUART_RX_PIN        HW_GPIO_PIN_15

/* These two values should always be related */
#define VERSION         (0x0003) // BCD
#define VERSION_STR     "0.0.0.3"

#define TMO_COMMAND     (2)
#define TMO_DATA        (5)
#define TMO_ACK         (3)

/*
 * this is 'magic' address which can be used in some commands to indicate some kind of temporary
 * storage, i.e. command needs to store some data but does not care where as long as it can be
 * accessed later
 */
#define ADDRESS_TMP     (0xFFFFFFFF)

/*
 *
 */
#define VIRTUAL_BUF_ADDRESS   (0x80000000)
#define VIRTUAL_BUF_MASK      (0xFFFE0000)

#define IS_EMPTY_CHECK_SIZE     2048

/* Convert GPIO pad (1 byte) to GPIO port/pin */
#define GPIO_PAD_TO_PORT(pad)   (((pad) & 0xE0) >> 5)
#define GPIO_PAD_TO_PIN(pad)    ((pad) & 0x1F)

/* compile-time assertion */
#define C_ASSERT(cond) typedef char __c_assert[(cond) ? 1 : -1] __attribute__((unused))

#define UARTBOOT_LIVE_MARKER            "Live"
#define UNDETERMINED                    "Undetermined"

extern uint8_t __inputbuffer_start; // start of .inputbuffer section
extern uint8_t __inputbuffer_end;
uint32_t input_buffer_size;

#define VERIFY_EFLASH_WRITE 	(1)

/*
 * a complete flow for transmission handling (including in/out data) is as follows:
 *
 * <= <STX> <SOH> (ver1) (ver2)
 * => <SOH>
 * => (type) (len1) (len2)
 * call HOP_INIT
 * <= <ACK> / <NAK>
 * if len > 0
 *      => (data...)
 *      call HOP_DATA
 *      <= <ACK> / <NAK>
 *      <= (crc1) (crc2)
 *      => <ACK> / <NAK>
 * call HOP_EXEC
 * <= <ACK> / <NAK>
 * call HOP_SEND_LEN
 * if len > 0
 *      <= (len1) (len2)
 *      => <ACK> / <NAK>
 *      call HOP_SEND_DATA
 *      <= (data...)
 *      => (crc1) (crc2)
 *      <= <ACK> / <NAK>
 *
 * If NAK has been sent at some step, next steps shouldn't be performed.
 */

/* call type for command handler */
typedef enum {
        HOP_INIT,       // command header is received, i.e. type and length of incoming data
                        //      return false to NAK
        HOP_HEADER,     // full header is received
                        //      return false to NAK
        HOP_DATA,       // command data is received
                        //      return false to NAK
        HOP_EXEC,       // complete command data is received
                        //      return false to NAK
        HOP_SEND_LEN,   // need to send outgoing data length - use xmit_data()
                        //      return false if no data to be sent
        HOP_SEND_DATA,  // called for handler send data back - use xmit_data()
                        //      return false to abort
} HANDLER_OP;

/* UART configuration */
static uart_config UART_INIT = {
                .baud_rate              = HW_UART_BAUDRATE_115200,
                .data                   = HW_UART_DATABITS_8,
                .parity                 = HW_UART_PARITY_NONE,
                .stop                   = HW_UART_STOPBITS_1,
                .auto_flow_control      = 0,
                .use_fifo               = 1,
#if (dg_configUART_DMA_SUPPORT == 1)
                .use_dma                = 0,
                .tx_dma_channel         = HW_DMA_CHANNEL_INVALID,
                .rx_dma_channel         = HW_DMA_CHANNEL_INVALID,
#endif
};

static uint8_t uart_buf[32];                // buffer for incoming data (control data only)

static volatile bool timer1_soh_tmo = true; // timeout waiting for SOH flag

static volatile bool uart_soh = false;      // UART waiting for SOH flag

static bool uart_tmo = false;               // timeout waiting for data from UART

static volatile uint16_t tick = 0;          // 1s tick counter

static volatile uint16_t uart_data_len = 0; // length of data received from UART

/*
 * Valid port and pin values will be set in GPIO watchdog function. Port max and pin max aren't
 * a valid value - they won't be used as GPIO output without later initialization.
 */
static HW_GPIO_PORT gpio_wd_port = HW_GPIO_PORT_MAX;
static HW_GPIO_PIN gpio_wd_pin = HW_GPIO_PIN_MAX;
static uint32_t gpio_wd_timer_cnt;

#if dg_configNVMS_ADAPTER
static bool ad_nvms_init_called = false;    // ad_nvms_init() should be called once and only if needed
#endif


/**
 * \brief Send to RAM command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_send_to_ram {
        uint8_t *ptr;                   /**< Pointer to RAM where data will be written */
};

 /**
  * \brief Read from RAM command's parameters
  *
  */
__PACKED_STRUCT cmdhdr_read_from_ram {
        uint8_t *ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Read data length in bytes */
};

/**
 * \brief Write RAM to QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_ram_to_qspi {
        uint8_t *ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Data length in bytes */
        uint32_t addr;                  /**< QSPI FLASH address where data will be written */
};

/**
 * \brief Erase QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_erase_qspi {
        uint32_t addr;                  /**< QSPI FLASH erase start address */
        uint32_t len;                   /**< Erase size in bytes */
};

/**
 * \brief Execute code command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_execute_code {
        uint32_t addr;                  /**< Address of the function to call */
};

/**
 * \brief Write OTP command's parameters
 *
 * \note OTP cell size is 64-bits for DA1468x and 32-bits for DA1469x.
 *
 */
__PACKED_STRUCT cmdhdr_write_otp {
        uint32_t addr;                  /**< OTP cell offset */
};

/**
 * \brief Read OTP command's parameters
 *
 * \note OTP cell size is 64-bits for DA1468x and 32-bits for DA1469x.
 *
 */
__PACKED_STRUCT cmdhdr_read_otp {
        uint32_t addr;                  /**< OTP cell offset */
        uint16_t len;                   /**< Number of 32-bits words */
};

/**
 * \brief Read QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_qspi {
        uint32_t addr;                  /**< Address in QSPI FLASH */
        uint16_t len;                   /**< Read size in bytes */
};

#if dg_configNVMS_ADAPTER
/**
 * \brief Read partition command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_partition {
        uint32_t addr;                  /**< Offset from the partition's beginning */
        uint16_t len;                   /**< Read size in bytes */
        nvms_partition_id_t id;         /**< Partition ID */
};

/**
 * \brief Write partition command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_partition {
        uint8_t *ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Write size in bytes */
        uint32_t addr;                  /**< Offset from the partition's beginning */
        nvms_partition_id_t id;         /**< Partition ID */
};
#endif

/**
 * \brief Get uartboot version command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_get_version {
};

/**
 * \brief Is empty QSPI command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_is_empty_qspi {
        uint32_t size;                  /**< Check size in bytes */
        uint32_t start_address;         /**< QSPI FLASH check start address */
};

/**
 * \brief Get QSPI state command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_get_qspi_state {
        uint8_t id;                     /**< QSPI controller ID */
};

/**
 * \brief Direct write to QSPI command's parameters
 *
 */
__PACKED_STRUCT  cmdhdr_direct_write_qspi {
        uint8_t read_back_verify;       /**< Verify written data (value other than 0 ) */
        uint32_t addr;                  /**< QSPI FLASH address where data will be written */
};

/**
 * \brief Change UART's baudrate command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_change_baudrate {
        uint32_t baudrate;              /**< New UART baudrate */
};

/**
 * \brief GPIO external watchdog notification command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_gpio_wd {
        uint8_t gpio_pad;               /**< Encoded GPIO port and pin */
        uint8_t gpio_lvl;               /**< GPIO power source */
};

/**
 * \brief Read eFLASH command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_read_eflash {
        uint32_t addr;                  /**< Address in eFLASH */
        uint16_t len;                   /**< Read size in bytes */
};

/**
 * \brief Direct write to eFLASH command's parameters
 *
 */
__PACKED_STRUCT  cmdhdr_direct_write_eflash {
        uint8_t read_back_verify;       /**< Verify written data (value other than 0 ) */
        uint32_t addr;                  /**< eFLASH address where data will be written */
};

/**
 * \brief Write RAM to eFLASH command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_write_ram_to_eflash {
        uint8_t *ptr;                   /**< Pointer to RAM from where data will be read */
        uint16_t len;                   /**< Data length in bytes */
        uint32_t addr;                  /**< eFLASH address where data will be written */
};

/**
 * \brief Erase eFLASH command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_erase_eflash {
        uint32_t addr;                  /**< eFLASH erase start address */
        uint32_t len;                   /**< Erase size in bytes */
};

/**
 * \brief Is empty eFLASH command's parameters
 *
 */
__PACKED_STRUCT cmdhdr_is_empty_eflash {
        uint32_t size;                  /**< Check size in bytes */
        uint32_t start_address;         /**< eFLASH check start address */
};

/*
 * union of all cmdhdr structures, this is used to create buffer to which command header will be
 * loaded so we can safely use payload buffer to keep data between commands
 */
union cmdhdr {
        struct cmdhdr_send_to_ram send_to_ram;
        struct cmdhdr_read_from_ram read_from_ram;
        struct cmdhdr_write_ram_to_qspi write_ram_to_qspi;
        struct cmdhdr_erase_qspi erase_qspi;
        struct cmdhdr_execute_code execute_code;
        struct cmdhdr_write_otp write_otp;
        struct cmdhdr_read_otp read_otp;
        struct cmdhdr_read_qspi read_qspi;
#if dg_configNVMS_ADAPTER
        struct cmdhdr_read_partition read_partition;
        struct cmdhdr_write_partition write_partition;
#endif
        struct cmdhdr_get_version get_version;
        struct cmdhdr_is_empty_qspi is_empty_qspi;
        struct cmdhdr_get_qspi_state get_qspi_state;
        struct cmdhdr_direct_write_qspi direct_write_qspi;
        struct cmdhdr_change_baudrate change_baudrate;
        struct cmdhdr_gpio_wd gpio_wd;
        struct cmdhdr_read_eflash read_eflash;
        struct cmdhdr_direct_write_eflash direct_write_eflash;
        struct cmdhdr_write_ram_to_eflash write_ram_to_eflash;
        struct cmdhdr_erase_eflash erase_eflash;
        struct cmdhdr_is_empty_eflash is_empty_eflash;
};

/* state of incoming command handler */
static struct cmd_state {
        uint8_t type;                           // type of command being handled
        uint16_t len;                           // command length (header and payload)
        union cmdhdr hdr;                       // command header
        uint16_t hdr_len;                       // command header length
        uint8_t *data;                          // command payload
        uint16_t data_len;                      // command payload length
        bool (* handler) (HANDLER_OP);          // command handler;

        uint16_t crc;                           // CRC of transmitted data;
} cmd_state;

typedef struct {
        char magic[4];
        volatile uint32_t run_swd;     /* This is set to 1 by debugger to enter SWD mode */
        volatile uint32_t cmd_num;     /* Debugger command sequence number, this field is
                                          incremented by debugger after arguments in uart_buf have
                                          been set for new command. Bootloader starts interpreting
                                          command when this number changes. This will prevent
                                          executing same command twice by accident */
        uint8_t *cmd_hdr_buf;          /* buffer for header stored here for debugger to see */
        uint8_t *buf;                  /* Big buffer for data transfer */
        volatile uint32_t ack_nak;     /* ACK or NAK for swd command */
} swd_interface_t;

const swd_interface_t swd_interface __attribute__((section (".swd_section"))) = {
        "DBGP", /* This marker is for debugger to search for swd_interface structure in memory */
        0,
        0,
        uart_buf,
        &__inputbuffer_start
};

/**
 * \brief Translate 'magic' addresses into actual memory location
 *
 * \param [in,out] addr memory address
 *
 */
__STATIC_INLINE void translate_ram_addr(uint32_t *addr)
{
        /*
         * ADDRESS_TMP will point to inputbuffer which is large enough to hold all received data
         * and it's not necessary to move data around since they are already received into this
         * buffer
         */
        if (*addr == ADDRESS_TMP) {
                *addr = (uint32_t) &__inputbuffer_start;
        } else if ((*addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS) {
                *addr = (*addr & ~VIRTUAL_BUF_MASK) + (uint32_t) &__inputbuffer_start;
        }
}

/**
 * \brief Check that given RAM address is in valid range
 *
 * This function handles 'magic' address to input buffer, therefore it should be called before
 * translate_ram_addr function.
 *
 * \param [in] addr memory address
 * \param [in] size requested size
 *
 * \return false if the address + size exceeds the temporary buffer (if address with
 *         VIRTUAL_BUF_MASK/ADDRESS_TMP passed), true otherwise
 *
 *  \sa translate_ram_addr
 *
 */
static bool check_ram_addr(uint32_t addr, uint32_t size)
{
        if (addr == ADDRESS_TMP) {
                addr = ((uint32_t) &__inputbuffer_start);
        } else if ((addr & VIRTUAL_BUF_MASK) == VIRTUAL_BUF_ADDRESS) {
                addr = (addr & ~VIRTUAL_BUF_MASK) + (uint32_t) &__inputbuffer_start;
        } else {
                /* Raw address - can be a SysRAM, CacheRam or register, do nothing */
                return true;
        }

        return (addr + size) <= (uint32_t) (&__inputbuffer_end);
}

static void timer1_soh_cb(void)
{
        hw_uart_abort_receive(BOOTUART);
        timer1_soh_tmo = true;
}

static void uart_soh_cb(uint8_t *data, uint16_t len)
{
        if (len == 1 && data[0] == SOH) {
                uart_soh = true;
        }
}

static void timer1_tick_cb(void)
{
        tick++;
}

static void timer_gpio_wd_cb(void)
{
        if (gpio_wd_timer_cnt == 0) {
                hw_gpio_set_active(gpio_wd_port, gpio_wd_pin);
        } else {
                hw_gpio_set_inactive(gpio_wd_port, gpio_wd_pin);
        }

        /* 15ms high, 2s low. Callback is called every 15ms by timer. 2000 / 15 = 133.33 */
        gpio_wd_timer_cnt = (gpio_wd_timer_cnt + 1) % 134;
}

static void uart_data_cb(void *user_data, uint16_t len)
{
        uart_data_len = len;
}

__STATIC_INLINE void xmit_hello(void)
{
        static const uint8_t msg[] = {
                        STX, SOH,
                        (VERSION & 0xFF00) >> 8, VERSION & 0x00FF };

        hw_uart_send(BOOTUART, msg, sizeof(msg), NULL, NULL);
}

__STATIC_INLINE void set_ack_nak_field(char sign)
{
        if (swd_interface.run_swd) {
                *((uint32_t *) &swd_interface.ack_nak) = sign;
        }
}

__STATIC_INLINE void xmit_ack(void)
{
        if (swd_interface.run_swd) {
                set_ack_nak_field(ACK);
                return;
        }

        hw_uart_write(BOOTUART, ACK);
}

__STATIC_INLINE void xmit_nak(void)
{
        if (swd_interface.run_swd) {
                set_ack_nak_field(NAK);
                return;
        }

        hw_uart_write(BOOTUART, NAK);
}

__STATIC_INLINE void xmit_crc16(uint16_t crc16)
{
        hw_uart_send(BOOTUART, (void *) &crc16, sizeof(crc16), NULL, NULL);
}

__STATIC_INLINE void xmit_data(const uint8_t *buf, uint16_t len)
{
        uint8_t byt;
        uint16_t i;

        for (i = 0; i < len; ++i) {
                byt = buf[i];

                hw_uart_write(BOOTUART, byt);
                crc16_update(&cmd_state.crc, &byt, 1);
        }
}

static bool recv_with_tmo(uint8_t *buf, uint16_t len, uint16_t tmo)
{
        if (!len) {
                return true;
        }

        tick = 0;
        uart_data_len = 0;
        uart_tmo = false;

        hw_timer_register_int(HW_TIMER, timer1_tick_cb);
        hw_timer_enable(HW_TIMER);
        hw_timer_enable_clk(HW_TIMER);

        hw_uart_receive(BOOTUART, buf, len, uart_data_cb, NULL);

        while (tick < tmo && uart_data_len == 0) {
                __WFI();
        }

        hw_timer_disable(HW_TIMER);

        /* abort if no data received */
        if (uart_data_len == 0) {
                uart_tmo = true;
                hw_uart_abort_receive(BOOTUART);
        }

        return !uart_tmo;
}

#if dg_configNVMS_ADAPTER
static uint16_t push_partition_entry_name(uint8_t *ram, nvms_partition_id_t id)
{
#define _STR_(token) #token
#define _PUSH_ENUM_AS_STRING_2_RAM_(_enum_)      \
        do {                                     \
                len = strlen(_STR_(_enum_)) + 1; \
                memcpy(ram, _STR_(_enum_), len); \
        } while (0);
#define _ALIGN32_(size) (((size) + 3) & (~0x3))

        uint16_t len;

        switch (id) {
        case NVMS_FIRMWARE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FIRMWARE_PART);
                break;

        case NVMS_PARAM_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARAM_PART);
                break;

        case NVMS_BIN_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_BIN_PART);
                break;

        case NVMS_LOG_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_LOG_PART);
                break;

        case NVMS_GENERIC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_GENERIC_PART);
                break;

        case NVMS_PLATFORM_PARAMS_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PLATFORM_PARAMS_PART);
                break;

        case NVMS_PARTITION_TABLE:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PARTITION_TABLE);
                break;

        case NVMS_FW_EXEC_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_EXEC_PART);
                break;

        case NVMS_FW_UPDATE_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_FW_UPDATE_PART);
                break;

        case NVMS_PRODUCT_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_PRODUCT_HEADER_PART);
                break;

        case NVMS_IMAGE_HEADER_PART:
                _PUSH_ENUM_AS_STRING_2_RAM_(NVMS_IMAGE_HEADER_PART);
                break;

        default:
                _PUSH_ENUM_AS_STRING_2_RAM_(UNKNOWN_PARTITION_ID);
        }

        /* len should be multiple of 4 to avoid unaligned loads/stores */
        return _ALIGN32_(len);
#undef _STR_
#undef _PUSH_ENUM_AS_STRING_2_RAM_
#undef _ALIGN32_
}

static uint16_t piggy_back_partition_entry(uint8_t *ram, const partition_entry_t *flash_entry)
{
        cmd_partition_entry_t *ram_entry = (cmd_partition_entry_t *)(ram);
        uint8_t *ram_str = &(ram_entry->name.str);
        ram_entry->start_address = flash_entry->start_address;
        ram_entry->size = flash_entry->size;
        ram_entry->sector_size = AD_FLASH_GET_SECTOR_SIZE(flash_entry->start_address);
        ram_entry->type = flash_entry->type;
        ram_entry->name.len = push_partition_entry_name((ram_str) , flash_entry->type);
        return sizeof(cmd_partition_entry_t) + ram_entry->name.len;
}

static bool piggy_back_partition_table(uint8_t *ram)
{
        uint16_t entry_size = 0;
        cmd_partition_table_t *ram_table = (cmd_partition_table_t *)ram;
        cmd_partition_entry_t *ram_entry = &(ram_table->entry);
        partition_entry_t flash_entry;
        uint32_t flash_addr = PARTITION_TABLE_ADDR;
        ram_table->len = 0;

#ifdef PARTITION_TABLE_ADDR_ALT
        /* Check if a partition table does not exist in the primary location and use alternative
         * partition table location */
        ad_flash_read(flash_addr, (uint8_t *)&flash_entry, sizeof(partition_entry_t));
        if (flash_entry.type == 0xFF) {
                flash_addr = PARTITION_TABLE_ADDR_ALT;
        } else {
                /* Check for asymmetric SUOTA image and use alternative partition table location */
                uint8_t magic[sizeof(dg_configASYMMETRIC_SUOTA_MAGIC) - 1] = { 0 };
                ad_flash_read(EFLASH_MEM1_VIRTUAL_BASE_ADDR + 0x30000 + 0x400 + 0x200, magic, sizeof(magic));
                if (!memcmp(magic, dg_configASYMMETRIC_SUOTA_MAGIC, sizeof(magic))) {
                        flash_addr = PARTITION_TABLE_ADDR_ALT;
                }
        }
#endif
        do {
                ad_flash_read(flash_addr, (uint8_t *)&flash_entry, sizeof(partition_entry_t));
                if (flash_entry.type != 0xFF && flash_entry.type != 0 && flash_entry.magic == 0xEA &&
                                flash_entry.valid == 0xFF) {
                        entry_size = piggy_back_partition_entry((uint8_t *)ram_entry, &flash_entry);

                        ram_entry = (cmd_partition_entry_t *)((uint8_t *)ram_entry + entry_size);
                        ram_table->len += entry_size;
                }

                flash_addr += sizeof(partition_entry_t);
        } while (flash_entry.type != 0xFF);
        ram_table->len += sizeof(cmd_partition_table_t);

        return true;
}
#endif

#if dg_configFLASH_ADAPTER
static size_t safe_flash_write(uint32_t flash_addr, const uint8_t *buf, size_t length)
{
        size_t written = 0;
        uint32_t sector_size = AD_FLASH_GET_SECTOR_SIZE(flash_addr);

        while (written < length) {
                uint32_t sector_start = flash_addr & ~(sector_size - 1);
                uint32_t sector_offset = flash_addr - sector_start;
                uint32_t chunk_size = sector_size - sector_offset;
                int off;

                if (chunk_size > length - written) {
                        chunk_size = length - written;
                }

                off = ad_flash_update_possible(flash_addr, buf, chunk_size);

                /* No write needed in this sector, same data */
                if (off == (int) chunk_size) {
                        goto advance;
                }

                /* Write without erase possible */
                if (off >= 0) {
                        ad_flash_write(flash_addr + off, buf + off, chunk_size - off);
                        goto advance;
                }

                /* If entire sector is to be written, no need to read old data */
                if (flash_addr == sector_start && chunk_size == sector_size) {
                        ad_flash_erase_region(flash_addr, sector_size);
                        ad_flash_write(flash_addr, buf, sector_size);
                } else {
                        uint8_t array[AD_FLASH_MAX_SECTOR_SIZE];

                        ad_flash_read(sector_start, array, sector_size);

                        /* Overwrite old data with new one */
                        memcpy(array + sector_offset, buf, chunk_size);

                        /* Erase and write entire sector */
                        ad_flash_erase_region(sector_start, sector_size);
                        ad_flash_write(sector_start, array, sector_size);
                }
advance:
                written += chunk_size;
                buf += chunk_size;
                flash_addr += chunk_size;
        }

        return written;
}

static bool flash_content_cmp(uint32_t flash_addr, size_t length, uint8_t *read_buf, const uint8_t *buf)
{
        /* Read back data and verify */
        if (ad_flash_read(flash_addr, read_buf, length) != length) {
                return false;
        }

        return memcmp(buf, read_buf, length) == 0;
}

/*
 * Wrapper for writing from RAM buffer to eFLASH memory. Verification of written data is performed
 * only if 'read_buf' pointer is not NULL.
 */
static bool __eflash_write(uint32_t flash_addr, const uint8_t *ram_ptr, size_t length, uint8_t *read_buf)
{

        flash_addr += EFLASH_MEM1_VIRTUAL_BASE_ADDR;

        /* Check whole write area at once */
        if (!eflash_automode_is_valid_virtual_address_range(flash_addr, length)) {
                return false;
        }

        if (safe_flash_write(flash_addr, ram_ptr, length) != length) {
                return false;
        }

        return read_buf ? flash_content_cmp(flash_addr, length, read_buf, ram_ptr) : true;
}
#endif /* dg_configFLASH_ADAPTER */

#if (dg_configUSE_SYS_TCS == 1)
static bool prod_info_print_to_buffer(cmd_product_info_t *product_info, const char* format, ...)
{
        int16_t info_size;
        va_list argptr;

        va_start(argptr, format);
        info_size = vsnprintf(&product_info->str + product_info->len,
                                input_buffer_size - product_info->len - sizeof(product_info->len),
                                format, argptr);
        va_end(argptr);

        if (info_size < 0) {
                return false;
        }

        product_info->len += info_size;

        return true;
}

static bool product_info_helper(uint8_t *info)
{
        cmd_product_info_t *product_info = (cmd_product_info_t *)info;
        const char *res;

        product_info->len = 0;

        /* Retrieve and compose device classification attributes */

        if (false == prod_info_print_to_buffer(product_info,
                                "PRODUCT INFORMATION:\nDevice classification attributes:\n")) {
                return false;
        }

        res = UNDETERMINED;

        if (hw_sys_device_info_check(DEVICE_FAMILY_MASK, DA1459X)) {
               res = "DA1459x";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device family = %s\n", res)) {
                return false;
        }

        res = UNDETERMINED;

        if (hw_sys_device_info_check(DEVICE_CHIP_ID_MASK, DEVICE_CHIP_ID_2634)) {
                res = "D2634";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device chip ID = %s\n", res)) {
                return false;
        }

        res = UNDETERMINED;

        if (hw_sys_device_info_check(DEVICE_VARIANT_MASK, DA14592)) {
                res = "DA14592";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "Device variant = %s\n", res)) {
                return false;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Device version (revision|SWC) = ")) {
                return false;
        }

        res = UNDETERMINED;

        if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_A)) {
                res = "A";
        }
        else if (hw_sys_device_info_check(DEVICE_REVISION_MASK, DEVICE_REV_B)) {
                res = "B";
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "%s", res)) {
                return false;
        }

        res = UNDETERMINED;
        if (hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_0)) {
                res = "0";
        }
        else if (hw_sys_device_info_check(DEVICE_SWC_MASK, DEVICE_SWC_1)) {
                res = "1";
        }
        if (false == prod_info_print_to_buffer(product_info,
                                "%s\n\n", res)) {
                return false;
        }

        /* Retrieve production information attributes as stored in the corresponding TCS group */
        #define TCS_PROD_INFO_LEN       2
        uint32_t *values = NULL;
        uint32_t production_info[TCS_PROD_INFO_LEN] = {0};
        uint8_t size = 0;

        sys_tcs_get_custom_values(SYS_TCS_GROUP_PROD_INFO, &values, &size);

        /* Check that the corresponding TCS group returned the expected number of entries */
        if (size != TCS_PROD_INFO_LEN) {
                return false;
        }

        memcpy(production_info, values, sizeof(production_info));

        /* Extract the production package coding that is stored in Byte 7 of the TCS group */
        uint8_t production_package_raw = (production_info[1] >> 24) & 0xFF;

        if (false == prod_info_print_to_buffer(product_info,
                                "Production layout information:\n")) {
                return false;
        }

        switch (production_package_raw) {
        case 0x00:
                res = "WLCSP39";
                break;
        case 0x55:
                res = "FCQFN52";
                break;
        default:
                res = UNDETERMINED;
                break;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Package = %s\n", res)) {
                return false;
        }

        if (false == prod_info_print_to_buffer(product_info,
                                "Production testing information:\nTimestamp = 0x%08lX\n",
                production_info[0])) {
                return false;
        }


        /* Add to the string length one byte for the '\0' character */
        product_info->len = product_info->len + 1;

        product_info->len += sizeof(product_info->len);
        #undef TCS_PROD_INFO_LEN

        return true;
}
#endif /* dg_configUSE_SYS_TCS */

#if dg_configFLASH_ADAPTER
/*
 * Wrapper for writing from RAM buffer to FLASH memory. Verification of written data is performed
 * only if 'read_buf' pointer is not NULL.
 */
static bool __qspi_write(uint32_t flash_addr, const uint8_t *ram_ptr, size_t length, uint8_t *read_buf)
{
#if dg_configQSPI_AUTOMODE_ENABLE

        // Convert the zero based address to virtual address
        flash_addr += QSPI_MEM1_VIRTUAL_BASE_ADDR;

        if (!qspi_automode_is_valid_virtual_address_range(flash_addr, length)) {
                /* Data will exceed the QSPI FLASH memory size - don't write the data. */
                return false;
        }

        if (qspi_automode_is_ram(flash_addr)) {
                /* We can write data directly. No need to check if write is possible */
                if (ad_flash_write(flash_addr, ram_ptr, length) != length) {
                        return false;
                }
        } else {
                if (safe_flash_write(flash_addr, ram_ptr, length) != length) {
                        return false;
                }
        }

        return read_buf ? flash_content_cmp(flash_addr, length, read_buf, ram_ptr) : true;
#else /* dg_configQSPI_AUTOMODE_ENABLE */
        return false;
#endif /* dg_configQSPI_AUTOMODE_ENABLE */
}
#endif /* dg_configFLASH_ADAPTER */


/* handler for 'send data to RAM' */
static bool cmd_send_to_ram(HANDLER_OP hop)
{
        struct cmdhdr_send_to_ram *hdr = &cmd_state.hdr.send_to_ram;

        switch (hop) {
        case HOP_INIT:
                /* some payload is required, otherwise there's nothing to write */
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                /*
                 * When data is written to RAM there is no need to store it in buffer
                 * and then copy to destination. Change address of data from buffer
                 * which is preset to what command wants;
                 */
                cmd_state.data = hdr->ptr;
                /*
                 * When address is explicitly set to ADDRESS_TMP or lays in range
                 * assigned for buffer, convert it to real address in RAM.
                 * hdr->ptr is not modified since it is needed for CRC calculation.
                 */
                if (!check_ram_addr((uint32_t) cmd_state.data, cmd_state.data_len)) {
                        return false;
                }

                translate_ram_addr((uint32_t *) &cmd_state.data);
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Data was already put in correct place */

                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

#ifndef UARTBOOT_MINIMAL
/* handler for 'read memory region from device' */
static bool cmd_read_from_ram(HANDLER_OP hop)
{
        struct cmdhdr_read_from_ram *hdr = &cmd_state.hdr.read_from_ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len)) {
                        return false;
                }

                translate_ram_addr((uint32_t *) &hdr->ptr);
                xmit_data(hdr->ptr, hdr->len);
                return true;
        }
        return false;
}
#endif /* UARTBOOT_MINIMAL */

#if dg_configFLASH_ADAPTER
/* handler for 'write RAM region to QSPI' */
static bool cmd_write_ram_to_qspi(HANDLER_OP hop)
{
        struct cmdhdr_write_ram_to_qspi *hdr = &cmd_state.hdr.write_ram_to_qspi;
        uint32_t read_buf_addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len)) {
                        return false;
                }

                /* check for 'magic' address */
                translate_ram_addr((uint32_t *) &hdr->ptr);
                return true;

        case HOP_EXEC:
#if dg_configVERIFY_QSPI_WRITE
                /* Read buffer is placed after write buffer and has the same length */
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len * 2)) {
                        return false;
                }

                if (((uint32_t) hdr->ptr) >= ((uint32_t) &__inputbuffer_start) &&
                                ((uint32_t) hdr->ptr) <= ((uint32_t) &__inputbuffer_end)) {
                        read_buf_addr = ((uint32_t) hdr->ptr) + hdr->len;       // move after written data
                } else {
                        /* Write is not performed from data buffer - used it for verification */
                        read_buf_addr = ADDRESS_TMP;
                        translate_ram_addr(&read_buf_addr);
                }

#else
                read_buf_addr = 0;
#endif /* dg_configVERIFY_QSPI_WRITE */

                return __qspi_write(hdr->addr, hdr->ptr, hdr->len, (uint8_t *) read_buf_addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'erase region of flash' */
static bool cmd_erase_qspi(HANDLER_OP hop)
{
        struct cmdhdr_erase_qspi *hdr = &cmd_state.hdr.erase_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return hdr->len > 0;

        case HOP_EXEC:
                addr = QSPI_MEM1_VIRTUAL_BASE_ADDR + hdr->addr;

                return ad_flash_erase_region(addr, hdr->len);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}
#endif /* dg_configFLASH_ADAPTER */

void __attribute__((section("reboot_section"), noinline)) move_to_0_and_boot(void *start, size_t size)
{
        uint32_t *src = start;
        uint32_t *dst = NULL;
        int s = (int) ((size + 4) >> 2);
        int i;

        /* Perform a deinitialization */
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);
        hw_clk_dblr_sys_off();
#ifdef UARTBOOT_MINIMAL
        hw_sys_disable_cmac_cache_ram();
#endif

        /*
         * Disable interrupts to prevent calling handlers which will be replaced with new
         * application code. Also do not restore them (will be restored during reset procedure)
         * because pending interrupts will be handled immediately and will corrupt image with
         * stack data.
         */
        __disable_irq();

        for (i = 0; i < s; ++i) {
                dst[i] = src[i];
        }

        REG_SET_BIT(CRG_TOP, SYS_CTRL_REG, SW_RESET);

        /*
         * Wait for reset in the infinite loop (this part of code shouldn't be reached due to
         * the triggered SW reset).
         */
        while (1);
}

/* handler for 'execute code on device' */
static bool cmd_execute_code(HANDLER_OP hop)
{
        struct cmdhdr_execute_code *hdr = &cmd_state.hdr.execute_code;
        void (* func) (void);

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;
        case HOP_EXEC:
                if (!check_ram_addr(hdr->addr, 1)) {
                        return false;
                }

                /* 'xmit_ack' should be used here - function could not reach the end */
                xmit_ack();
                translate_ram_addr((uint32_t *) &hdr->addr);
                /* make sure lsb is 1 (thumb mode) */
                func = (void *) (hdr->addr | 1);
                if ((uint32_t) func == ((uint32_t)&__inputbuffer_start) + 1) {
                        move_to_0_and_boot(&__inputbuffer_start,
                                                        &__inputbuffer_end - &__inputbuffer_start);
                } else {
                        func();
                }
                return true; // we actually should never reach this

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

#ifndef UARTBOOT_MINIMAL
/* handler for 'write to OTP' */
static bool cmd_write_otp(HANDLER_OP hop)
{
        struct cmdhdr_write_otp *hdr = &cmd_state.hdr.write_otp;

        switch (hop) {
        case HOP_INIT:
                /* make sure data to be written length is multiply of word size (4 bytes) */
                return (cmd_state.data_len > 0) && ((cmd_state.data_len & 0x03) == 0);

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* make sure cell address is valid */
                return true;

        case HOP_EXEC:
                return __eflash_write((hdr->addr << 2), cmd_state.data, cmd_state.data_len, NULL);
        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }
        return false;
}

/* handler for 'read from OTP' */
static bool cmd_read_otp(HANDLER_OP hop)
{
        struct cmdhdr_read_otp *hdr = &cmd_state.hdr.read_otp;
        static uint16_t size;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                size = hdr->len * sizeof(uint32_t);
                return true;

        case HOP_EXEC:
                return (ad_flash_read((hdr->addr << 2) + EFLASH_MEM1_VIRTUAL_BASE_ADDR, cmd_state.data,
                                                                                size) == size);

        case HOP_SEND_LEN:
                xmit_data((void *) &size, sizeof(size));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, size);
                return true;
        }
        return false;
}
#endif /* UARTBOOT_MINIMAL */

__PACKED_STRUCT qspi_status {
        uint8_t driver_configured;
        uint8_t manufacturer_id;
        uint8_t device_type;
        uint8_t density;
};

#if dg_configFLASH_ADAPTER
static bool get_qspi_state(uint8_t id, uint16_t *len, uint8_t *buf)
{
#if dg_configQSPI_AUTOMODE_ENABLE
        struct qspi_status *qspi_status = (struct qspi_status *) buf;

        jedec_id_t jedec;
        QSPI_AUTOMODE_MEMORY_STATUS ret;

        ret = qspi_automode_read_jedec_id(QSPI_MEM1_VIRTUAL_BASE_ADDR, &jedec);
        switch (ret) {
        case QSPI_AUTOMODE_MEMORY_STATUS_ABSENT:
                (*len) = sizeof(*qspi_status);

                qspi_status->driver_configured = false;
                qspi_status->manufacturer_id = 0;
                qspi_status->device_type = 0;
                qspi_status->density = 0;
                break;
        case QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_UNIDENTIFIED:
                (*len) = sizeof(*qspi_status);

                qspi_status->driver_configured = false;
                qspi_status->manufacturer_id = jedec.manufacturer_id;
                qspi_status->device_type = jedec.type;
                qspi_status->density = jedec.density;
                break;
        case QSPI_AUTOMODE_MEMORY_STATUS_PRESENT_IDENTIFIED:
                (*len) = sizeof(*qspi_status);

                qspi_status->driver_configured = true;
                qspi_status->manufacturer_id = jedec.manufacturer_id;
                qspi_status->device_type = jedec.type;
                qspi_status->density = jedec.density;

                break;
        }

        return true;
#else /* dg_configQSPI_AUTOMODE_ENABLE */
        return false;
#endif /* dg_configQSPI_AUTOMODE_ENABLE */
}

/* handler for 'init_qspi' */
static bool cmd_get_qspi_state(HANDLER_OP hop)
{
        struct cmdhdr_get_qspi_state *hdr = &cmd_state.hdr.get_qspi_state;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                /* nothing to do */
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                return get_qspi_state(hdr->id, &cmd_state.data_len, cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data((void *) &cmd_state.data_len, sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                if (!cmd_state.data_len) {
                        return false;
                }

                xmit_data((void*) cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}
#endif /* dg_configFLASH_ADAPTER */

/* Handler for 'gpio_wd'. */
static bool cmd_gpio_wd(HANDLER_OP hop)
{
        struct cmdhdr_gpio_wd *hdr = &cmd_state.hdr.gpio_wd;
        static HW_GPIO_PORT port;
        static HW_GPIO_PIN pin;
        static uint8_t volt_rail;

        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_EXT,
                .prescaler = 0x1f, // 32MHz / (31 + 1) = 1MHz
                .mode = HW_TIMER_MODE_TIMER,
                .timer = {
                        .direction = HW_TIMER_DIR_UP,
                        .reload_val = 15000, // interrupt every 15ms
                },
        };

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* Transform the input. */
                port = GPIO_PAD_TO_PORT(hdr->gpio_pad);
                pin = GPIO_PAD_TO_PIN(hdr->gpio_pad);
                volt_rail = hdr->gpio_lvl;

                /* Validate the GPIO port */
                if (port < HW_GPIO_PORT_0 || port >= HW_GPIO_PORT_MAX) {
                        return false;
                }

                /* Validate the GPIO pin */
                if (pin < HW_GPIO_PIN_0 || pin >= hw_gpio_port_num_pins[port]) {
                        return false;
                }

                /* Validate the GPIO voltage rail. 0 = 3.3V, 1 = 1.8V. */
                if (volt_rail > 1) {
                        return false;
                }

                return true;

        case HOP_EXEC:
                /* Disable timer here - avoid timer callback calling */
                hw_timer_disable(HW_TIMER2);

                /* Update global variable */
                gpio_wd_port = port;
                gpio_wd_pin = pin;

                hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);

                /* Reset counter used in callback. Initialize and start timer. */
                gpio_wd_timer_cnt = 0;
                hw_timer_init(HW_TIMER2, &timer_cfg);
                hw_timer_register_int(HW_TIMER2, timer_gpio_wd_cb);
                hw_timer_enable(HW_TIMER2);

                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

#if dg_configFLASH_ADAPTER
/* handler for 'read QSPI' */
static bool cmd_read_qspi(HANDLER_OP hop)
{
        struct cmdhdr_read_qspi *hdr = &cmd_state.hdr.read_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                addr = QSPI_MEM1_VIRTUAL_BASE_ADDR + hdr->addr;

                return ad_flash_read(addr, cmd_state.data, hdr->len) == hdr->len;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}
#endif /* dg_configFLASH_ADAPTER */

/* handler for 'get_version on device' */
static bool cmd_get_version(HANDLER_OP hop)
{
        /* Send without the last character '\0' */
        const uint16_t msg_len = sizeof(VERSION_STR) - 1;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* nothing to do */
                return true;

        case HOP_EXEC:
                /* nothing to do */
                return true;

        case HOP_SEND_LEN:
                /* send length */
                xmit_data((uint8_t *) &msg_len , sizeof(msg_len));
                return true;

        case HOP_SEND_DATA:
                /* send data */
                xmit_data((const uint8_t *) VERSION_STR, msg_len);
                return true;
        }

        return false;
}

#if dg_configFLASH_ADAPTER
static bool cmd_is_empty_qspi(HANDLER_OP hop)
{
        struct cmdhdr_is_empty_qspi *hdr = &cmd_state.hdr.is_empty_qspi;
        static int32_t return_val;
        uint32_t i = 0;
        uint32_t tmp_addr = ADDRESS_TMP;
        uint32_t tmp_addr2;
        uint32_t start_addr;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* something is wrong - do not execute command if size is zero */
                return hdr->size != 0;

        case HOP_EXEC:
                /* Check read and pattern buffer addresses */
                if (!check_ram_addr(tmp_addr, 2 * IS_EMPTY_CHECK_SIZE)) {
                        return false;
                }

                translate_ram_addr(&tmp_addr); // get address to big buffer
                memset((uint8_t *) tmp_addr, 0xFF, IS_EMPTY_CHECK_SIZE);  // FF pattern
                tmp_addr2 = tmp_addr + IS_EMPTY_CHECK_SIZE; // address for read values
                cmd_state.data_len = sizeof(return_val);
                cmd_state.data = (uint8_t *) &return_val;
                start_addr = QSPI_MEM1_VIRTUAL_BASE_ADDR + hdr->start_address;

                while (i < hdr->size) {
                        const uint32_t read_len = ((hdr->size - i) > IS_EMPTY_CHECK_SIZE ?
                                                        IS_EMPTY_CHECK_SIZE : (hdr->size - i));

                        if (ad_flash_read(start_addr + i, (uint8_t *) tmp_addr2, read_len) !=
                                                                                        read_len) {
                                return false;
                        } else {
                                goto compare;
                        }

compare:
                        if (memcmp((uint8_t *) tmp_addr, (uint8_t *) tmp_addr2, read_len)) {
                                uint32_t j;

                                for (j = 0; j < read_len; j++) {
                                        if (*(uint8_t *) (tmp_addr2 + j) != 0xFF) {
                                                break;
                                        }
                                }

                                return_val = (int32_t) (-1 * (i + j));
                                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                                return true;
                        }

                        i += read_len;
                }

                return_val = hdr->size;
                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                return true;
        case HOP_SEND_LEN:
                xmit_data((uint8_t *) &cmd_state.data_len , sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

#if dg_configNVMS_ADAPTER
static bool cmd_read_partition_table(HANDLER_OP hop)
{
        uint8 *ram = (uint8_t *)cmd_state.data;
        cmd_partition_table_t  *ram_table = (cmd_partition_table_t *)ram;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                return piggy_back_partition_table(cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data((void *) &ram_table->len, sizeof(ram_table->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(ram, ram_table->len);
                return true;
        }

        return false;
}

static bool cmd_read_partition(HANDLER_OP hop)
{
        struct cmdhdr_read_partition *hdr = &cmd_state.hdr.read_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);

                return ad_nvms_read(nvms, hdr->addr, cmd_state.data, hdr->len) >= 0;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

static bool cmd_write_partition(HANDLER_OP hop)
{
        struct cmdhdr_write_partition *hdr = &cmd_state.hdr.write_partition;
        nvms_t nvms;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* check for 'magic' address */
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len)) {
                        return false;
                }

                translate_ram_addr((uint32_t *) &hdr->ptr);
                return true;

        case HOP_EXEC:
                if (!ad_nvms_init_called) {
                        ad_nvms_init_called = true;
                        ad_nvms_init();
                }
                nvms = ad_nvms_open(hdr->id);

                return ad_nvms_write(nvms, hdr->addr, hdr->ptr, hdr->len) >= 0;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}
#endif

static bool cmd_chip_erase_qspi(HANDLER_OP hop)
{
        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Do not perform erase on RAM device - it will get stuck */
# if dg_configQSPI_AUTOMODE_ENABLE
                if (qspi_automode_is_ram(QSPI_MEM1_VIRTUAL_BASE_ADDR)) {
                        return false;
                }

                return ad_flash_chip_erase_by_addr(QSPI_MEM1_VIRTUAL_BASE_ADDR);
#else /* dg_configQSPI_AUTOMODE_ENABLE */
                return false;
#endif /* dg_configQSPI_AUTOMODE_ENABLE */

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}
#endif /* dg_configFLASH_ADAPTER */

#ifndef UARTBOOT_MINIMAL
/* This command is needed only by GDB Server interface */
static bool cmd_dummy(HANDLER_OP hop)
{
        char live_str[] = UARTBOOT_LIVE_MARKER;
        uint32_t tmp_addr = ADDRESS_TMP;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                if (!check_ram_addr(tmp_addr, strlen(live_str))) {
                     return false;
                }

                translate_ram_addr(&tmp_addr); // get address to big buffer
                memcpy((uint8_t *) tmp_addr, live_str, strlen(live_str));
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                return false;
        }

        return false;
}
#endif /* UARTBOOT_MINIMAL */

#if dg_configFLASH_ADAPTER
static bool cmd_direct_write_to_qspi(HANDLER_OP hop)
{
        struct cmdhdr_direct_write_qspi *hdr = &cmd_state.hdr.direct_write_qspi;
        uint8_t *read_buffer;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Read back buffer is placed just after data */
                read_buffer = (uint8_t *) (cmd_state.data + cmd_state.data_len);

                return __qspi_write(hdr->addr, cmd_state.data, cmd_state.data_len,
                                                        hdr->read_back_verify ? read_buffer : NULL);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

static bool cmd_chip_erase_eflash(HANDLER_OP hop)
{
        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                return eflash_automode_erase_chip();

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'read eFLASH' */
static bool cmd_read_eflash(HANDLER_OP hop)
{
        struct cmdhdr_read_eflash *hdr = &cmd_state.hdr.read_eflash;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* there's no payload for this command so we can safely read into buffer */
                addr = EFLASH_MEM1_VIRTUAL_BASE_ADDR + hdr->addr;

                return ad_flash_read(addr, cmd_state.data, hdr->len) == hdr->len;

        case HOP_SEND_LEN:
                xmit_data((void *) &hdr->len, sizeof(hdr->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, hdr->len);
                return true;
        }

        return false;
}

static bool cmd_direct_write_to_eflash(HANDLER_OP hop)
{
        struct cmdhdr_direct_write_qspi *hdr = &cmd_state.hdr.direct_write_qspi;
        uint8_t *read_buffer;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len > 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return true;

        case HOP_EXEC:
                /* Read back buffer is placed just after data */
                read_buffer = (uint8_t *) (cmd_state.data + cmd_state.data_len);

                return __eflash_write(hdr->addr, cmd_state.data, cmd_state.data_len,
                                                        hdr->read_back_verify ? read_buffer : NULL);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'write RAM region to eFLASH' */
static bool cmd_write_ram_to_eflash(HANDLER_OP hop)
{
        struct cmdhdr_write_ram_to_qspi *hdr = &cmd_state.hdr.write_ram_to_qspi;
        uint32_t read_buf_addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len)) {
                        return false;
                }

                /* check for 'magic' address */
                translate_ram_addr((uint32_t *) &hdr->ptr);
                return true;

        case HOP_EXEC:
#if VERIFY_EFLASH_WRITE
                /* Read buffer is placed after write buffer and has the same length */
                if (!check_ram_addr((uint32_t) hdr->ptr, hdr->len * 2)) {
                        return false;
                }

                if (((uint32_t) hdr->ptr) >= ((uint32_t) &__inputbuffer_start) &&
                                ((uint32_t) hdr->ptr) <= ((uint32_t) &__inputbuffer_end)) {
                        read_buf_addr = ((uint32_t) hdr->ptr) + hdr->len;       // move after written data
                } else {
                        /* Write is not performed from data buffer - used it for verification */
                        read_buf_addr = ADDRESS_TMP;
                        translate_ram_addr(&read_buf_addr);
                }

#else
                read_buf_addr = 0;
#endif /* VERIFY_EFLASH_WRITE */

                return __eflash_write(hdr->addr, hdr->ptr, hdr->len, (uint8_t *) read_buf_addr);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* handler for 'erase region of eFLASH' */
static bool cmd_erase_eflash(HANDLER_OP hop)
{
        struct cmdhdr_erase_qspi *hdr = &cmd_state.hdr.erase_qspi;
        uint32_t addr;

        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                return hdr->len > 0;

        case HOP_EXEC:
                addr = EFLASH_MEM1_VIRTUAL_BASE_ADDR + hdr->addr;

                return ad_flash_erase_region(addr, hdr->len);

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

static bool cmd_is_empty_eflash(HANDLER_OP hop)
{
        struct cmdhdr_is_empty_qspi *hdr = &cmd_state.hdr.is_empty_qspi;
        static int32_t return_val;
        uint32_t i = 0;
        uint32_t tmp_addr = ADDRESS_TMP;
        uint32_t tmp_addr2;
        uint32_t start_addr;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                /* something is wrong - do not execute command if size is zero */
                return hdr->size != 0;

        case HOP_EXEC:
                /* Check read and pattern buffer addresses */
                if (!check_ram_addr(tmp_addr, 2 * IS_EMPTY_CHECK_SIZE)) {
                        return false;
                }

                translate_ram_addr(&tmp_addr); // get address to big buffer
                memset((uint8_t *) tmp_addr, 0xFF, IS_EMPTY_CHECK_SIZE);  // FF pattern
                tmp_addr2 = tmp_addr + IS_EMPTY_CHECK_SIZE; // address for read values
                cmd_state.data_len = sizeof(return_val);
                cmd_state.data = (uint8_t *) &return_val;
                start_addr = hdr->start_address;

                start_addr += EFLASH_MEM1_VIRTUAL_BASE_ADDR;

                while (i < hdr->size) {
                        const uint32_t read_len = ((hdr->size - i) > IS_EMPTY_CHECK_SIZE ?
                                                        IS_EMPTY_CHECK_SIZE : (hdr->size - i));

                        if (ad_flash_read(start_addr + i, (uint8_t *) tmp_addr2, read_len) !=
                                                                                        read_len) {
                                return false;
                        } else {
                                goto compare;
                        }

compare:
                        if (memcmp((uint8_t *) tmp_addr, (uint8_t *) tmp_addr2, read_len)) {
                                uint32_t j;

                                for (j = 0; j < read_len; j++) {
                                        if (*(uint8_t *) (tmp_addr2 + j) != 0xFF) {
                                                break;
                                        }
                                }

                                return_val = (int32_t) (-1 * (i + j));
                                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                                return true;
                        }

                        i += read_len;
                }

                return_val = hdr->size;
                memcpy((uint8_t *) tmp_addr, &return_val, sizeof(return_val));
                return true;
        case HOP_SEND_LEN:
                xmit_data((uint8_t *) &cmd_state.data_len , sizeof(cmd_state.data_len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(cmd_state.data, cmd_state.data_len);
                return true;
        }

        return false;
}

#endif /* dg_configFLASH_ADAPTER */

#if (dg_configUSE_SYS_TCS == 1)
/* handler for 'get_product_info on device' */
static bool cmd_get_product_info(HANDLER_OP hop)
{
        uint8 *info = (uint8_t *)cmd_state.data;
        cmd_product_info_t *product_info = (cmd_product_info_t *)info;
        switch (hop) {
        case HOP_INIT:
                /* no payload is expected */
                return cmd_state.data_len == 0;

        case HOP_HEADER:
        case HOP_DATA:
                return true;

        case HOP_EXEC:

                return product_info_helper(cmd_state.data);

        case HOP_SEND_LEN:
                xmit_data((void *) &product_info->len, sizeof(product_info->len));
                return true;

        case HOP_SEND_DATA:
                xmit_data(info, product_info->len);
                return true;
        }

        return false;
}
#endif /* dg_configUSE_SYS_TCS */

static bool convert_baudrate(uint32_t value, HW_UART_BAUDRATE *baudrate)
{
        switch (value) {
        case 4800:
                *baudrate = HW_UART_BAUDRATE_4800;
                break;
        case 9600:
                *baudrate = HW_UART_BAUDRATE_9600;
                break;
        case 14400:
                *baudrate = HW_UART_BAUDRATE_14400;
                break;
        case 19200:
                *baudrate = HW_UART_BAUDRATE_19200;
                break;
        case 28800:
                *baudrate = HW_UART_BAUDRATE_28800;
                break;
        case 38400:
                *baudrate = HW_UART_BAUDRATE_38400;
                break;
        case 57600:
                *baudrate = HW_UART_BAUDRATE_57600;
                break;
        case 115200:
                *baudrate = HW_UART_BAUDRATE_115200;
                break;
        case 230400:
                *baudrate = HW_UART_BAUDRATE_230400;
                break;
        case 500000:
                *baudrate = HW_UART_BAUDRATE_500000;
                break;
        case 1000000:
                *baudrate = HW_UART_BAUDRATE_1000000;
                break;
        default:
                return false;
        }

        return true;
}

static bool cmd_change_baudrate(HANDLER_OP hop)
{
        struct cmdhdr_change_baudrate *hdr = &cmd_state.hdr.change_baudrate;
        static HW_UART_BAUDRATE baudrate;

        switch (hop) {
        case HOP_INIT:
                return cmd_state.data_len == 0;

        case HOP_HEADER:
                return true;

        case HOP_DATA:
                // Update init baudrate
                return convert_baudrate(hdr->baudrate, &baudrate);

        case HOP_EXEC:
                UART_INIT.baud_rate = baudrate;
                hw_uart_reinit(BOOTUART, &UART_INIT);
                return true;

        case HOP_SEND_LEN:
        case HOP_SEND_DATA:
                /* nothing to send back */
                return false;
        }

        return false;
}

/* provided by linker script */
extern char __patchable_params;

static void init(void)
{
                timer_config t_cfg = {
                        .clk_src = HW_TIMER_CLK_SRC_EXT,
                        .prescaler = 0x1f, // 32MHz / (31 + 1) = 1MHz

                        .timer = {
                                .direction = HW_TIMER_DIR_UP,
                                .reload_val = 999999, // interrupt every 1s
                        },
                };
        uint32_t *pparams = (void*)&__patchable_params;
        HW_GPIO_PORT tx_port, rx_port;
        HW_GPIO_PIN tx_pin, rx_pin;

        /*
         * get UART parameters from patchable area, if their value is not 0xffffffff,
         * or else, use the CFG_* values
         */
        if (pparams[0] != 0xffffffff)
                tx_port = (HW_GPIO_PORT)pparams[0];
        else
                tx_port = CFG_GPIO_BOOTUART_TX_PORT;
        if (pparams[1] != 0xffffffff)
                tx_pin = (HW_GPIO_PIN)pparams[1];
        else
                tx_pin = CFG_GPIO_BOOTUART_TX_PIN;
        if (pparams[2] != 0xffffffff)
                rx_port = (HW_GPIO_PORT)pparams[2];
        else
                rx_port = CFG_GPIO_BOOTUART_RX_PORT;
        if (pparams[3] != 0xffffffff)
                rx_pin = (HW_GPIO_PIN)pparams[3];
        else
                rx_pin = CFG_GPIO_BOOTUART_RX_PIN;
        if (pparams[4] != 0xffffffff)
        {
                convert_baudrate(pparams[4], &UART_INIT.baud_rate);
        }

        REG_SETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP, 0);
        hw_gpio_set_pin_function(tx_port, tx_pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(rx_port, rx_pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_UART2_RX);

        hw_uart_init(BOOTUART, &UART_INIT);

        hw_timer_init(HW_TIMER, &t_cfg);

#if dg_configFLASH_ADAPTER
        ad_flash_init();
#endif

        /* Switch to PLL 96 MHz */

}

/* transmit announcement message every 1s and wait for <SOH> response */
static void wait_for_soh(void)
{
        uart_soh = false;
        timer1_soh_tmo = true;

        hw_timer_register_int(HW_TIMER, timer1_soh_cb);
        hw_timer_enable(HW_TIMER);
        hw_timer_enable_clk(HW_TIMER);

        while (!uart_soh) {
                if (timer1_soh_tmo) {
                        timer1_soh_tmo = false;
#if (dg_configSUPPRESS_HelloMsg == 0)
                        xmit_hello();
#endif
                        hw_uart_receive(BOOTUART, uart_buf, 1, (hw_uart_rx_callback) uart_soh_cb,
                                                                                        uart_buf);
                }

                __WFI();
        };

        hw_timer_disable(HW_TIMER);
}

static void process_header(void)
{
        memset(&cmd_state, 0, sizeof(cmd_state));
        cmd_state.data = &__inputbuffer_start;

        input_buffer_size = &__inputbuffer_end - &__inputbuffer_start;

        cmd_state.type = uart_buf[1];
        cmd_state.len = uart_buf[2] | (uart_buf[3] << 8);

        switch (cmd_state.type) {
        case CMD_WRITE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.send_to_ram);
                cmd_state.handler = cmd_send_to_ram;
                break;
#ifndef UARTBOOT_MINIMAL
        case CMD_READ:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_from_ram);
                cmd_state.handler = cmd_read_from_ram;
                break;
#endif /* UARTBOOT_MINIMAL */
#if dg_configFLASH_ADAPTER
        case CMD_COPY_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_ram_to_qspi);
                cmd_state.handler = cmd_write_ram_to_qspi;
                break;
        case CMD_ERASE_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.erase_qspi);
                cmd_state.handler = cmd_erase_qspi;
                break;
#endif /* dg_configFLASH_ADAPTER */
        case CMD_RUN:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.execute_code);
                cmd_state.handler = cmd_execute_code;
                break;
#ifndef UARTBOOT_MINIMAL
        case CMD_WRITE_OTP:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_otp);
                cmd_state.handler = cmd_write_otp;
                break;
        case CMD_READ_OTP:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_otp);
                cmd_state.handler = cmd_read_otp;
                break;
#endif /* UARTBOOT_MINIMAL */
#if dg_configFLASH_ADAPTER
        case CMD_READ_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_qspi);
                cmd_state.handler = cmd_read_qspi;
                break;
#endif /* dg_configFLASH_ADAPTER */
        case CMD_GET_VERSION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.get_version);
                cmd_state.handler = cmd_get_version;
                break;
#if dg_configFLASH_ADAPTER
        case CMD_CHIP_ERASE_QSPI:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_chip_erase_qspi;
                break;
        case CMD_IS_EMPTY_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.is_empty_qspi);
                cmd_state.handler = cmd_is_empty_qspi;
                break;
#if dg_configNVMS_ADAPTER
        case CMD_READ_PARTITION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_partition);
                cmd_state.handler = cmd_read_partition;
                break;
        case CMD_WRITE_PARTITION:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_partition);
                cmd_state.handler = cmd_write_partition;
                break;
        case CMD_READ_PARTITION_TABLE:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_read_partition_table;
                break;
#endif /* dg_configNVMS_ADAPTER */
        case CMD_GET_QSPI_STATE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.get_qspi_state);
                cmd_state.handler = cmd_get_qspi_state;
                break;
#endif /* dg_configFLASH_ADAPTER */
        case CMD_GPIO_WD:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.gpio_wd);
                cmd_state.handler = cmd_gpio_wd;
                break;
#if dg_configFLASH_ADAPTER
        case CMD_DIRECT_WRITE_TO_QSPI:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.direct_write_qspi);
                cmd_state.handler = cmd_direct_write_to_qspi;
                break;
        case CMD_CHIP_ERASE_EFLASH:
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_chip_erase_eflash;
                break;
        case CMD_READ_EFLASH:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.read_eflash);
                cmd_state.handler = cmd_read_eflash;
                break;
        case CMD_DIRECT_WRITE_TO_EFLASH:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.direct_write_eflash);
                cmd_state.handler = cmd_direct_write_to_eflash;
                break;
        case CMD_COPY_EFLASH:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.write_ram_to_eflash);
                cmd_state.handler = cmd_write_ram_to_eflash;
                break;
        case CMD_ERASE_EFLASH:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.erase_eflash);
                cmd_state.handler = cmd_erase_eflash;
                break;
        case CMD_IS_EMPTY_EFLASH:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.is_empty_eflash);
                cmd_state.handler = cmd_is_empty_eflash;
                break;
#endif /* dg_configFLASH_ADAPTER */

#if dg_configUSE_SYS_TCS
        case CMD_GET_PRODUCT_INFO:
               cmd_state.hdr_len = 0;
               cmd_state.handler = cmd_get_product_info;
               break;

#endif /* dg_configUSE_SYS_TCS */
        case CMD_CHANGE_BAUDRATE:
                cmd_state.hdr_len = sizeof(cmd_state.hdr.change_baudrate);
                cmd_state.handler = cmd_change_baudrate;
                break;
#ifndef UARTBOOT_MINIMAL
        case CMD_DUMMY:
                /* Dummy command - only for GDB server interface */
                cmd_state.hdr_len = 0;
                cmd_state.handler = cmd_dummy;
                break;
#endif /* UARTBOOT_MINIMAL */
        }

        /* store length of payload (command data excluding command header) */
        cmd_state.data_len = cmd_state.len - cmd_state.hdr_len;
}

/* wait for command header (type + length) */
static bool wait_for_cmd(void)
{
        int soh_len = 1;

        /*
         * uart_soh is set when SOH was already received in response to announcement thus we won't
         * receive another one here. By resetting this flag we make sure that for next command we'll
         * expect SOH to be received here.
         */
        if (uart_soh) {
                soh_len = 0;
        }
        uart_soh = false;

        if (!recv_with_tmo(uart_buf + 1 - soh_len, 3 + soh_len, TMO_COMMAND)) {
                return false;
        }

        process_header();

        return true;
}


static bool load_data(void)
{
        bool ret;

        /* receive command header */
        if (!recv_with_tmo((uint8_t *) &cmd_state.hdr, cmd_state.hdr_len, TMO_DATA)) {
                return false;
        }

        cmd_state.handler(HOP_HEADER);

        /* receive command payload */
        if (!recv_with_tmo(cmd_state.data, cmd_state.data_len,
                                        1 + cmd_state.data_len * (UART_INIT.baud_rate / 10))) {
                return false;
        }

        crc16_init(&cmd_state.crc);
        crc16_update(&cmd_state.crc, (uint8_t *) &cmd_state.hdr, cmd_state.hdr_len);
        crc16_update(&cmd_state.crc, cmd_state.data, cmd_state.data_len);

        ret = cmd_state.handler(HOP_DATA);

        if (!ret) {
                xmit_nak();
                return false;
        }

        xmit_ack();
        xmit_crc16(cmd_state.crc);

        ret = recv_with_tmo(uart_buf, 1, TMO_ACK);
        ret &= (uart_buf[0] == ACK);
        if (ret) {
                ret &= cmd_state.handler(HOP_EXEC);
        }

        ret ? xmit_ack() : xmit_nak();

        return ret;
}

static void swd_handle_header(void)
{
        const HANDLER_OP hop[] = { HOP_INIT, HOP_HEADER, HOP_DATA, HOP_EXEC };
        int i;

        process_header();
        memcpy(&cmd_state.hdr, uart_buf + 4, cmd_state.hdr_len);

        if (!cmd_state.handler) {
                return;
        }

        for (i = 0; i < (sizeof(hop) / sizeof(hop[0])); i++) {
                /* Don't perform next step if previous fails */
                if (!cmd_state.handler(hop[i])) {
                        set_ack_nak_field(NAK);
                        return;
                }
        }

        /* This point won't be reached if error occurs */
        set_ack_nak_field(ACK);
}

/*
 * swd_interface.run_swd is constant value 0
 * Debugger will setup it to 1 when uartboot is to be controlled from debugger
 */
void swd_loop(void) {
        uint32_t last_num = swd_interface.cmd_num;
        uint32_t current_num;
        while (swd_interface.run_swd) {
                current_num = swd_interface.cmd_num;
                if (last_num != current_num) {
                        last_num = current_num;
                        /* Debugger put header in uart_buf, process it */
                        swd_handle_header();
                }

                /* Make sure to enter breakpoint only when debugger is attached */
                if (REG_GETF(CRG_TOP, SYS_STAT_REG, DBG_IS_ACTIVE)) {
                        __BKPT(12);
                }
        }
}

#ifdef UARTBOOT_MINIMAL
/**
 * Enables the CMAC cache area to be used as RAM
 */
void UserApp_SystemInitPre(void)
{
        /*
         * Enable CMAC cache to be used for data
         */
        hw_sys_enable_cmac_cache_ram();
}
#endif

int main()
{

        hw_watchdog_freeze();
        hw_gpio_pad_latch_enable_all();


#if dg_configUSE_HW_QSPI
        /* qspi */
        hw_qspi_set_div(HW_QSPIC, HW_QSPI_CLK_DIV_1);
        hw_qspi_clock_enable(HW_QSPIC);
#endif /* dg_configUSE_HW_QSPI */


        eflash_automode_wakeup();


        init();

        swd_loop();

soh_loop:
        wait_for_soh();

cmd_loop:
        /* receive command header (type + length) */
        if (!wait_for_cmd()) {
                goto soh_loop;
        }

        /* NAK for commands we do not support or have faulty header, i.e. length is incorrect */
        if (!cmd_state.handler || !cmd_state.handler(HOP_INIT)) {
                xmit_nak();
                goto cmd_loop;
        }

        xmit_ack();
        /* receive data from CLI */
        if (cmd_state.len) {
                if (!load_data()) {
                        if (uart_tmo) {
                                goto soh_loop;
                        } else {
                                goto cmd_loop;
                        }
                }
        } else {
                if (!cmd_state.handler(HOP_EXEC)) {
                        xmit_nak();
                        goto cmd_loop;
                }
                xmit_ack();
        }

        /* send data length of response, if any */
        if (!cmd_state.handler(HOP_SEND_LEN)) {
                goto cmd_loop;
        }
        if (!recv_with_tmo(uart_buf, 1, 5) || uart_buf[0] != ACK) {
                goto soh_loop;
        }

        /* send response data */
        crc16_init(&cmd_state.crc);
        if (!cmd_state.handler(HOP_SEND_DATA)) {
                goto soh_loop;
        }

        /* receive and check CRC */
        if (!recv_with_tmo(uart_buf, 2, 5)) {
                goto soh_loop;
        }
        if (!memcmp(uart_buf, &cmd_state.crc, 2)) { // we're l-endian and CRC is transmitted lsb-first
                xmit_ack();
        } else {
                xmit_nak();
        }

        goto cmd_loop;

        return 0;
}

/*
 * \}
 * \}
 * \}
 */
