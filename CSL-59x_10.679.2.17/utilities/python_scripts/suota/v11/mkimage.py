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

import argparse
import os
import re
import string
import sys
import time

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.script_base import run_script, ScriptArgumentsParser
from api.mkimage import Mkimage, MKIMAGE_BIN
from api.utils import SDK_ROOT
from secure_image.secure_keys_cfg import SecurityConfig
from secure_image.generate_keys import ProductKeys
from eflash.program_eflash_config import ProductId

DEFAULT_VERSION_FILE = 'sw_version.h'
DEFAULT_VERSION = '1.0.0.1'
SW_VERSION_PATTERN = re.compile(r'#define SW_VERSION ["](.*)["]')


def make_sw_version_file(version_file=DEFAULT_VERSION_FILE, version=DEFAULT_VERSION,
                         date=time.strftime('%Y-%m-%d %H:%M')):

    sw_version_template = string.Template(
        '#define SW_VERSION "$version"\n'
        '#define SW_VERSION_DATE "$date"\n'
        '#define SW_VERSION_STATUS "REPOSITORY VERSION"\n'
    )

    with open(version_file, 'w') as f:
        f.write(sw_version_template.substitute(version=version, date=date))


def mkimage(binary_file, image_file=None, prod_id=None, version_file=None, secure_cfg_file=None,
            keys_file=None):
    if not os.path.isfile(MKIMAGE_BIN):
        raise RuntimeError('{} not found, please build it'.format(os.path.basename(MKIMAGE_BIN)))

    if not os.path.exists(binary_file):
        raise RuntimeError('Binary file {} does not exist'.format(binary_file))

    if not version_file:
        version_file = DEFAULT_VERSION_FILE

    ui.print_message('Using SDK from {}'.format(SDK_ROOT))

    version = DEFAULT_VERSION
    if os.path.isfile(version_file):
        with open(version_file, 'r') as f:
            try:
                version = SW_VERSION_PATTERN.findall(f.read())[0]
            except IndexError:
                os.rename(f.name, f.name + '.err')

    if not os.path.exists(version_file):
        if secure_cfg_file:
            raise RuntimeError('Invalid version file: {}'.format(version_file))
        else:
            make_sw_version_file(version_file)

    if not image_file:
        file_name = binary_file.split('.bin')[0]
        image_file = '.'.join([file_name, version, 'img'])
        ui.print_message('No output image file specified, creating new file {}'.format(image_file))

    if secure_cfg_file and not keys_file:
        raise RuntimeError('Product keys file not passed - secure image cannot be created.')

    if prod_id == ProductId.DA1459x_00:
        if secure_cfg_file:
            ui.print_message('Creating secure image for {}.'.format(prod_id))

            secure_cfg = SecurityConfig(secure_cfg_file)
            keys = ProductKeys(keys_file)
            asym_key = keys.asymmetric_keys[secure_cfg.security_pub_key_idx]

            if not secure_cfg.is_valid():
                raise RuntimeError('Secure configuration is not valid.')

            if not keys.is_valid(secure_cfg.product_id):
                raise RuntimeError('Product key file is not valid.')

            return Mkimage().da1459x_secure(
                binary_file, version_file, image_file,
                key=asym_key.private,
                key_id=secure_cfg.security_pub_key_idx,
                rev_string=secure_cfg.make_revocation_string(),
                fw_version = secure_cfg.current_version,
                min_ver=True if secure_cfg.adm_minimal_version else False,
                ver=secure_cfg.adm_minimal_version)
        else:
            ui.print_message('Creating image for {}.'.format(prod_id))
            return Mkimage().da1459x(binary_file, version_file, image_file)
    elif prod_id == ProductId.DA1469x_00:
        if secure_cfg_file:
            ui.print_message('Creating secure image for {}.'.format(prod_id))
            secure_cfg = SecurityConfig(secure_cfg_file)
            keys = ProductKeys(keys_file)
            asym_key = keys.asymmetric_keys[secure_cfg.security_pub_key_idx]
            sym_key = keys.symmetric_fw_dec_keys[secure_cfg.security_sym_key_idx]

            if not secure_cfg.is_valid():
                raise RuntimeError('Secure configuration is not valid.')

            if not keys.is_valid(secure_cfg.product_id):
                raise RuntimeError('Product key file is not valid.')

            return Mkimage().da1469x_secure(binary_file, version_file, image_file,
                                          key=asym_key.private,
                                          key_id=secure_cfg.security_pub_key_idx,
                                          sym_key=sym_key,
                                          sym_key_idx=secure_cfg.security_sym_key_idx,
                                          nonce=secure_cfg.security_nonce,
                                          rev_string=secure_cfg.make_revocation_string())
        else:
            ui.print_message('Creating non secure image for DA1469x_00, {}.'.format(prod_id))
            return Mkimage().da1469x(binary_file, version_file, image_file)

    elif prod_id == ProductId.DA14681_01 or prod_id == ProductId.DA14683_00:
        if secure_cfg_file:
            ui.print_message('Creating secure image for {}.'.format(prod_id))
            secure_cfg = SecurityConfig(secure_cfg_file)
            keys = ProductKeys(keys_file)
            asym_key = keys.asymmetric_keys[secure_cfg.security_pub_key_idx]

            if not secure_cfg.is_valid():
                raise RuntimeError('Secure configuration is not valid.')

            if not keys_file.is_valid(secure_cfg.product_id):
                raise RuntimeError('Product key file is not valid.')

            return Mkimage().secure(binary_file, version_file, image_file,
                                    ec=asym_key.elliptic_curve,
                                    hash_type=secure_cfg.security_hash_method,
                                    key=asym_key.private,
                                    key_id=secure_cfg.security_pub_key_idx,
                                    rev_string=secure_cfg.make_revocation_string(),
                                    min_ver=True if secure_cfg.adm_minimal_version else False,
                                    ver=secure_cfg.adm_minimal_version)

        else:
            return Mkimage().single(binary_file, version_file, image_file)

    else:
        raise RuntimeError('No supported revision selected ().'.format(prod_id))


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('binary', metavar='bin_file', type=str,
                        help='file with application binary')
    parser.add_argument('image', metavar='image_file', type=str, nargs='?',
                        help='output file with image for SUOTA')
    parser.add_revision_args()
    parser.add_argument('--sw_version', metavar='sw_version_file', dest='sw_version_file',
                        type=str, help='version file used for binary')
    parser.add_argument('-s', '--sec_cfg', metavar='security_config', dest='security_config_file',
                        type=str, help='configuration file for secure image (forces secure image)')
    parser.add_argument('--prod_keys', metavar='product_keys', dest='keys_file', type=str,
                        help='keys file for secure image')
    args = parser.parse_args()
    return args.binary, args.image, args.prod_id, args.sw_version_file, args.security_config_file, \
           args.keys_file


if __name__ == '__main__':
    run_script(mkimage, parse_args, suppress_errors=True)
