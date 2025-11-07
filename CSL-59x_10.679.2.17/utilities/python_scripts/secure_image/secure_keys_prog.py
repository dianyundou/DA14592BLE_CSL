#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2024 Renesas Electronics Corporation and/or its affiliates.
# All rights reserved. Confidential Information.
#
# This software ("Software") is supplied by Renesas Electronics Corporation and/or its
# affiliates ("Renesas"). Renesas grants you a personal, non-exclusive, non-transferable,
# revocable, non-sub-licensable right and license to use the Software, solely if used in
# or together with Renesas products. You may make copies of this Software, provided this
# copyright notice and disclaimer ("Notice") is included in all such copies. Renesas
# reserves the right to change or discontinue the Software at any time without notice.
#
# THE SOFTWARE IS PROVIDED "AS IS". RENESAS DISCLAIMS ALL WARRANTIES OF ANY KIND,
# WHETHER EXPRESS, IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. TO THE
# MAXIMUM EXTENT PERMITTED UNDER LAW, IN NO EVENT SHALL RENESAS BE LIABLE FOR ANY DIRECT,
# INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE, EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGES. USE OF THIS SOFTWARE MAY BE SUBJECT TO TERMS AND CONDITIONS CONTAINED IN
# AN ADDITIONAL AGREEMENT BETWEEN YOU AND RENESAS. IN CASE OF CONFLICT BETWEEN THE TERMS
# OF THIS NOTICE AND ANY SUCH ADDITIONAL LICENSE AGREEMENT, THE TERMS OF THE AGREEMENT
# SHALL TAKE PRECEDENCE. BY CONTINUING TO USE THIS SOFTWARE, YOU AGREE TO THE TERMS OF
# THIS NOTICE.IF YOU DO NOT AGREE TO THESE TERMS, YOU ARE NOT PERMITTED TO USE THIS
# SOFTWARE.
#########################################################################################

import os
import sys
from collections import OrderedDict


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.script_base import run_script, ScriptArgumentsParser
from eflash.program_eflash_config import ProductId
from eflash.program_eflash import PRODUCT_HEADER_PRIMARY_ADDRESS
from secure_image.secure_keys_cfg import SecurityConfig, secure_keys_cfg
from secure_image.generate_keys import ProductKeys
import secure_image.sticky_bits as sb

DEFAULT_PRODUCT_KEYS_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                         'product_keys.xml'))
DEFAULT_SECURE_CFG_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                       'secure_cfg.xml'))
# Configuration script address and maximum size in eFlash
AREA_CS_START = 0x00000000
AREA_CS_SIZE = 0x7C0
AREA_CS_END = (AREA_CS_START + AREA_CS_SIZE)
# Configuration script key words
CS_START_WORD = 0xA5A5A5A5
CS_EMPTY_WORD = 0xFFFFFFFF
CS_STOP_WORD = 0x00000000
CS_REG_START = 0x50000000
CS_REG_END = 0x50060500
# Sticky Bits format - 0xC000XXXX up to 0xC00007FF
STICKY_BITS_MIN = 0xC0000000
STICKY_BITS_MAX = 0xC00007FF
# Key index area addresses
AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS_INDEX = AREA_CS_END
AREA_INDEX_SIZE = 0x20
AREA_ADDRESS_SIGNATURE_KEYS_INDEX = \
    (AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS_INDEX + AREA_INDEX_SIZE)
# Script steps and statuses
STEP_CONFIG_FILE_CHECK = 'Checking configuration and product keys files'
STEP_CHECK_PROD_ID = 'Checking product ID'
STEP_KEY_VERIFICATION = 'Product keys verification'
STEP_EFLASH_CHECK = 'Checking eFlash memory emptiness (keys area)'
STEP_KEY_WRITING = 'Writing keys to the eFlash memory'
STEP_KEY_MATCHING = 'Matching the programmed eFlash keys with the ones in product keys file'
STEP_READ_REV_KEYS = 'Reading revocation info from eFlash'
STEP_CS_WRITE = 'Enabling secure boot feature in configuration script'
STATUS_PASS = 'PASS'
STATUS_NOT_RUN = 'NOT RUN'
STATUS_FAIL = 'FAIL'


