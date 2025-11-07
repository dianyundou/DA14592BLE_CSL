#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2020 Renesas Electronics Corporation and/or its affiliates.
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
import sys
import tempfile
from string import Template

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api.jlink import JLinkExe
from qspi.program_qspi_config import ProgramQspiConfig, program_qspi_config
from api.script_base import run_script, ExecutionCanceled, ScriptArgumentsParser, \
    select_jlink_serial, ProductId


def make_reboot_script_file(script_file):

    reboot_script = \
        'r0\n' \
        'r1\n' \
        'exit\n'

    with open(script_file, 'w') as f:
        f.write(reboot_script)


def reboot_device(prod_id=None, jlink_number=None, jlink_path=None):
    jlink_exe = JLinkExe(jlink_path)

    if not prod_id:
        config = ProgramQspiConfig()

        if not config.is_valid():
            program_qspi_config()
            config.load()

        prod_id = config.product_id
    else:
        prod_id = ProductId(prod_id)

    if not prod_id:
        raise ExecutionCanceled()

    if not jlink_number:
        jlink_number = select_jlink_serial(jlink_path)

    if not jlink_number:
        raise ExecutionCanceled()

    if prod_id == ProductId.DA1469x_00 or prod_id == ProductId.DA1459x_00:
        device = 'Cortex-M33'
    elif prod_id == ProductId.DA14681_01 or prod_id == ProductId.DA14683_00:
        device = 'Cortex-M0'
    else:
        raise RuntimeError("Unknown product id: " + str(prod_id))

    try:
        script = tempfile.mktemp()
        make_reboot_script_file(script)

        jlink_exe.run(args=['-if', 'SWD', '-device', device, '-speed', 'adaptive',
                            '-SelectEmuBySN', jlink_number, '-CommandFile', script], silent=True)
    finally:
        os.remove(script)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_revision_args()
    parser.add_argument('jlink_number', nargs='?', type=str, help='JLink number')
    parser.add_argument('--jlink_path', type=str, help='path to JLink')
    args = parser.parse_args()

    return args.prod_id, args.jlink_number, args.jlink_path


if __name__ == '__main__':
    run_script(reboot_device, parse_args)
