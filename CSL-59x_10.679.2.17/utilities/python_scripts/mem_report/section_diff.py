#!/usr/bin/env python
#########################################################################################
# Copyright (C) 2019-2020 Renesas Electronics Corporation and/or its affiliates.
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
from collections import OrderedDict

import map_reader

ROW_WIDTH = 24


def diff_process(diff_dict, module_data, method):
    for sw_module in module_data:
        for object_file in module_data[sw_module]:
            for entry in module_data[sw_module][object_file]:
                if entry[0] not in diff_dict:
                    diff_dict[entry[0]] = OrderedDict()

                if entry[2] not in diff_dict[entry[0]]:
                    diff_dict[entry[0]][entry[2]] = 0

                diff_dict[entry[0]][entry[2]] = method(diff_dict[entry[0]][entry[2]], entry[1])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Compare map files')
    parser.add_argument('map_file_a')
    parser.add_argument('map_file_b')
    parser.add_argument("--verbose", "-v", help="increase output verbosity", action="store_true")
    args = parser.parse_args()

    if not os.path.isfile(args.map_file_a):
        raise Exception('File \'{}\' does not exist.'.format(args.map_file_a))

    if not os.path.isfile(args.map_file_b):
        raise Exception('File \'{}\' does not exist.'.format(args.map_file_b))

    section_data_a = map_reader.generate_report(args.map_file_a)
    section_data_b = map_reader.generate_report(args.map_file_b)

    _, module_data_a = map_reader.analyse_modules(section_data_a)
    _, module_data_b = map_reader.analyse_modules(section_data_b)

    diff = OrderedDict()
    diff_process(diff, module_data_b, lambda a, b: a + b)
    diff_process(diff, module_data_a, lambda a, b: a - b)

    if args.verbose:
        fmt = "{{:<{width}.{width}}} {{:<{width}.{width}}} {{:<{width}.{width}}}".format(
            width=ROW_WIDTH)

        print(fmt.format("SECTION", "SUBSECTION", "DIFF"))

        for section in diff:
            for subsection in diff[section]:
                if diff[section][subsection] != 0:
                    print(fmt.format(section, subsection, "{:+}".format(diff[section][subsection])))
    else:
        fmt = "{{:<{width}.{width}}} {{:<{width}.{width}}}".format(width=ROW_WIDTH)

        print(fmt.format("SECTION", "DIFF"))

        for section in diff:
            section_diff = sum([diff[section][subsection] for subsection in diff[section]])
            if section_diff is not 0:
                print(fmt.format(section, "{:+}".format(section_diff)))
