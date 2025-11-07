#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2023 Renesas Electronics Corporation and/or its affiliates.
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
import tempfile


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.mkimage import Mkimage
from api.script_base import run_script, ScriptArgumentsParser
from suota.v11.mkimage import make_sw_version_file
from eflash.program_eflash_config import ProductId
from eflash.program_eflash import program_product_header, handle_eflash_config, PRODUCT_HEADER_PRIMARY_ADDRESS, \
    PRODUCT_HEADER_SIZE
from secure_image.secure_keys_cfg import SecurityConfig, secure_keys_cfg
from secure_image.generate_keys import ProductKeys

DEFAULT_PRODUCT_KEYS_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                         'product_keys.xml'))
DEFAULT_SECURE_CFG_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                       'secure_cfg.xml'))


def secure_img_prog(image_file, secure_cfg_file, keys_file, cli_programmer, \
    serial=None, header_config_path=None, flash_config_path=None, *args):
    if image_file is None or not os.path.exists(image_file):
        raise RuntimeError("Application binary file not passed or not exist")

    eflash_config = handle_eflash_config(header_config_path, flash_config_path)

    if eflash_config.product_id != ProductId.DA1459x_00:
        raise RuntimeError("This script could be run only for {} ({} selected)".format(
            ProductId.DA1459x_00.value, eflash_config.product_id))

    if secure_cfg_file is None:
        secure_cfg_file = DEFAULT_SECURE_CFG_FILE

    if keys_file is None:
        keys_file = DEFAULT_PRODUCT_KEYS_FILE

    if not os.path.exists(secure_cfg_file) or not os.path.exists(keys_file):
        secure_keys_cfg(configuration_file=secure_cfg_file, keys_file=keys_file,
                        prod_id=eflash_config.product_id)

    secure_cfg = SecurityConfig(secure_cfg_file)
    product_keys = ProductKeys(keys_file)

    if not secure_cfg.is_valid():
        raise RuntimeError("Secure configuration is not valid.")

    if not product_keys.is_valid(secure_cfg.product_id):
        raise RuntimeError('Product keys file is not valid.')

    ui.print_message('Using product keys file: {}'.format(os.path.normpath(keys_file)))
    ui.print_message('Using secure configuration file: {}'.format(os.path.normpath(secure_cfg_file)))

    ui.print_title("Erasing product header area")
    cli_programmer.erase_eflash(PRODUCT_HEADER_PRIMARY_ADDRESS, PRODUCT_HEADER_SIZE, serial_port=serial)

    ui.print_title("Programming image")

    version_file = tempfile.mktemp()
    make_sw_version_file(version_file)

    output_image = tempfile.mktemp()
    asym_key = product_keys.asymmetric_keys[secure_cfg.security_pub_key_idx]
    sym_key = None
    Mkimage().da1459x_secure(
        image_file, version_file, output_image,
        key=asym_key.private,
        key_id=secure_cfg.security_pub_key_idx,
        rev_string=secure_cfg.make_revocation_string(),
        fw_version = secure_cfg.current_version,
        min_ver=True if secure_cfg.adm_minimal_version else False,
        ver=secure_cfg.adm_minimal_version)
    cli_programmer.write_eflash(eflash_config.update_image_address, output_image,
                              serial_port=serial, silent=False)

    ui.print_title("Programming product header")
    ui.print_message("Using configuration:\n" + eflash_config.get_config_info())
    program_product_header(eflash_config, cli_programmer, serial)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    parser.add_argument('image_file', type=str, nargs='?', help='path to image')
    parser.add_argument('--secure_cfg', metavar='<file_path>', dest='secure_cfg_file',
                        help='path to DA1459x-00 secure configuration file')
    parser.add_argument('--keys', metavar='<file_path>', dest='keys_file',
                        help='path to DA1459x-00 keys file')
    parser.add_config_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return args.image_file, args.secure_cfg_file, args.keys_file, cli, \
        serial, args.header_config, args.flash_config


if __name__ == '__main__':
    ui.print_header('PROGRAM SECURE IMAGE (EFLASH)')
    run_script(secure_img_prog, parse_args)
    ui.print_footer('FINISHED')
