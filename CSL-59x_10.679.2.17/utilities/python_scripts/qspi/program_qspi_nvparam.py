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

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.cli_programmer import CliProgrammer
from api.utils import SDK_ROOT
from api.script_base import run_script, ScriptArgumentsParser
from api.prepare_local_ini_file import prepare_local_ini_file
from qspi.program_qspi_config import ProgramQspiConfig, program_qspi_config, ProductId
from qspi.create_nvparam import create_nvparam


def program_qspi_nvparam(project_output_path, cfg=None, device_id=None, arm_gcc_path=None):
    config = ProgramQspiConfig()
    while not config.is_valid():
        program_qspi_config()
        config = ProgramQspiConfig()

    if not os.path.exists(project_output_path):
        raise RuntimeError("Build the desired project first")

    if config.product_id == ProductId.DA1459x_00:
        cli_programmer = CliProgrammer(cfg_path=cfg, jlink_id=device_id)
    else:
        cli_programmer = CliProgrammer(cfg_path=cfg, prod_id=config.product_id, jlink_id=device_id)

    if not cfg:
        prepare_local_ini_file(cfg=cli_programmer.get_cfg(), device_id=device_id)

    project_path = os.path.join(project_output_path, '..')

    create_nvparam(project_output_path, [project_path + "/config", SDK_ROOT + "/sdk/middleware/adapters/include"],
                   arm_gcc_path)

    nvparam_bin = os.path.join(project_output_path, 'nvparam.bin')
    cli_programmer.write_partition("NVMS_PARAM_PART", 0x00, nvparam_bin)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('--cfg', metavar='<cfg_path>', type=str, help='config path')
    parser.add_argument('--id', metavar='<serial_number>', type=str, help='device serial number')
    parser.add_argument('--arm_gcc_path', metavar='<arm_gcc_path>', type=str,
                        help='path to cross compiler')
    parser.add_argument('path', metavar='<project_output_path>', type=str,
                        help='folder which contains the binary you want to program')
    args = parser.parse_args()
    return args.path, args.cfg, args.id, args.arm_gcc_path


if __name__ == '__main__':
    ui.print_header('PROGRAM NVPARAM')
    run_script(program_qspi_nvparam, parse_args)
    ui.print_footer('FINISHED')
