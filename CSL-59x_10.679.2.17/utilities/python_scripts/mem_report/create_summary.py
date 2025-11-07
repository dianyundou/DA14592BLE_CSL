#########################################################################################
# Copyright (C) 2016-2023 Renesas Electronics Corporation and/or its affiliates.
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

import csv
import errno
import sys
import os
import map_reader
import xlxcreator
from collections import OrderedDict


def get_prj_names(full_path):
    build_conf = os.path.basename(os.path.dirname(full_path))
    project_name = os.path.splitext(os.path.basename(full_path))[0]
    return project_name, build_conf


def adjust_maximum_name_size(current_maximum_name_size, name):
    return max(current_maximum_name_size, len(name))


def get_summary_sizes(map_file):
    section_data = map_reader.generate_report(map_file)

    build_type = map_reader.get_build_type(map_file)

    otp_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                    # DA1468x layout
                    if 0x7fc0000 > int(section_data[sec]["address"], 16) >= 0x7f80000 or
                    # DA1469x layout
                    0x10090000 > int(section_data[sec]["address"], 16) >= 0x10080000])
    if build_type is None:
        ram_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                        # DA1468x layout
                        if 0x7fe0000 > int(section_data[sec]["address"], 16) >= 0x7fc0000 or
                        # DA1469x and DA1459x layout
                        0x20080000 > int(section_data[sec]["address"], 16) >= 0x20000000])
    elif build_type.lower() =="ram":
        ram_size = sum([int(section_data[sec]["size"], 16) for sec in section_data])
    else:
        ram_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                         # DA1470x layout
                         if 0x0F002000 > int(section_data[sec]["address"], 16) >= 0x0F000000 or
                         0x10050000 > int(section_data[sec]["address"], 16) >= 0x10000000 or
                         0x20183000 > int(section_data[sec]["address"], 16) >= 0x20000000
                         ])
    if build_type is None:
        flash_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                            # DA1468x layout
                            if 0xBF00000 > int(section_data[sec]["address"], 16) >= 0x8000000 or
                            # DA1469x and DA1459x layout, code remapped to 0
                            0x2000000 > int(section_data[sec]["address"], 16) >= 0x00])
        # Add .retention_ram_init and .non_retention_ram_init to ROM
        if '.retention_ram_init' in [sec for sec in section_data]:
            flash_size += int(section_data[".retention_ram_init"]["size"],16)
        if '.non_retention_ram_init' in [sec for sec in section_data]:
            flash_size += int(section_data[".non_retention_ram_init"]["size"],16)
        flash_size += map_reader.read_capacity_of_cs_prod_sections(map_file)
    else:
        flash_size = sum([int(section_data[sec]["size"], 16) for sec in section_data]) - ram_size

    cache_ram_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                          # DA1468x layout
                          if 0x8000000 > int(section_data[sec]["address"], 16) >= 0x7fe0000 or
                          # DA1469x layout
                          0x30070000 > int(section_data[sec]["address"], 16) >= 0x30060000 or
                          # DA1459x layout
                          0x36002000 > int(section_data[sec]["address"], 16) >= 0x36000000])
    retained_size = sum([int(section_data[sec]["size"], 16) for sec in section_data
                         # DA1468x layout
                         if "RETENTION" in sec or
                         # DA1469x/DA1470x layout
                         ".retention" in sec or ".stack" in sec])

    if not build_type:
        # Reassign done calculation if RAM target
        for key in section_data:
            # find stack segment
            if "stack" in key:
                # stack segment located within SYSRAM region for DA1469x, then it means the RAM target
                if 0 < int(section_data[key]["address"], 16) < 0x80000:
                    # calculation made for FLASH size above acctually deals with RAM target where
                    # all code + data lands in SYSRAM
                    ram_size = flash_size
                    flash_size = 0
                    break

    if os.path.isfile(map_file[:-4] + '.bin'):
        bin_size = os.stat(map_file[:-4] + '.bin').st_size
    else:
        print("WARNING: " + map_file[:-4] + '.bin' + " does not exist")
        bin_size = 0
    return otp_size, flash_size, ram_size, cache_ram_size, retained_size, bin_size


