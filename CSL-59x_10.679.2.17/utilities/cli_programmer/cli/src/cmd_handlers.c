/**
 ****************************************************************************************
 *
 * @file cmd_handlers.c
 *
 * @brief Handling of CLI commands provided on command line
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <programmer.h>
#include "cli_common.h"

#define DEFAULT_SIZE_QSPI_FLASH 0x00100000
#define DEFAULT_SIZE_EFLASH     0x00040000

/* Maximum size for an image */
#define MAX_IMAGE_SIZE 0x7F000

/* Supported QSPI memories */
#define ADESTO_ID       0x1F
#define GIGADEVICE_ID   0xC8
#define MACRONIX_ID     0xC2
#define WINBOND_ID      0xEF

/* Unsupported QSPI memories */
#define ISSI_ID         0x9D
#define PUYA_ID         0x85
#define XMC_ID          0x20
#define XTX_ID          0x0B
#define EON_ID          0x1C

/* Pointer will hold memory for executable application code */
static uint8_t *executable_bin;
/* Executable application size */
static size_t executable_size;

static int cmdh_write(int argc, char *argv[]);
static int cmdh_read(int argc, char *argv[]);
static int cmdh_write_qspi(int argc, char *argv[]);
static int cmdh_write_qspi_bytes(int argc, char *argv[]);
static int cmdh_read_qspi(int argc, char *argv[]);
static int cmdh_read_partition_table(int argc, char *argv[]);
static int cmdh_read_partition(int argc, char *argv[]);
static int cmdh_write_partition(int argc, char *argv[]);
static int cmdh_write_partition_bytes(int argc, char *argv[]);
static int cmdh_erase_qspi(int argc, char *argv[]);
static int cmdh_chip_erase_qspi(int argc, char *argv[]);
static int cmdh_copy_qspi(int argc, char *argv[]);
static int cmdh_is_empty_qspi(int argc, char *argv[]);
static int cmdh_write_otp(int argc, char *argv[]);
static int cmdh_read_otp(int argc, char *argv[]);
static int cmdh_boot(int argc, char *argvp[]);
static int cmdh_run(int argc, char *argvp[]);
static int cmdh_get_product_info(int argc, char *argvp[]);
static int cmdh_chip_erase_eflash(int argc, char *argv[]);
static int cmdh_write_eflash(int argc, char *argv[]);
static int cmdh_read_eflash(int argc, char *argv[]);
static int cmdh_copy_eflash(int argc, char *argv[]);
static int cmdh_erase_eflash(int argc, char *argv[]);
static int cmdh_is_empty_eflash(int argc, char *argv[]);
static int cmdh_write_eflash_bytes(int argc, char *argv[]);
static int cmdh_read_flash_info(int argc, char *argv[]);

/**
 * \brief CLI command handler description
 *
 */
struct cli_command {
        const char *name;                       /**< name of command */
        int min_num_p;                          /**< minimum number of parameters */
        int (* func) (int argc, char *argv[]);  /**< handler function, return non-zero for success */
};

/**
 * \brief CLI command handlers
 *
 */
static struct cli_command cmds[] = {
        { "write",                 2, cmdh_write, },
        { "read",                  3, cmdh_read, },
        { "write_qspi",            2, cmdh_write_qspi, },
        { "write_qspi_bytes",      2, cmdh_write_qspi_bytes, },
        { "read_qspi",             3, cmdh_read_qspi, },
        { "erase_qspi",            2, cmdh_erase_qspi, },
        { "chip_erase_qspi",       0, cmdh_chip_erase_qspi, },
        { "read_partition_table",  0, cmdh_read_partition_table, },
        { "read_partition",        4, cmdh_read_partition, },
        { "write_partition",       3, cmdh_write_partition, },
        { "write_partition_bytes", 3, cmdh_write_partition_bytes, },
        { "copy_qspi",             3, cmdh_copy_qspi, },
        { "is_empty_qspi",         0, cmdh_is_empty_qspi, },
        { "write_otp",             2, cmdh_write_otp, },
        { "read_otp",              2, cmdh_read_otp, },
        { "boot",                  1, cmdh_boot, },
        { "run",                   1, cmdh_run, },
        { "get_product_info",      0, cmdh_get_product_info, },
        { "copy_eflash",           3, cmdh_copy_eflash, },
        { "erase_eflash",          2, cmdh_erase_eflash, },
        { "is_empty_eflash",       0, cmdh_is_empty_eflash, },
        { "write_eflash_bytes",    2, cmdh_write_eflash_bytes, },
        { "write_eflash",          2, cmdh_write_eflash, },
        { "read_eflash",           3, cmdh_read_eflash, },
        { "chip_erase_eflash",     0, cmdh_chip_erase_eflash, },
        {"read_flash_info",        0, cmdh_read_flash_info, },
        /* end of table */
        { NULL,             0, NULL, }
};

