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

import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.script_base import run_script, ScriptArgumentsParser, ExecutionCanceled


def erase_qspi_jtag(cli_programmer=None, serial=None):
    if ui.ask(text='Are you sure you want to completely erase QSPI?') is False:
        raise ExecutionCanceled()

    cli_programmer.chip_erase_qspi(serial_port=serial, silent=False)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return cli, serial


if __name__ == '__main__':
    ui.print_header('ERASE QSPI')
    run_script(erase_qspi_jtag, parse_args)
    ui.print_footer('FINISHED')