def main():
    search_path = os.path.abspath(sys.argv[1])
    report_name = sys.argv[2]

    # Generate CSV Folder
    csv_folder = os.path.abspath(os.path.splitext(os.path.basename(report_name))[0])
    csv_data = []

    if not os.path.exists(csv_folder):
        try:
            os.makedirs(csv_folder)
        except OSError as exc:  # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise

    # Start Excel file creation
    E = xlxcreator.XLwithXlswriter(report_name)

    # Create Main worksheet
    E.create_sheet("Projects Summary")

    # Write main title
    title_data = [["Project", "Configuration", "OTP size", "FLASH size", "RAM size",
                   "Cache RAM size", "Retained RAM size", "Bin file size"]]

    csv_header = title_data[0][2:]

    E.write_worksheet("Projects Summary", 0, 0, title_data, "main row title")

    maximum_prj_name_size = len(title_data[0][0])
    maximum_cfg_name_size = len(title_data[0][1])

    # Write data
    active_row = 1
    for root, dirs, files in os.walk(search_path):
        for f in files:
            if f.endswith(".map"):
                project, configuration = get_prj_names(os.path.join(root, f))
                maximum_prj_name_size = adjust_maximum_name_size(maximum_prj_name_size, project)
                maximum_cfg_name_size = adjust_maximum_name_size(maximum_cfg_name_size,
                                                                 configuration)
                E.write_worksheet("Projects Summary", active_row, 0, [[project]],
                                  "main column title")
                E.write_worksheet("Projects Summary", active_row, 1, [[configuration]],
                                  "main column title")
                otp, flash, ram, cram, ret, bins = get_summary_sizes(os.path.join(root, f))
                E.write_worksheet("Projects Summary", active_row, 2,
                                  [[otp, flash, ram, cram, ret, bins]], "data")
                active_row += 1
                # Create the CSV data
                csv_data.append((os.path.join(csv_folder, "{}.csv".format('_'.join([project,
                                                                            configuration]))),
                    OrderedDict(zip(csv_header, [otp, flash, ram, cram, ret, bins]))))

    # Set column widths
    E.set_column("Projects Summary", 0, maximum_prj_name_size + 2)
    E.set_column("Projects Summary", 1, maximum_cfg_name_size + 2)
    E.set_column("Projects Summary", 2, len(title_data[0][2]) + 2)
    E.set_column("Projects Summary", 3, len(title_data[0][3]) + 2)
    E.set_column("Projects Summary", 4, len(title_data[0][4]) + 2)
    E.set_column("Projects Summary", 5, len(title_data[0][5]) + 2)
    E.set_column("Projects Summary", 6, len(title_data[0][6]) + 2)
    E.set_column("Projects Summary", 7, len(title_data[0][7]) + 2)

    # Add an auto-filter in project name column to allow filtering out some
    E.add_drop_down_selector("Projects Summary", 0, 0, active_row - 1, 0)

    # Add comments
    E.add_comment("Projects Summary", 0, 2,
                  "The sum of sizes of sections within the OTP memory region.")
    E.add_comment("Projects Summary", 0, 3,
                  "The sum of sizes of sections within the FLASH memory region.")
    E.add_comment("Projects Summary", 0, 4,
                  "The sum of sizes of sections within the RAM memory region.")
    E.add_comment("Projects Summary", 0, 5,
                  "The sum of sizes of sections within the Cache RAM memory region.")
    E.add_comment("Projects Summary", 0, 6,
                  "The sum of sizes of sections  that end up in retained RAM.\n\n"
                  "For the SDK projects this is the sum of the sections that have"
                  " \"RETENTION\" in their name.")
    E.add_comment("Projects Summary", 0, 7,
                  "The size of the .bin file. If the .bin is not found then the size is 0.")

    # Close Excel file
    E.close_workbook()

    # Generate CSV reports
    for filename, values in csv_data:
        with open(filename, 'w') as csv_fp:
            csv_writer = csv.DictWriter(csv_fp, dialect='excel', fieldnames=values.keys())
            csv_writer.writeheader()
            csv_writer.writerow(values)


if __name__ == '__main__':
    main()