def __check_keys_group(group):
    if not group:
        return False

    if len(group) < 1 or len(group) > 8:
        return False

    for key in group:
        if hasattr(key, 'public'):
            key = key.public
        if len(key) != 64:
            return False

    return True


# Create the list of address-value pairs
def __parse_otp_read_output(output, start):
    data = []

    data_started = False
    for line in output.splitlines():
        if line.find("Using serial port") >= 0:
            continue
        if "{:04x}".format(start).upper() in line.upper():
            data_started = True
        elif data_started and 'done.' in line:
            break

        if not data_started:
            continue

        # Format: address b3 b2 b1 b0 ....
        address = int('0x{}'.format(line.split()[0].upper()), 0)
        word = int('0x{}'.format(''.join(reversed(line.split()[1:5])).upper()), 0)

        data.append((address, word))
    return data


def write_product_keys(cli_programmer, serial, product_keys, report):
    # Keep 32-bits words addressing (shift to the left by 2 is an equivalent of dividing by 4)
    AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS = 0x00000800
    AREA_ADDRESS_SIGNATURE_KEYS = 0x00000C00
    EMPTY_PLACEHOLDER = 'f' * 64

    # Initialize with mandatory key groups
    key_groups = {AREA_ADDRESS_SIGNATURE_KEYS : product_keys.asymmetric_keys}
    key_group_from_otp = {AREA_ADDRESS_SIGNATURE_KEYS: []}

    # Check product keys which should be written. User data encryption keys are optional. The rest
    # of the groups should contain at least one 256-bits key, but not more than 8 keys.
    for group in key_groups.values():
        if not __check_keys_group(group):
            report.update({STEP_KEY_VERIFICATION: STATUS_FAIL})
            return report

    report.update({STEP_KEY_VERIFICATION: STATUS_PASS})

    if __check_keys_group(product_keys.symmetric_keys):
        key_groups.update({AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS : product_keys.symmetric_keys})
        key_group_from_otp.update({AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS : []})

    # Read all key sections from eFlash memory
    for area, group in key_groups.items():
        size = len(group) * 32
        output = cli_programmer.read_otp((area >> 2), (size >> 2), serial_port=serial)
        # Skip address information - they are not needed
        words = [data[1] for data in __parse_otp_read_output(output, (area >> 2))]

        for i in range(0, size >> 2, 8):
            key_group_from_otp[area].append(''.join('{:08x}'.format(w) for w in words[i: i + 8]))

    # Check eFlash areas content
    for area, group in key_group_from_otp.items():
        for idx, otp_key in enumerate(group):
            if otp_key.lower() != EMPTY_PLACEHOLDER:
                report.update({STEP_EFLASH_CHECK: STATUS_FAIL})

                key = key_groups[area][idx]
                if hasattr(key, 'public'):
                    key = key.public

                if otp_key.lower() != key.lower():
                    report.update({STEP_KEY_MATCHING: STATUS_FAIL})

    if report[STEP_EFLASH_CHECK] == STATUS_NOT_RUN:
        report.update({STEP_EFLASH_CHECK: STATUS_PASS})
    elif report[STEP_KEY_MATCHING] == STATUS_NOT_RUN:
        report.update({STEP_KEY_MATCHING: STATUS_PASS})

    if report[STEP_EFLASH_CHECK] != STATUS_PASS:
        return report

    # Write keys to specified areas
    for area, group in key_groups.items():
        words = []

        for key in group:
            if hasattr(key, 'public'):
                key = key.public

            for i in range(0, len(key), 8):
                words.append('0x{}'.format(key[i : i + 8]))

        # Use one write per each area - should speedup keys writing
        output = cli_programmer.write_otp((area >> 2), len(words), *words, serial_port=serial)
        if 'failed' in output:
            report.update({STEP_KEY_WRITING: STATUS_FAIL})
            return report

    report.update({STEP_KEY_WRITING: STATUS_PASS})
    return report


