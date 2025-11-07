#
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
#

from map_reader import classify_by_object_file


def test_sdk_module_classification():
    assert 'moduleA' == classify_by_object_file('./sdk/moduleA/foo.o')
    assert 'moduleB' == classify_by_object_file('./sdk/moduleB/foo/bar.o')
    assert 'application' == classify_by_object_file('./bar/moduleC/foo.o')


def test_sdk_startup_classification():
    assert 'startup' == classify_by_object_file('./startup/foo_bar.o')
    assert 'startup' == classify_by_object_file('./sdk/startup/foo_bar.o')
    assert 'startup' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                                'gcc/arm-none-eabi/4.9.3/armv6-m/crti.o')
    assert 'application' == classify_by_object_file('./foo_bar/startup/foo_bar.o')


def test_sdk_application_classification():
    assert 'application' == classify_by_object_file('./foo/bar/blah.o')
    assert 'application' == classify_by_object_file('./foo/bar/blah.a')
    assert 'application' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\projects\\target_apps\\'
                                                    'wrbl\\health_toolbox\\src\\lib_sf\\'
                                                    'libSF_Library.a(smartfusionahrs.o)')
    assert 'application' == classify_by_object_file('/home/user/projects/680/_WRBL/wrbl_68x_ref/projects/target_apps/'
                                                    'wrbl/health_toolbox/src/lib_sf/libSF_Library.a(smartfusionahrs.o)')


def test_lto_classification():
    assert 'unknown' == classify_by_object_file('/tmp/ccgbAMkv.ltrans2.ltrans.o')
    assert 'unknown' == classify_by_object_file(r'C:\Users\user\AppData\Local\Temp\ccl0fEDC.ltrans4.ltrans.o')


def test_sdk_other_classification():
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                              'gcc/arm-none-eabi/4.9.3/armv6-m/libgcc.a(_thumb1_case_sqi.o)')
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                              'gcc/arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/armv6-m/'
                                              'libg_nano.a(lib_a-memcpy.o)')
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/gcc'
                                              '/arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/armv6-m/'
                                              'libnosys.a(_exit.o)')
    assert 'other' == classify_by_object_file('c:/diasemi/smartsnippetsstudio/gcc/4_9-2015q1/bin/../lib/gcc/'
                                              'arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/'
                                              'armv6-m\libnosys.a(sbrk.o)')
    assert 'startup' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                                'gcc/arm-none-eabi/4.9.3/armv6-m/crtbegin.o')


def test_libble_stack_classification_linux():
    assert 'libble_stack_da14681_01' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                                'ble_stack/DA14681-01-Debug/libble_stack_da14681_01.a'
                                                                '(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                                'ble_stack/DA14683-00-Debug/libble_stack_da14683_00.a'
                                                                '(rom_patch.o)')


def test_libble_stack_classification_win():
    assert 'libble_stack_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                                'interfaces\\ble_stack\\DA14681-01-Release\\'
                                                                'libble_stack_da14681_01.a(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                                'interfaces\\ble_stack\\DA14683-00-Release\\'
                                                                'libble_stack_da14683_00.a(rom_patch.o)')
    assert 'libble_stack_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                                'sdk\\interfaces\\ble_stack\\DA14681-01-Release\\'
                                                                'libble_stack_da14681_01.a(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                                'sdk\\interfaces\\ble_stack\\DA14683-00-Release\\'
                                                                'libble_stack_da14683_00.a(rom_patch.o)')


def test_libusb_lib_classification_linux():
    assert 'libusb_lib_da14681_01' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                              'usb/DA14681-01-Release/'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14683_00' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                              'usb/DA14683-00-Release/'
                                                              'libusb_lib_da14683_00.a(USB_CDC.o)')

def test_libusb_lib_classification_win():
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                              'interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                              'interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk'
                                                              '\\interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                              'sdk\\interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')


def test_new_sdk_lib_classification():
    assert 'libfoo' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/foo/bar/libfoo.a(bar.o)')
    assert 'libfoo' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_ref\\sdk\\foo\\bar\\libfoo.a(bar.o)')