static int get_filesize(const char *fname)
{
        struct stat st;

        if (stat(fname, &st) < 0) {
                return -1;
        }

        return st.st_size;
}

static bool check_otp_cell_address_range(uint32_t *addr, uint32_t length)
{
        /* In DA1459x, the OTP is eFLASH-emulated, but we choose to access the data in 32bit
         * words, which we consider as 'cells' (in accordance with other devices having a
         * physical OTP block that is usually organized in cells too). */
        uint32_t _cell_addr = *addr;
        uint32_t otp_base_c_mem = 0x00A00000;
        uint32_t otp_base_s_mem = 0x31000000;
        uint8_t  addr_to_cel_div = 2; /*4 bytes per cell */

        /* If physical byte address is given, convert it to relative word address */
        if (_cell_addr >= otp_base_s_mem) {
                _cell_addr = (_cell_addr - otp_base_s_mem) >> addr_to_cel_div;
        }
        else if (_cell_addr >= otp_base_c_mem) {
                _cell_addr = (_cell_addr - otp_base_c_mem) >> addr_to_cel_div;
        }
        /* Make sure that given address range lies within the write-protectable sectors,
         * i.e. inside the following ranges (assuming byte addresses):
         * 0x00000000 - 0x00000FFF (sectors 1-2)
         * 0x00002000 - 0x00005FFF (sectors 5-12)
         * 0x00040000 - 0x000407FF ('Information Page' sector)
         * If not, return false. */
        if ((_cell_addr + length <= 0x00000400) ||
              ((_cell_addr >= 0x00000800) && (_cell_addr + length <= 0x00001800)) ||
              ((_cell_addr >= 0x00010000) && (_cell_addr + length <= 0x00010200))) {
                *addr = _cell_addr;
                return true;
        }
        return false;
}