def __read_keys_revocation_status(cli_programmer, serial):
    CURRENT_MARKER = int('0x{}'.format('f' * 8), 0)
    revoked = {AREA_ADDRESS_SIGNATURE_KEYS_INDEX: [],
               AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS_INDEX: []}

    for area, rev in revoked.items():
        output = cli_programmer.read_otp((area >> 2), (AREA_INDEX_SIZE >> 2), serial_port=serial)
        data = __parse_otp_read_output(output, (area >> 2))
        index = 0

        for address, word in data:
            if word != CURRENT_MARKER:
                rev.append(index)
            index += 1

    return revoked


def __read_otp_cs(cli_programmer, serial):
    cs_data = {'START_ADDR': None,                   # CS START = 0xA5A5A5A5
               'STICKY_BITS_VAL': None,              # Check masks to indetify bits
               'EMPTY_ADDR': None,                   # EMPTY WORD = 0xFFFFFFFF
               'STOP_ADDR': None                     # STOP WORD = 0x00000000
               }

    output = cli_programmer.read_otp((AREA_CS_START >> 2), (AREA_CS_SIZE >> 2), serial_port=serial)
    data = __parse_otp_read_output(output, (AREA_CS_START >> 2))

    # Get information from parsed data
    idx = 0
    while idx < (len(data) - 1):
        address, word = data[idx]
        next_address, next_word = data[idx + 1]

        if word == CS_START_WORD:
            cs_data['START_ADDR'] = address
        elif ((word & STICKY_BITS_MIN) == STICKY_BITS_MIN) and \
            (word <= STICKY_BITS_MAX):
            if cs_data['STICKY_BITS_VAL'] is None:
                cs_data['STICKY_BITS_VAL'] = 0
            cs_data['STICKY_BITS_VAL'] |= word
        elif word == CS_EMPTY_WORD and next_word == CS_EMPTY_WORD:
            # Two 32-bits words is required for setting a register value in CS. CS won't be parsed
            # by ROM bootloader after this address
            cs_data['EMPTY_ADDR'] = address
            break
        elif word == CS_STOP_WORD:
            # CS won't be parsed by ROM bootloader after this address
            cs_data['STOP_ADDR'] = address
            break

        if CS_REG_START <= word <= CS_REG_END:
            # Setting a register value takes 2 word in CS
            idx += 2
            continue

        idx += 1

    return cs_data


def __print_report(report):
    ui.print_header('Script execution report')
    for info, status in report.items():
        details = []

        if isinstance(status, list):
            details = status[1:]
            status = status[0]

        ui.print_message('{}... {}'.format(info, status), ignore_verbose=True)
        for d in details:
            ui.print_message('\t{}'.format(d), ignore_verbose=True)


