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

import enum
import os

from api import ui
from api.application import ApplicationError
from argparse import ArgumentParser, ArgumentTypeError
import sys
import re
from string import Template
from configparser import ConfigParser
from api.cli_programmer import CliProgrammer, kill_jlink_gdb_server
from api.jlink import JLinkExe, get_jlink_gdb
from api.utils import is_win, is_linux, normpath


class ProductId(enum.Enum):
    DA14681_01 = 'DA14681-01'
    DA14683_00 = 'DA14683-00'
    DA1469x_00 = 'DA1469x-00'
    DA1459x_00 = 'DA1459x-00'


class ExecutionCanceled(Exception):
    pass


def get_jlink_prod_id(product_id):
    if product_id in [ProductId.DA14681_01, ProductId.DA14683_00]:
        return 'Cortex-M0'

    if product_id in [ProductId.DA1469x_00, ProductId.DA1459x_00]:
        return 'Cortex-M33'

    # FIXME: Should return None, however it will crash JlinkGDBServer.
    #        This way chip will be automatically selected.
    return 'Cortex-M0'


def run_script(callback, argument_parser, suppress_errors=False):
    ui.set_suppress_errors(suppress_errors)
    try:
        callback(*argument_parser())
    except RuntimeError as e:
        ui.error(str(e))
    except ApplicationError as e:
        ui.error(str(e), details='call: ' + e.call)
    except ExecutionCanceled:
        ui.print_message('Script execution canceled by user.')
    finally:
        kill_jlink_gdb_server()


def select_jlink_serial(jlink_path):
    devices = JLinkExe(jlink_path).find_jlink_numbers()

    if not devices:
        raise RuntimeError('No devices found. Please connect at least one and retry.')

    if len(devices) == 1:
        return devices[0]

    device = ui.select_item(item_list=devices, text='Select jlink serial')

    if not device:
        ui.print_message("No valid device selected.\nExecution aborted.\n")
        raise ExecutionCanceled()

    return device


def select_serial_port():
    def find_serial_ports():
        serial_list = []

        if is_win():
            from winreg import OpenKey, HKEY_LOCAL_MACHINE, EnumValue

            def enumerate_values(key):
                i = 0
                while True:
                    try:
                        yield EnumValue(key, i)
                        i += 1
                    except WindowsError:
                        break

            with OpenKey(HKEY_LOCAL_MACHINE, 'HARDWARE\\DEVICEMAP\\SERIALCOMM') as key:
                for value in enumerate_values(key):
                    if re.match(r'COM\d+', value[1]):
                        serial_list.append(value[1])

        elif is_linux():
            for value in os.listdir('/dev/'):
                if re.match(r'tty(USB|ACM)\d+', value):
                    serial_list.append(os.path.join('/', 'dev', value))
        else:
            pass

        return serial_list

    serials = find_serial_ports()

    if not serials:
        raise RuntimeError('No devices found. Please connect at least one and retry.')

    if len(serials) == 1:
        return serials[0]

    device = ui.select_item(item_list=serials, text='Select serial port')

    if not device:
        ui.print_message("No valid device selected.\nExecution aborted.\n")
        raise ExecutionCanceled()

    return device


def prepare_cli_programmer_ini(cfg=None, prod_id=None, device_id=None, port=None, log=None,
                               target_reset_cmd=None, jlink_path=None):
    default_gdb_port = 2331

    default_cfg = 'cli_programmer.ini'
    default_jlink_log = 'jlink.log'

    gdb_server_section = 'gdb server'
    trc_section = 'target reset'

    def get_gdb_server_path(jlink_path, prod_id, device_id, port, swoport, telnetport, log):
        gdb_server_path_template = Template(
            '$gdb -if swd -device $prod_id -endian little -speed 4000 -select usb=$device_id -port '
            '$port -swoport $swoport -telnetport $telnetport -log $log'
        )

        substitutes = {
            'gdb': normpath(get_jlink_gdb(jlink_path)),
            'prod_id': get_jlink_prod_id(prod_id),
            'device_id': device_id,
            'port': str(port),
            'swoport': str(swoport),
            'telnetport': str(telnetport),
            'log': normpath(log)
        }

        gdb_server_path = gdb_server_path_template.substitute(substitutes)

        if not device_id:
            gdb_server_path = re.sub('-select usb=None *', '', gdb_server_path)

        return gdb_server_path

    if not device_id:
        device_id = select_jlink_serial(jlink_path)

    if not device_id:
        ui.print_message("No valid device selected.\nExecution aborted.\n")
        raise ExecutionCanceled()

    if cfg is None:
        cfg = default_cfg

    if not port:
        port = default_gdb_port

    if not log:
        log = default_jlink_log

    if not target_reset_cmd:
        target_reset_cmd = ''

    # Initialize cli_programmer.ini if doesn't exist
    CliProgrammer(cfg_path=cfg)

    config = ConfigParser()
    config.read(cfg)

    config.set(gdb_server_section, 'gdb_server_path',
               get_gdb_server_path(jlink_path=jlink_path, prod_id=prod_id, device_id=device_id,
                                   port=port, swoport=port + 1, telnetport=port + 2, log=log))
    config.set(gdb_server_section, 'port', str(port))

    trc_cmd = config.get(trc_section, 'target_reset_cmd')
    if len(trc_cmd) < 20:
        trc_cmd = target_reset_cmd
    if target_reset_cmd:
        if device_id:
            if '-selectemubysn' in trc_cmd:
                trc_cmd = re.sub(
                    '[ ]*-selectemubysn [0-9]*', ' -selectemubysn ' + device_id, trc_cmd)
            else:
                trc_cmd += ' -selectemubysn ' + device_id
        else:
            trc_cmd = re.sub("[ ]*-selectemubysn [0-9]*", "", trc_cmd)

    config.set(trc_section, 'target_reset_cmd', trc_cmd)

    with open(cfg, 'w+') as out_file:
        config.write(out_file)


