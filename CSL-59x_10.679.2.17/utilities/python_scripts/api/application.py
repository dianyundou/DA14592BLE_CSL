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
import subprocess

from api.utils import normpath


class ApplicationError(Exception):
    def __init__(self, message, call='', stderr='', return_code=None):
        self.call = call
        self.stderr = stderr
        self.return_code = return_code
        super(ApplicationError, self).__init__(message)


class Application(object):
    def __init__(self, path):
        if not os.path.exists(path):
            raise ApplicationError(str(path) + ' does not exists.')
        if not os.path.isfile(path) or not os.access(path, os.X_OK):
            raise ApplicationError(str(path) + ' is not application file.')

        self.__path = normpath(os.path.abspath(path))

    def get_path(self):
        return self.__path

    def get_basename(self):
        return os.path.basename(self.__path)

    def run(self, args=None, cwd=None, silent=False):
        call = [self.__path] + args if args else [self.__path]
        process_params = {
            'args': ' '.join(call),
            'shell': True,
            'cwd': cwd,
            'stdout': subprocess.PIPE,
            'stderr': subprocess.PIPE,
        }

        sys.stdout.flush()
        pr = subprocess.Popen(**process_params)

        buf = b''
        for line in pr.stdout:
            if not silent:
                sys.stdout.buffer.write(line)
                sys.stdout.flush()
            buf += line

        pr.wait()

        if pr.returncode:
            err_out = pr.stderr.read().decode('utf-8')

            sys.stdout.write(err_out)

            err_msg = '{} has exited with code: {}'.format(os.path.basename(call[0]), pr.returncode)
            raise ApplicationError(err_msg, call=' '.join(call), stderr=err_out,
                                   return_code=pr.returncode)

        return buf.decode('utf-8')