def write_configuration_script(enable_sticky_bits, sticky_bits, min_ver_str, cli_programmer, serial):
    cs_data = __read_otp_cs(cli_programmer, serial)

    if cs_data['START_ADDR'] != AREA_CS_START and cs_data['EMPTY_ADDR'] != AREA_CS_START:
        return [STATUS_NOT_RUN, 'Invalid CS in eFlash.']

    if cs_data['EMPTY_ADDR'] is None:
        return [STATUS_NOT_RUN, 'Configuration script cannot be updated.']

    if cs_data['STOP_ADDR'] is not None and (cs_data['EMPTY_ADDR'] > cs_data['STOP_ADDR']):
        return [STATUS_NOT_RUN, 'Configuration script cannot be updated.']

    sticky_bits_old = cs_data['STICKY_BITS_VAL'] if cs_data['STICKY_BITS_VAL'] is not None else 0

    # CS header if there is none
    words = [hex(CS_START_WORD), hex(0x60000000 | PRODUCT_HEADER_PRIMARY_ADDRESS)] if cs_data['EMPTY_ADDR'] == AREA_CS_START else []

    # Sticky bits - including Secure Boot
    if (enable_sticky_bits and (sticky_bits_old != sticky_bits)):
        ui.print_message('Sticky bits enabled:')
        # Display ALL bits that are turned on
        sticky_bits |= sticky_bits_old
        if sticky_bits & sb.PROT_INFO_PAGE_Msk > 0: ui.print_message('PROT_INFO_PAGE')
        if sticky_bits & sb.DISABLE_AUDIO_CODEC_ACC_Msk > 0: ui.print_message('DISABLE_AUDIO_CODEC_ACC')
        if sticky_bits & sb.DISABLE_QSPI_CONTROLLER_Msk > 0: ui.print_message('DISABLE_QSPI_CONTROLLER')
        if sticky_bits & sb.SECURE_BOOT_Msk > 0: ui.print_message('SECURE_BOOT')
        if sticky_bits & sb.FORCE_CMAC_DEBUGGER_OFF_Msk > 0: ui.print_message('FORCE_CMAC_DEBUGGER_OFF')
        if sticky_bits & sb.FORCE_M33_DEBUGGER_OFF_Msk > 0: ui.print_message('FORCE_M33_DEBUGGER_OFF')
        if sticky_bits & sb.ENABLE_DCDC_Msk > 0: ui.print_message('ENABLE_DCDC')
        if sticky_bits & sb.PROT_USER_APP_CODE_Msk > 0: ui.print_message('PROT_USER_APP_CODE')
        if sticky_bits & sb.PROT_VALID_KEY_Msk > 0: ui.print_message('PROT_VALID_KEY')
        if sticky_bits & sb.PROT_APP_KEY_Msk > 0: ui.print_message('PROT_APP_KEY')
        if sticky_bits & sb.PROT_CONFIG_SCRIPT_Msk > 0: ui.print_message('PROT_CONFIG_SCRIPT')

        sticky_bits_config = 0xC0000000 | sticky_bits
        # Needed twice for redundancy.
        words.append(hex(sticky_bits_config))
        words.append(hex(sticky_bits_config))
        # TCS values for 64Kb Flash region size
        words.append(hex(0x1A0C0044))
        words.append(hex(0x00A00007))

    if len(words):
        ui.print_message('Writing to CS area.')
        cli_programmer.write_otp(hex(cs_data['EMPTY_ADDR']), len(words), *words, serial_port=serial)

    return STATUS_PASS