class ScriptArgumentsParser(ArgumentParser):
    def __init__(self):
        super(ScriptArgumentsParser, self).__init__()
        self.add_argument('-q', '--quiet', action='store_false', help='reduce verbose')

    def add_revision_args(self, revisions=None):
        if revisions is None:
            revisions = [*ProductId]

        def parse_product_id(product_id_str):
            try:
                revision = ProductId(product_id_str)
            except ValueError:
                revision = None

            if revision not in revisions:
                raise ArgumentTypeError("invalid choice: {} (choose from {})".format(product_id_str,
                                                                                     [_.value for _ in revisions]))

            return revision

        return self.add_argument('--prod_id', '-p', metavar="PROD_ID", type=parse_product_id,
                                 help='device product id {}'.format([_.value for _ in revisions]))

    def add_interface_args(self):
        self.add_argument('--cfg', '-c', type=str, help='cli programmer configuration file')

        group = self.add_argument_group('Interface',
                                        "Mutually exclusive group, use only one. "
                                        "Combined with '-h/--help' flag will display interface "
                                        "specific parameters.")

        interface_group = group.add_mutually_exclusive_group()
        interface_group.add_argument('--serial', action='store_true',
                                     help="select serial interface")
        interface_group.add_argument('--jlink', action='store_true',
                                     help="select jlink interface (default)")

        args = \
            self.parse_known_args(filter(lambda arg: arg != '-h' and arg != '--help', sys.argv))[0]

        if args.serial is True:
            group = self.add_argument_group('Serial',
                                            "Group only available if --serial flag is enabled")
            group.add_argument('-port', '--serial_port', type=str, help='serial port')
            group.add_argument('--baud', type=int, help='baud rate')
            group.add_argument('--init_baud', type=int, help='initial baud rate')
            group.add_argument('--tx_port', type=int, help='TX port')
            group.add_argument('--tx_pin', type=int, help='TX pin')
            group.add_argument('--rx_port', type=int, help='RX port')
            group.add_argument('--rx_pin', type=int, help='RX pin')

        if args.jlink is True:
            group = self.add_argument_group('JLink',
                                            "Group only available if --jlink flag is enabled")
            group.add_argument('-jsn', '--jlink_sn', type=int,
                               help='JLink serial number')
            group.add_argument('--jlink_path', type=str, help='path to JLink tools')

    def add_config_args(self):
        self.add_argument('-hc', '--header_config', type=str,
                          help='path to custom header configuration xml file')
        self.add_argument('-fc', '--flash_config', type=str,
                          help='path to custom Flash options configuration xml file')

    @staticmethod
    def get_interface_from_args(args):
        if not args.serial:
            args.jlink = True

        cli = CliProgrammer(cfg_path=getattr(args, 'cfg', None),
                            tx_port=getattr(args, 'tx_port', None),
                            tx_pin=getattr(args, 'tx_pin', None),
                            rx_port=getattr(args, 'rx_port', None),
                            rx_pin=getattr(args, 'rx_pin', None),
                            baudrate=getattr(args, 'baud', None),
                            init_baudrate=getattr(args, 'init_baud', None))
        if args.jlink is True and getattr(args, 'cfg', None) is None:
            prepare_cli_programmer_ini(cli.get_cfg(), prod_id=getattr(args, 'prod_id', None),
                                       device_id=getattr(args, 'jlink_sn', None),
                                       jlink_path=getattr(args, 'jlink_path', None))

        serial = getattr(args, 'serial_port', None)
        if args.serial is True and not serial:
            serial = select_serial_port()

        return cli, serial

    def parse_args(self, args=None, namespace=None):
        args = super(ScriptArgumentsParser, self).parse_args()

        ui.set_verbose(args.quiet)
        return args