static int cmdh_write(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        ret = prog_write_file_to_ram(addr, fname, size);
        if (ret) {
                prog_print_err("write to RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                ret = prog_read_memory(addr, buf, size);
        } else {
                ret = prog_read_memory_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->qspi_size;

        if (size > size_limit) {
                prog_print_err("invalid size exceeding QSPI size\n");
                return 0;
        }

        ret = prog_write_file_to_qspi(addr, fname, size);
        if (ret) {
                prog_print_err("write to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_qspi_bytes(int argc, char *argv[])
{
        unsigned int addr;
        int ret;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc);

        if (buf == NULL) {
                return 0;
        }

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                ret = 0;
                goto end;
        }

        for (i = 1; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        ret = 0;
                        goto end;
                }
                buf[i - 1] = (uint8_t) b;
        }

        ret = prog_write_to_qspi(addr, buf, argc - 1);
        if (ret) {
                prog_print_err("write to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto end;
        } else {
                ret = 1;
        }
end:
        free(buf);
        return ret;
}


static int cmdh_read_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->qspi_size;

        if (size > size_limit) {
                prog_print_err("invalid size exceeding QSPI size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {

                uint8_t *buf = malloc(size);
                if (buf == NULL) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_qspi(addr, buf, size);
                        if (!ret) {
                                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                        }
                        free(buf);
                }

        } else {
                ret = prog_read_qspi_to_file(addr, fname, size);
        }
        if (ret) {
                prog_print_err("read from QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_erase_qspi(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[1], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_erase_qspi(addr, size);
        if (ret) {
                prog_print_err("erase QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read_partition_table(int argc, char *argv[])
{
        int ret = 0;
        unsigned int size = 0;
        uint8_t *buf = NULL;

        ret = prog_read_partition_table(&buf, &size);
        if (ret) {
                prog_print_err("read partition table failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                goto done;
        }

        ret = dump_partition_table(buf, size);

done:
        free(buf);
        return (ret == 0);
}

static int cmdh_read_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id or selected partition doesn't "
                                                                                        "exist\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[3], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_partition(id, addr, buf, size);
                }
        } else {
                ret = prog_read_patrition_to_file(id, addr, fname, size);
        }

        if (ret) {
                free(buf);
                prog_print_err("read from partition failed (%d)\n", ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 3) {
                if (!get_number(argv[3], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        ret = prog_write_file_to_partition(id, addr, fname, size);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_partition_bytes(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int id;
        int ret = 0;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc - 2);

        if (buf == NULL) {
                return ERR_ALLOC_FAILED;
        }

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        goto end;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                goto end;
        }

        for (i = 2; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        goto end;
                }
                buf[i - 2] = (uint8_t) b;
        }

        ret = prog_write_partition(id, addr, buf, argc - 2);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                ret = 0;
                goto end;
        }
        ret = 1;

end:
        free(buf);

        return ret;
}

static int cmdh_copy_qspi(int argc, char *argv[])
{
        unsigned int addr_ram;
        unsigned int addr_qspi;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr_ram)) {
                prog_print_err("invalid RAM address\n");
                return 0;
        }

        if (!get_number(argv[1], &addr_qspi)) {
                prog_print_err("invalid QSPI address\n");
                return 0;
        }

        if (!get_number(argv[2], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_copy_to_qspi(addr_ram, addr_qspi, size);
        if (ret) {
                prog_print_err("copy to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_is_empty_qspi(int argc, char *argv[])
{
        unsigned int size = DEFAULT_SIZE_QSPI_FLASH;
        unsigned int start_address = 0;
        int ret_number;
        int ret = 0;

        if (argc != 0 && argc != 2) {
                prog_print_err("invalid argument - function is_empty_qspi needs zero or two"
                                                                                "arguments\n");
                return 0;
        }

        if (argc == 2) {
                if (!get_number(argv[0], &start_address)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (!get_number(argv[1], &size) || size == 0) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        }

        ret = prog_is_empty_qspi(size, start_address, &ret_number);

        if (!ret) {
                if (ret_number <= 0) {
                        prog_print_log("QSPI flash region is not empty (byte at 0x%08x + 0x%08x is not "
                                        "0xFF).\n", start_address, (-1 * ret_number));
                } else {
                        prog_print_log("QSPI flash region is empty (checked %u bytes).\n", ret_number);
                }
                ret = 1;
        } else {
                prog_print_err("check QSPI emptiness failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
        }

        return ret;
}

static int cmdh_chip_erase_qspi(int argc, char *argv[])
{
        int ret;

        ret = prog_chip_erase_qspi();

        if (ret) {
                prog_print_err("chip erase QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int i;
        int ret = 0;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        argc -= 2;
        argv += 2;

        if (!check_otp_cell_address_range(&addr, length)) {
                prog_print_err("The specified address range does not lie fully "
                        "within the write-protectable area of eFLASH.\n");
                goto done;
        }

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        for (i = 0; i < argc && i < length; i++) {
                if (!get_number(argv[i], &buf[i])) {
                        prog_print_err("invalid data (#%d)\n", i + 1);
                        goto done;
                }
        }

        ret = prog_write_otp(addr, buf, length);
        if (ret) {
                prog_print_err("write to OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto done;
        }

        ret = 1;

done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int cmdh_read_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int ret = 0;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        if (!check_otp_cell_address_range(&addr, length)) {
                prog_print_err("The specified address range does not lie fully "
                        "within the write-protectable area of eFLASH.\n");
                goto done;
        }
        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        ret = prog_read_otp(addr, buf, length);
        if (ret) {
                prog_print_err("read from OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }

        dump_otp(addr, buf, length);
        ret = 1;
done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int set_executable_from_file(const char *file_name)
{
        FILE *f;
        struct stat st;
        int err = 0;

        if (file_name == NULL || stat(file_name, &st) < 0) {
                return ERR_FILE_OPEN;
        }

        f = fopen(file_name, "rb");
        if (f == NULL) {
                return ERR_FILE_OPEN;
        }

        executable_bin = realloc(executable_bin, st.st_size);
        if (executable_bin == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (st.st_size != fread(executable_bin, 1, st.st_size, f)) {
                err = ERR_FILE_READ;
                goto end;
        }

        if (st.st_size > 0) {
                executable_size = st.st_size;
        } else {
                err = ERR_FILE_EMPTY;
        }
end:
        if (f != NULL) {
                fclose(f);
        }

        return err;
}

int cmdh_boot(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_boot(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to boot executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

int cmdh_run(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_run(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to run executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

/* Example of product information output:
         * Device classification attributes:
         * Device family: DA1459x
         * Device chip ID: D2634
         * Device variant: DA14592
         * Device version (revision|step): A1
         *
         * Production layout information:
         * Package = WLCSP39
         *
         * Production testing information:
         * Timestamp = 0x1689D30A
         */
int cmdh_get_product_info(int argc, char *argv[])
{
        int ret = 0;
        unsigned int size = 0;
        uint8_t *buf = NULL;


        ret = prog_get_product_info(&buf, &size);
        if (ret) {
                prog_print_err("get product info failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }

        ret = dump_product_info(buf, size);

done:
        free(buf);
        return (ret == 0);
}

static int cmdh_chip_erase_eflash(int argc, char *argv[])
{
        int ret;

        ret = prog_chip_erase_eflash();

        if (ret) {
                prog_print_err("chip erase eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_eflash(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->eflash_size;


        if (size > size_limit) {
                prog_print_err("invalid size exceeding eFLASH size\n");
                return 0;
        }

        ret = prog_write_file_to_eflash(addr, fname, size);
        if (ret) {
                prog_print_err("write to eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}
static int cmdh_read_eflash(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        unsigned int size_limit;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        size_limit = sizes->eflash_size;

        if (size > size_limit) {
                prog_print_err("invalid size exceeding eFLASH size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {

                uint8_t *buf = malloc(size);
                if (buf == NULL) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_eflash(addr, buf, size);
                        if (!ret) {
                                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                        }
                        free(buf);
                }

        } else {
                ret = prog_read_eflash_to_file(addr, fname, size);
        }
        if (ret) {
                prog_print_err("read from eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_copy_eflash(int argc, char *argv[])
{
        unsigned int addr_ram;
        unsigned int addr_eflash;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr_ram)) {
                prog_print_err("invalid RAM address\n");
                return 0;
        }

        if (!get_number(argv[1], &addr_eflash)) {

                prog_print_err("invalid eFLASH address\n");

                return 0;
        }

        if (!get_number(argv[2], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_copy_to_eflash(addr_ram, addr_eflash, size);
        if (ret) {
                prog_print_err("copy to eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_erase_eflash(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[1], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_erase_eflash(addr, size);
        if (ret) {
                prog_print_err("erase eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_is_empty_eflash(int argc, char *argv[])
{
        unsigned int size = DEFAULT_SIZE_EFLASH;
        unsigned int start_address = 0;
        int ret_number;
        int ret = 0;

        if (argc != 0 && argc != 2) {
                prog_print_err("invalid argument - function is_empty_eflash needs zero or two"
                                                                                "arguments\n");
                return 0;
        }

        if (argc == 2) {
                if (!get_number(argv[0], &start_address)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (!get_number(argv[1], &size) || size == 0) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        }

        ret = prog_is_empty_eflash(size, start_address, &ret_number);

        if (!ret) {
                if (ret_number <= 0) {
                        prog_print_log("eFLASH flash region is not empty (byte at 0x%08x + 0x%08x is not "
                                        "0xFF).\n", start_address, (-1 * ret_number));
                } else {
                        prog_print_log("eFLASH flash region is empty (checked %u bytes).\n", ret_number);
                }
                ret = 1;
        } else {
                prog_print_err("check eFLASH emptiness failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
        }

        return ret;
}

static int cmdh_write_eflash_bytes(int argc, char *argv[])
{
        unsigned int addr;
        int ret;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc);

        if (buf == NULL) {
                return 0;
        }

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                ret = 0;
                goto end;
        }

        for (i = 1; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        ret = 0;
                        goto end;
                }
                buf[i - 1] = (uint8_t) b;
        }

        ret = prog_write_to_eflash(addr, buf, argc - 1);
        if (ret) {
                prog_print_err("write to eFLASH failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto end;
        } else {
                ret = 1;
        }
end:
        free(buf);
        return ret;
}


static int cmdh_read_flash_info(int argc, char *argv[])
{
        int ret;

        if (argc != 0) {
                prog_print_err("invalid argument - read_flash_info takes no argument\n");
                return 0;
        }
/*
 * eFLASH information, applicable only on DA1459X
 */
        prog_print_log("Εmbedded flash mem info:\n");
        prog_print_log("Device size = 256 KB\n");
        prog_print_log("\n");

        const char *man, *model;
        unsigned int size;

        flash_info_t flash_info = {
                .qspic_id         = 0,
                .qspi_flash_info  = {
                        .driver_configured = false,
                        .man_id            = 0x0,
                        .type              = 0x0,
                        .density           = 0x0
                },
                .oqspi_flash_info = {
                        .driver_configured = false,
                        .man_id            = 0x0,
                        .type              = 0x0,
                        .density           = 0x0
                }
        };

        ret = prog_read_flash_info(&flash_info);

        if (ret < 0) {
                prog_print_err("failed to read flash mem info: %s (%d)\n", prog_get_err_message(ret), ret);

                return 0;
        }

        if (flash_info.qspi_flash_info.driver_configured == true) {
                prog_print_log("QSPI flash mem info:\n");

                switch (flash_info.qspi_flash_info.man_id) {
                case ADESTO_ID:
                        man = "Adesto";

                        if ((flash_info.qspi_flash_info.type == 0x42) &&
                                        (flash_info.qspi_flash_info.density == 0x18)) {
                                model = "AT25SL128";
                                size  = 16; // 128 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case WINBOND_ID:
                        man = "Winbond";

                        if ((flash_info.qspi_flash_info.type == 0x80) &&
                                        (flash_info.qspi_flash_info.density == 0x19)) {
                                model = "W25Q256JW";
                                size  = 32; // 256 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;
                default:
                        man   = "N/A";
                        model = "N/A";
                        size  = 0;
                        break;
                }

                prog_print_log("Manufacturer = %s\n", man);
                prog_print_log("Device model = %s\n", model);
                prog_print_log("Device size = %u MB\n", size);
        } else {
                if (flash_info.qspi_flash_info.man_id != 0) {
                        prog_print_log("Unsupported QSPI flash memory\n");
                        switch (flash_info.qspi_flash_info.man_id) {
                        case ISSI_ID:
                                man = "ISSI";
                                break;
                        case PUYA_ID:
                                man = "Puya";
                                break;
                        case XMC_ID:
                                man = "XMC";
                                break;
                        case XTX_ID:
                                man = "XTX";
                                break;
                        case EON_ID:
                                man = "EON";
                                break;
                        case ADESTO_ID:
                                man = "Adesto";
                                break;
                        case GIGADEVICE_ID:
                                man = "Gigadevice";
                                break;
                        case MACRONIX_ID:
                                man = "Macronix";
                                break;
                        case WINBOND_ID:
                                man = "Winbond";
                                break;
                        default:
                                man = "Unknown";
                                break;
                        }
                        prog_print_log("Manufacturer = %s\n", man);
                        prog_print_log("Manufacturer ID = %x\n", flash_info.qspi_flash_info.man_id);
                        prog_print_log("Memory Type = %x\n", flash_info.qspi_flash_info.type);
                        prog_print_log("Memory Density = %x\n", flash_info.qspi_flash_info.density);
                } else {
                        prog_print_log("QSPI flash NOT present\n");
                }
        }

        prog_print_log("\n");

        if (flash_info.oqspi_flash_info.driver_configured == true) {
                prog_print_log("OQSPI flash mem info:\n");

                switch (flash_info.oqspi_flash_info.man_id) {
                case GIGADEVICE_ID:
                        man = "GigaDevice";

                        if ((flash_info.oqspi_flash_info.type == 0x60) &&
                                        (flash_info.oqspi_flash_info.density == 0x17)) {
                                model = "GD25LQ64C/GD25LE64E";
                                size  = 8; // 64 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case MACRONIX_ID:
                        man = "Macronix";

                        if ((flash_info.oqspi_flash_info.type == 0x25) &&
                                (flash_info.oqspi_flash_info.density == 0x37)) {
                                model = "MX25U6432";
                                size  = 8; // 64 megabits
                        } else if ((flash_info.oqspi_flash_info.type == 0x80) &&
                                        (flash_info.oqspi_flash_info.density == 0x3B)) {
                                model = "MX66UM1G45G";
                                size  = 128; // 1 gigabit
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                case WINBOND_ID:
                        man = "Winbond";

                        if ((flash_info.oqspi_flash_info.type == 0x80) &&
                                        (flash_info.oqspi_flash_info.density == 0x17)) {
                                model = "W25Q64JWIM";
                                size = 8; // 64 megabits
                        } else {
                                model = "N/A";
                                size  = 0;
                        }

                        break;

                default:
                        man   = "N/A";
                        model = "N/A";
                        size  = 0;
                        break;
                }

                prog_print_log("Manufacturer = %s\n", man);
                prog_print_log("Device model = %s\n", model);
                prog_print_log("Device size = %u MB\n", size);
        } else {
                prog_print_log("OQSPI flash NOT present\n");
        }
        return 1;

}

int handle_command(char *cmd, int argc, char *argv[])
{
        struct cli_command *cmdh = cmds;

        /* lookup command handler */
        while (cmdh->name && strcmp(cmdh->name, cmd)) {
                cmdh++;
        }

        /* handlers table is terminated by empty entry, so name == NULL means no handler found */
        if (!cmdh->name) {
                prog_print_err("invalid command\n");
                return 1;
        }

        if (argc < cmdh->min_num_p) {
                prog_print_err("not enough parameters\n");
                return 1;
        }

        /*
         * return value from handler (0=failure) will be used as exit code so need to do change it
         * here to have exit code 0 on success
         */
        return !cmdh->func(argc, argv);
}
