#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Renesas Electronics Corporation and/or its affiliates.
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
from qspi.program_qspi_config import ProductId
from qspi.program_qspi import MIN_FW_ADDRESS, make_product_header, handle_qspi_config

PADDING_VAL = b'\xFF'


def prepare_flash_image(bin_file, image_file=None, sw_version_file=None, header_config_path=None,
                        flash_config_path=None):
    qspi_config = handle_qspi_config(header_config_path, flash_config_path)

    if qspi_config.product_id != ProductId.DA1469x_00:
        raise RuntimeError("This script is not available for {} devices.".format(
            qspi_config.product_id))

    if qspi_config.update_image_address < MIN_FW_ADDRESS:
        raise RuntimeError("FW cannot be placed at address lower than 0x{:X} (0x{:X} "
                           "given)".format(MIN_FW_ADDRESS, qspi_config.update_image_address))

    if not os.path.exists(bin_file):
        raise RuntimeError("Application binary file path is invalid: {}".format(bin_file))

    if sw_version_file is not None and not os.path.exists(bin_file):
        raise RuntimeError("SW version file path is invalid: {}".format(sw_version_file))

    # Product header at 0 + copy of product header at 0x1000 = 0x2000 bytes
    product_header = make_product_header(qspi_config.active_image_address,
                                         qspi_config.update_image_address,
                                         qspi_config.flash_burstcmda_reg_value,
                                         qspi_config.flash_burstcmdb_reg_value,
                                         qspi_config.flash_write_config_command)
    image_data = product_header * 2
    padding_size = qspi_config.update_image_address - len(image_data)

    if padding_size < 0:
        # This shouldn't happen - PH + PH copy should have exactly 0x2000 bytes
        raise RuntimeError("Calculated offset is invalid: {}".format(padding_size))

    image_data += PADDING_VAL * padding_size

    try:
        app_mkimage_file = tempfile.mktemp()

        if sw_version_file is None:
            try:
                version_file = tempfile.mktemp()
                make_sw_version_file(version_file)
                Mkimage().da1469x(bin_file, version_file, app_mkimage_file)
            finally:
                os.remove(version_file)
        else:
            Mkimage().da1469x(bin_file, sw_version_file, app_mkimage_file)

        image_obj = open(app_mkimage_file, "rb")
        image_data += image_obj.read()
        image_obj.close()
    finally:
        os.remove(app_mkimage_file)

    if image_file is None:
        image_file = bin_file + ".flash_img"

    image_file = os.path.normpath(image_file)

    with open(image_file, "wb") as image_obj:
        image_obj.write(image_data)

    ui.print_message("Storing FLASH image in {}".format(image_file))


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('bin_file', type=str, help='path to app binary')
    parser.add_argument('--image', metavar='image_file', dest='image_file', type=str,
                        help='output file with FLASH image')
    parser.add_argument('--sw_version', metavar='sw_version_file', dest='sw_version_file',
                        type=str, help='version file used for binary')
    parser.add_config_args()
    args = parser.parse_args()

    return args.bin_file, args.image_file, args.sw_version_file, args.header_config, \
           args.flash_config


if __name__ == '__main__':
    ui.print_header('PREPARE FLASH IMAGE')
    run_script(prepare_flash_image, parse_args)
    ui.print_footer('FINISHED')
