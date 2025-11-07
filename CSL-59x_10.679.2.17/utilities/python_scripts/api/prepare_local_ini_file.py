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
from api.script_base import run_script, ScriptArgumentsParser, prepare_cli_programmer_ini, ProductId


def prepare_local_ini_file(cfg=None, prod_id=None, device_id=None, port=None, log=None,
                           target_reset_cmd=None, jlink_path=None):
    prepare_cli_programmer_ini(cfg=cfg, prod_id=prod_id, device_id=device_id, port=port, log=log,
                               target_reset_cmd=target_reset_cmd, jlink_path=jlink_path)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('--cfg', type=str, help='config path')
    parser.add_argument('--id', type=str, help='device serial number')
    parser.add_revision_args()
    parser.add_argument('--port', type=int, help='GDB port')
    parser.add_argument('--log', type=str, help='JLink log')
    parser.add_argument('--trc', type=str,
                        help='target reset command')
    parser.add_argument('--jlink_path', type=str, help='JLink path')
    args = parser.parse_args()

    return args.cfg, args.prod_id, args.id, args.port, args.log, args.trc, args.jlink_path


if __name__ == '__main__':
    run_script(prepare_local_ini_file, parse_args)