def secure_keys_prog(secure_cfg_file, keys_file, cli_programmer, serial=None):
    # Report contains all steps in the order of their execution
    report = OrderedDict([(STEP_CONFIG_FILE_CHECK, STATUS_NOT_RUN),
                          (STEP_CHECK_PROD_ID, STATUS_NOT_RUN),
                          (STEP_KEY_VERIFICATION, STATUS_NOT_RUN),
                          (STEP_EFLASH_CHECK, STATUS_NOT_RUN),
                          (STEP_KEY_MATCHING, STATUS_NOT_RUN),
                          (STEP_KEY_WRITING, STATUS_NOT_RUN),
                          (STEP_READ_REV_KEYS, STATUS_NOT_RUN),
                          (STEP_CS_WRITE, STATUS_NOT_RUN)])

    if secure_cfg_file is None:
        secure_cfg_file = DEFAULT_SECURE_CFG_FILE

    if keys_file is None:
        keys_file = DEFAULT_PRODUCT_KEYS_FILE

    if not os.path.exists(secure_cfg_file) or not os.path.exists(keys_file):
        secure_keys_cfg(configuration_file=secure_cfg_file, keys_file=keys_file,
                        prod_id=ProductId.DA1459x_00)

    secure_cfg = SecurityConfig(secure_cfg_file)
    product_keys = ProductKeys(keys_file)

    if not secure_cfg.is_valid():
        msg = 'Secure configuration is not valid.'
        report[STEP_CONFIG_FILE_CHECK] = [STATUS_FAIL, msg]
        __print_report(report)
        raise RuntimeError(msg)

    if not product_keys.is_valid(secure_cfg.product_id):
        msg = 'Product keys file is not valid.'
        report[STEP_CONFIG_FILE_CHECK] = [STATUS_FAIL, msg]
        __print_report(report)
        raise RuntimeError(msg)

    report[STEP_CONFIG_FILE_CHECK] = STATUS_PASS
    ui.print_message('Using product keys file: {}'.format(os.path.normpath(keys_file)))
    ui.print_message('Using secure configuration file: {}'.format(os.path.normpath(secure_cfg_file)))

    if secure_cfg.product_id != ProductId.DA1459x_00:
        report[STEP_CHECK_PROD_ID] = [STATUS_FAIL, 'Passed ID: {}'.format(secure_cfg.product_id)]
        __print_report(report)
        raise RuntimeError('This script could be run only for {} ({} selected)'.format(
            ProductId.DA1459x_00.value, secure_cfg.product_id))

    report[STEP_CHECK_PROD_ID] = STATUS_PASS
    ui.print_title('Programming product keys')
    write_product_keys(cli_programmer, serial, product_keys, report)

    ui.print_title('Checking key revocation status')
    status = __read_keys_revocation_status(cli_programmer, serial)
    report[STEP_READ_REV_KEYS] = \
        [STATUS_PASS,
         'Revoked signature keys: {}'.format(status[AREA_ADDRESS_SIGNATURE_KEYS_INDEX]),
         'Revoked user data keys: {}'.format(status[AREA_ADDRESS_USER_DATA_ENCRYPTION_KEYS_INDEX])
         ]

    # Sticky bits partition
    sticky_bits = 0
    enable_sticky_bits = secure_cfg.enable_sticky_bits
    if enable_sticky_bits is True:
        if secure_cfg.cs_prot_info_page is True:
            sticky_bits |= sb.PROT_INFO_PAGE_Msk
        if secure_cfg.cs_disable_audio_codec_acc is True:
            sticky_bits |= sb.DISABLE_AUDIO_CODEC_ACC_Msk
        if secure_cfg.cs_disable_qspi_controller is True:
            sticky_bits |= sb.DISABLE_QSPI_CONTROLLER_Msk
        if secure_cfg.cs_enable_secure_boot is True:
            sticky_bits |= sb.SECURE_BOOT_Msk
        if secure_cfg.cs_force_cmac_debugger_off is True:
            sticky_bits |= sb.FORCE_CMAC_DEBUGGER_OFF_Msk
        if secure_cfg.cs_for_m33_debugger_off is True:
            sticky_bits |= sb.FORCE_M33_DEBUGGER_OFF_Msk
        if secure_cfg.cs_enable_dcdc is True:
            sticky_bits |= sb.ENABLE_DCDC_Msk
        if secure_cfg.cs_prot_user_app_code is True:
            sticky_bits |= sb.PROT_USER_APP_CODE_Msk
        if secure_cfg.cs_prot_valid_key is True:
            sticky_bits |= sb.PROT_VALID_KEY_Msk
        if secure_cfg.cs_prot_app_key is True:
            sticky_bits |= sb.PROT_APP_KEY_Msk
        if secure_cfg.cs_prot_config_script is True:
            sticky_bits |= sb.PROT_CONFIG_SCRIPT_Msk

    if report[STEP_KEY_VERIFICATION] != STATUS_PASS:
        report[STEP_CS_WRITE] = [STATUS_NOT_RUN, 'Product keys are not valid.']
    elif report[STEP_KEY_MATCHING] == STATUS_FAIL:
        report[STEP_CS_WRITE] = [STATUS_NOT_RUN, 'Product keys from file do not match to the '
                                                 'keys read from eFlash.']
    elif report[STEP_KEY_WRITING] == STATUS_FAIL:
        report[STEP_CS_WRITE] = [STATUS_NOT_RUN, 'Product keys writing failed.']
    else:
        if enable_sticky_bits:
            ui.print_title('Enable sticky bits in Configuration Script')
        report[STEP_CS_WRITE] = write_configuration_script(enable_sticky_bits, sticky_bits, \
            secure_cfg.adm_minimal_version, cli_programmer, serial)

    __print_report(report)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    parser.add_argument('--secure_cfg', metavar='<file_path>', dest='secure_cfg_file',
                        help='path to DA1459x-00 secure configuration file')
    parser.add_argument('--keys', metavar='<file_path>', dest='keys_file',
                        help='path to DA1459x-00 keys file')
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return args.secure_cfg_file, args.keys_file, cli, serial


if __name__ == '__main__':
    ui.print_header('PROGRAM PRODUCT KEYS AND CS (eFlash)')
    run_script(secure_keys_prog, parse_args)
    ui.print_footer('FINISHED')
