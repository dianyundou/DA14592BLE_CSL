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

import re
import sys
from .user_interface import UserInterface


DEFAULT_CONSOLE_WIDTH = 120


class Cli(UserInterface):
    def __init__(self, verbose=True, console_width=DEFAULT_CONSOLE_WIDTH):
        self.verbose = verbose
        self.console_width = console_width

    def set_verbose(self, verbose):
        self.verbose = verbose

    def print_title(self, text, ignore_verbose=False):
        if self.verbose or ignore_verbose:
            print('.' * self.console_width)
            print('..')
            print('.. ' + text)
            print('..')
            print('.' * self.console_width)

    def print_message(self, text, ignore_verbose=False):
        if self.verbose or ignore_verbose:
            for line in text.split('\n'):
                print('. ' + line)

    def print_header(self, text):
        self.print_title(text, ignore_verbose=True)
        print('.')

    def print_footer(self, text):
        print('.')
        self.print_title(text, ignore_verbose=True)

    def info(self, text):
        print('.')
        self.print_message('INFO:\n' + text, ignore_verbose=True)
        print('.')

    def error(self, text='', details=''):
        text = text if text else 'Unknown error'
        print('.')
        print('.' * self.console_width)
        self.print_message('ERROR:\n' + text, ignore_verbose=True)
        if details:
            self.print_message('\n' + details, ignore_verbose=True)
        print('.' * self.console_width)
        print('.')

    def ask(self, text, confirmation='Yes', denial='No'):
        while True:
            print('.')
            self.print_message(text, ignore_verbose=True)
            if confirmation[0] == denial[0]:
                sys.stdout.write('. [{}/{}]: '.format(confirmation, denial))
            else:
                sys.stdout.write('. [({}){}/({}){}]: '.format(confirmation[0], confirmation[1:],
                                                              denial[0], denial[1:]))

            response = input().strip()

            if not response:
                print('. Input value can not be empty. Try Again')
                continue

            if confirmation[0] != denial[0] and len(response) == 1:
                response = response.upper()
                if response == confirmation[:1].upper():
                    return True
                elif response == denial[:1].upper():
                    return False

            if response == confirmation:
                return True
            elif response == denial:
                return False
            else:
                print('. Invalid value. Try Again')

    def ask_value(self, value_name, text='Insert value', default='', regex=None):
        while True:
            print('.')
            self.print_title(text + ' (default: {})'.format(default) if default else text,
                             ignore_verbose=True)

            sys.stdout.write('. ' + value_name + ': ')
            response = input().strip()

            if not response:
                if default != '':
                    return default
                print('. Input value can not be empty. Try Again')
                continue

            if regex and not re.fullmatch(regex, response):
                print("Value does not match format: '{}'".format(regex))
                continue

            return response

    def select_item(self, item_list, text='Select item', default=None):
        while True:
            print('.')
            self.print_title(text, ignore_verbose=True)
            for idx, option in enumerate(item_list):
                print('. {}: {}{}'.format(idx, str(option),
                                          ' (default)' if default is not None and default == idx
                                          else ''))

            sys.stdout.write('. Selected: ')
            response = input().strip()

            if not response:
                if default is not None:
                    return item_list[default]
                else:
                    print('. Input value can not be empty. Try Again')
                    continue

            try:
                return item_list[int(response, 0)]
            except ValueError:
                print('. Entered value must be number. Try Again')
            except IndexError:
                print('. Input value must be within list range. Try Again')

    def fill_checkbox_list(self, values_dict, text='Fill checkbox list'):
        ret_val = list(map(list, values_dict.items()))
        while True:
            print('.')
            self.print_title(text, ignore_verbose=True)
            print('.')
            for idx, (k, v) in enumerate(ret_val):
                print('. {}: [{}] {}'.format(idx, 'X' if v else ' ', k))

            print('.')
            print('. X: Exit')

            print('.')
            sys.stdout.write('. Selected: ')
            response = input().strip()

            if not response:
                print('. Input value can not be empty. Try Again')
                continue
            elif response == 'X':
                return dict(ret_val)

            try:
                print(ret_val[int(response, 0)])
                ret_val[int(response, 0)][1] = not ret_val[int(response, 0)][1]
            except ValueError:
                print(". Entered value must be number of 'X'. Try Again")
            except IndexError:
                print('. Input value must be within list range. Try Again')
