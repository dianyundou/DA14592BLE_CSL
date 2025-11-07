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
import xml.dom.minidom as xmldom
import xml.etree.ElementTree as ElemTree
from collections import OrderedDict

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api.script_base import run_script, ScriptArgumentsParser, ProductId, ExecutionCanceled
from api.mkimage import HashMethod
from api import ui
from secure_image.generate_keys import generate_keys, ProductKeys, DEFAULT_PRODUCT_KEYS_FILE, \
    ALLOWED_ELLIPTIC_CURVES
import secure_image.sticky_bits as sb

DEFAULT_CONFIGURATION_FILE = 'secure_cfg.xml'

ALLOWED_HASH_METHODS = [
    HashMethod.SHA224,
    HashMethod.SHA256,
    HashMethod.SHA384,
    HashMethod.SHA512,
]

DEFAULT_HASH_METHOD_IDX = ALLOWED_HASH_METHODS.index(HashMethod.SHA256)


class SecurityConfig:
    # Prefixes - used in key revocation string
    KEY_REV_SYM_USER_DATA_PREFIX = 's'
    KEY_REV_SYM_FW_DEC_PREFIX = 'd'
    # XML tags - common part
    PRODUCT_ID_ATTR = 'product_id'
    CURRENT_VERSION_TAG = 'current_version'
    SECURE_CONFIG_TAG = 'secure_cfg'
    SECURITY_TAG = 'security'
    PUB_KEY_IDX_TAG = 'public_key_idx'
    DEV_ADM_TAG = 'device_administration'
    KEY_REV_TAG = 'key_revocation'
    KEY_REV_PUB_TAG = 'public'
    KEY_REV_SYM_USER_DATA_TAG = 'user_data'
    KEY_REV_INDEX_TAG = 'key_index'
    # DA14683-00 specific
    HASH_METHOD_TAG = 'hash_method'
    MINIMAL_VERSION_TAG = 'minimal_version'
    # DA1469x-00 specific
    SYM_KEY_IDX_TAG = 'symmetric_key_idx'
    NONCE_TAG = 'nonce'
    KEY_REV_SYM_FW_DEC_TAG = 'fw_decryption'
    CS_ENABLE_SECURE_BOOT_TAG = 'cs_enable_secure_boot'
    # DA1459x-00 specific
    MINIMAL_VERSION_TAG = 'minimal_version'
    ENABLE_STICKY_BITS_TAG = 'enable_sticky_bits'
    CS_PROT_INFO_PAGE_TAG = 'cs_prot_info_page'
    CS_DISABLE_AUDIO_CODEC_ACC_TAG = 'cs_disable_audio_codec_acc'
    CS_DISABLE_QSPI_CONTROLLER_TAG = 'cs_disable_qspi_controller'
    CS_ENABLE_SECURE_BOOT_TAG = 'cs_enable_secure_boot'
    CS_FORCE_CMAC_DEBUGGER_OFF_TAG = 'cs_force_cmac_debugger_off'
    CS_FORCE_M33_DEBUGGER_OFF_TAG = 'cs_for_m33_debugger_off'
    CS_ENABLE_DCDC_TAG = 'cs_enable_dcdc'
    CS_PROT_USER_APP_CODE_TAG = 'cs_prot_user_app_code'
    CS_PROT_VALID_KEY_TAG = 'cs_prot_valid_key'
    CS_PROT_APP_KEY_TAG = 'cs_prot_app_key'
    CS_PROT_CONFIG_SCRIPT_TAG = 'cs_prot_config_script'

    def is_valid(self):
        # These values are mandatory for each platform
        if self.security_pub_key_idx is None:
            return False

        if self.product_id == ProductId.DA14683_00:
            return self.security_hash_method is not None
        elif self.product_id == ProductId.DA1469x_00:
            return self.security_sym_key_idx is not None
        elif self.product_id == ProductId.DA1459x_00:
            return True
        else:
            return False

    def __init__(self, configuration_file):
        self.__file = configuration_file

        self.product_id = None
        self.current_version = None
        self.cs_enable_secure_boot = None
        self.security_pub_key_idx = None
        self.security_hash_method = None
        self.security_sym_key_idx = None
        self.security_nonce = None

        self.enable_sticky_bits = False
        self.cs_prot_info_page = False
        self.cs_disable_audio_codec_acc = False
        self.cs_disable_qspi_controller = False
        self.cs_enable_secure_boot = False
        self.cs_force_cmac_debugger_off = False
        self.cs_for_m33_debugger_off = False
        self.cs_enable_dcdc = False
        self.cs_prot_user_app_code = False
        self.cs_prot_valid_key = False
        self.cs_prot_app_key = False
        self.cs_prot_config_script = False

        self.adm_minimal_version = None
        self.adm_key_revocations = {
            SecurityConfig.KEY_REV_PUB_TAG : [],
            SecurityConfig.KEY_REV_SYM_USER_DATA_TAG : [],
            SecurityConfig.KEY_REV_SYM_FW_DEC_TAG : []
        }

        try:
            root = ElemTree.parse(configuration_file).getroot()
        except (ElemTree.ParseError, FileNotFoundError):
            root = ElemTree.Element(SecurityConfig.SECURE_CONFIG_TAG)

        # Use field as Enum object instead of string
        try:
            self.product_id = ProductId(root.get(SecurityConfig.PRODUCT_ID_ATTR))
        except ValueError:
            self.product_id = None

        current_version = root.find(SecurityConfig.CURRENT_VERSION_TAG)
        if current_version is not None:
            self.current_version = int(current_version.text)

        cs_enable_secure_boot = root.find(SecurityConfig.CS_ENABLE_SECURE_BOOT_TAG)
        if cs_enable_secure_boot is not None:
            self.cs_enable_secure_boot = (cs_enable_secure_boot.text == 'True')

        security = root.find(SecurityConfig.SECURITY_TAG)
        if security:
            pub_key_idx = security.find(SecurityConfig.PUB_KEY_IDX_TAG)
            # Allow indexes (0-8) and addresses (e.g. 0x7F8E6C0 or 7F8E6C0) which are used only
            # by DA14683
            self.security_pub_key_idx = int(pub_key_idx.text, 16) \
                if pub_key_idx is not None else None

            hash_method = security.find(SecurityConfig.HASH_METHOD_TAG)
            self.security_hash_method = \
                HashMethod(hash_method.text) if hash_method is not None else None

            sym_key_idx = security.find(SecurityConfig.SYM_KEY_IDX_TAG)
            self.security_sym_key_idx = int(sym_key_idx.text) if sym_key_idx is not None else None

            nonce = security.find(SecurityConfig.NONCE_TAG)
            self.security_nonce = nonce.text if nonce is not None else None

        administration = root.find(SecurityConfig.DEV_ADM_TAG)
        if administration:
            min_version = administration.find(SecurityConfig.MINIMAL_VERSION_TAG)
            self.adm_minimal_version = min_version.text if min_version is not None else None

            key_revocation = administration.find(SecurityConfig.KEY_REV_TAG)
            for tag in [SecurityConfig.KEY_REV_PUB_TAG, SecurityConfig.KEY_REV_SYM_USER_DATA_TAG]:
                key_type = key_revocation.find(tag)

                self.adm_key_revocations[tag] = [] if not key_type else \
                    [k.text for k in key_type.findall(SecurityConfig.KEY_REV_INDEX_TAG)]
                self.adm_key_revocations[tag].sort()

        enable_sticky_bits = root.find(SecurityConfig.ENABLE_STICKY_BITS_TAG)
        if enable_sticky_bits:
            self.enable_sticky_bits = True
            cs_prot_info_page = enable_sticky_bits.find(SecurityConfig.CS_PROT_INFO_PAGE_TAG)
            if cs_prot_info_page is not None:
                self.cs_prot_info_page = (cs_prot_info_page.text == 'True')
            cs_disable_audio_codec_acc = enable_sticky_bits.find(SecurityConfig.CS_DISABLE_AUDIO_CODEC_ACC_TAG)
            if cs_disable_audio_codec_acc is not None:
                self.cs_disable_audio_codec_acc = (cs_disable_audio_codec_acc.text == 'True')
            cs_disable_qspi_controller = enable_sticky_bits.find(SecurityConfig.CS_DISABLE_QSPI_CONTROLLER_TAG)
            if cs_disable_qspi_controller is not None:
                self.cs_disable_qspi_controller = (cs_disable_qspi_controller.text == 'True')
            cs_enable_secure_boot = enable_sticky_bits.find(SecurityConfig.CS_ENABLE_SECURE_BOOT_TAG)
            if cs_enable_secure_boot is not None:
                self.cs_enable_secure_boot = (cs_enable_secure_boot.text == 'True')
            cs_force_cmac_debugger_off = enable_sticky_bits.find(SecurityConfig.CS_FORCE_CMAC_DEBUGGER_OFF_TAG)
            if cs_force_cmac_debugger_off is not None:
                self.cs_force_cmac_debugger_off = (cs_force_cmac_debugger_off.text == 'True')
            cs_for_m33_debugger_off = enable_sticky_bits.find(SecurityConfig.CS_FORCE_M33_DEBUGGER_OFF_TAG)
            if cs_for_m33_debugger_off is not None:
                self.cs_for_m33_debugger_off = (cs_for_m33_debugger_off.text == 'True')
            cs_enable_dcdc = enable_sticky_bits.find(SecurityConfig.CS_ENABLE_DCDC_TAG)
            if cs_enable_dcdc is not None:
                self.cs_enable_dcdc = (cs_enable_dcdc.text == 'True')
            cs_prot_user_app_code = enable_sticky_bits.find(SecurityConfig.CS_PROT_USER_APP_CODE_TAG)
            if cs_prot_user_app_code is not None:
                self.cs_prot_user_app_code = (cs_prot_user_app_code.text == 'True')
            cs_prot_valid_key = enable_sticky_bits.find(SecurityConfig.CS_PROT_VALID_KEY_TAG)
            if cs_prot_valid_key is not None:
                self.cs_prot_valid_key = (cs_prot_valid_key.text == 'True')
            cs_prot_app_key = enable_sticky_bits.find(SecurityConfig.CS_PROT_APP_KEY_TAG)
            if cs_prot_app_key is not None:
                self.cs_prot_app_key = (cs_prot_app_key.text == 'True')
            cs_prot_config_script = enable_sticky_bits.find(SecurityConfig.CS_PROT_CONFIG_SCRIPT_TAG)
            if cs_prot_config_script is not None:
                self.cs_prot_config_script = (cs_prot_config_script.text == 'True')

    def save(self):
        if not self.is_valid():
            raise ValueError('Configuration is incomplete')

        root = ElemTree.Element(SecurityConfig.SECURE_CONFIG_TAG)
        root.set(SecurityConfig.PRODUCT_ID_ATTR, self.product_id.value)

        current_version = ElemTree.Element(SecurityConfig.CURRENT_VERSION_TAG)
        current_version.text = str(self.current_version)
        root.append(current_version)

        if self.product_id == ProductId.DA1469x_00 and self.cs_enable_secure_boot is not None:
            cs_enable = ElemTree.Element(SecurityConfig.CS_ENABLE_SECURE_BOOT_TAG)
            cs_enable.text = str(self.cs_enable_secure_boot)
            root.append(cs_enable)

        if self.product_id == ProductId.DA1459x_00 and self.enable_sticky_bits is True:
            enable_sticky_bits = ElemTree.Element(SecurityConfig.ENABLE_STICKY_BITS_TAG)

            if self.cs_prot_info_page is True:
                cs_prot_info_page = ElemTree.Element(SecurityConfig.CS_PROT_INFO_PAGE_TAG)
                cs_prot_info_page.text = str(self.cs_prot_info_page)
                enable_sticky_bits.append(cs_prot_info_page)
            if self.cs_disable_audio_codec_acc is True:
                cs_disable_audio_codec_acc = ElemTree.Element(SecurityConfig.CS_DISABLE_AUDIO_CODEC_ACC_TAG)
                cs_disable_audio_codec_acc.text = str(self.cs_disable_audio_codec_acc)
                enable_sticky_bits.append(cs_disable_audio_codec_acc)
            if self.cs_disable_qspi_controller is True:
                cs_disable_qspi_controller = ElemTree.Element(SecurityConfig.CS_DISABLE_QSPI_CONTROLLER_TAG)
                cs_disable_qspi_controller.text = str(self.cs_disable_qspi_controller)
                enable_sticky_bits.append(cs_disable_qspi_controller)
            if self.cs_enable_secure_boot is True:
                cs_enable_secure_boot = ElemTree.Element(SecurityConfig.CS_ENABLE_SECURE_BOOT_TAG)
                cs_enable_secure_boot.text = str(self.cs_enable_secure_boot)
                enable_sticky_bits.append(cs_enable_secure_boot)
            if self.cs_force_cmac_debugger_off is True:
                cs_force_cmac_debugger_off = ElemTree.Element(SecurityConfig.CS_FORCE_CMAC_DEBUGGER_OFF_TAG)
                cs_force_cmac_debugger_off.text = str(self.cs_force_cmac_debugger_off)
                enable_sticky_bits.append(cs_force_cmac_debugger_off)
            if self.cs_for_m33_debugger_off is True:
                cs_for_m33_debugger_off = ElemTree.Element(SecurityConfig.CS_FORCE_M33_DEBUGGER_OFF_TAG)
                cs_for_m33_debugger_off.text = str(self.cs_for_m33_debugger_off)
                enable_sticky_bits.append(cs_for_m33_debugger_off)
            if self.cs_enable_dcdc is True:
                cs_enable_dcdc = ElemTree.Element(SecurityConfig.CS_ENABLE_DCDC_TAG)
                cs_enable_dcdc.text = str(self.cs_enable_dcdc)
                enable_sticky_bits.append(cs_enable_dcdc)
            if self.cs_prot_user_app_code is True:
                cs_prot_user_app_code = ElemTree.Element(SecurityConfig.CS_PROT_USER_APP_CODE_TAG)
                cs_prot_user_app_code.text = str(self.cs_prot_user_app_code)
                enable_sticky_bits.append(cs_prot_user_app_code)
            if self.cs_prot_valid_key is True:
                cs_prot_valid_key = ElemTree.Element(SecurityConfig.CS_PROT_VALID_KEY_TAG)
                cs_prot_valid_key.text = str(self.cs_prot_valid_key)
                enable_sticky_bits.append(cs_prot_valid_key)
            if self.cs_prot_app_key is True:
                cs_prot_app_key = ElemTree.Element(SecurityConfig.CS_PROT_APP_KEY_TAG)
                cs_prot_app_key.text = str(self.cs_prot_app_key)
                enable_sticky_bits.append(cs_prot_app_key)
            if self.cs_prot_config_script is True:
                cs_prot_config_script = ElemTree.Element(SecurityConfig.CS_PROT_CONFIG_SCRIPT_TAG)
                cs_prot_config_script.text = str(self.cs_prot_config_script)
                enable_sticky_bits.append(cs_prot_config_script)

            root.append(enable_sticky_bits)

        security = ElemTree.Element(SecurityConfig.SECURITY_TAG)

        pub_key_idx = ElemTree.Element(SecurityConfig.PUB_KEY_IDX_TAG)
        pub_key_idx.text = str(self.security_pub_key_idx)
        security.append(pub_key_idx)

        # Skip fields which are not necessary for specified device
        if self.product_id == ProductId.DA14683_00:
            if self.security_hash_method is not None:
                hash_method = ElemTree.Element(SecurityConfig.HASH_METHOD_TAG)
                hash_method.text = str(self.security_hash_method.value)
                #HASH_METHOD_TAG
                security.append(hash_method)
        elif self.product_id == ProductId.DA1469x_00:
            if self.security_sym_key_idx is not None:
                sym_key_idx = ElemTree.Element(SecurityConfig.SYM_KEY_IDX_TAG)
                sym_key_idx.text = str(self.security_sym_key_idx)
                security.append(sym_key_idx)

            if self.security_nonce is not None:
                nonce = ElemTree.Element(SecurityConfig.NONCE_TAG)
                nonce.text = str(self.security_nonce)
                security.append(nonce)

        root.append(security)

        administration = ElemTree.Element(SecurityConfig.DEV_ADM_TAG)

        if self.adm_minimal_version is not None and \
            (self.product_id == ProductId.DA14683_00 or self.product_id == ProductId.DA1459x_00):
            min_ver = ElemTree.Element(SecurityConfig.MINIMAL_VERSION_TAG)
            min_ver.text = str(self.adm_minimal_version)
            administration.append(min_ver)

        key_rev = ElemTree.Element(SecurityConfig.KEY_REV_TAG)
        tags = [SecurityConfig.KEY_REV_PUB_TAG, SecurityConfig.KEY_REV_SYM_USER_DATA_TAG]

        if self.product_id == ProductId.DA1469x_00:
            tags.append(SecurityConfig.KEY_REV_SYM_FW_DEC_TAG)

        for tag in tags:
            key_rev_type = ElemTree.Element(tag)
            for key in self.adm_key_revocations[tag]:
                elem = ElemTree.Element(SecurityConfig.KEY_REV_INDEX_TAG)
                elem.text = str(key)
                key_rev_type.append(elem)
            key_rev.append(key_rev_type)

        administration.append(key_rev)
        root.append(administration)

        with open(self.__file, 'wb') as f:
            text = ElemTree.tostring(root)
            f.write(xmldom.parseString(text).toprettyxml(indent=' ' * 4, encoding='UTF-8'))

    # Convert key revocation dictionary of lists to string which contains indexes. User data
    # decryption key index is preceded by 's' and FW decryption key index is preceded by 'd'
    # (DA1469x only).
    def make_revocation_string(self):
        rev_list = [str(k) for k in self.adm_key_revocations[SecurityConfig.KEY_REV_PUB_TAG]]

        if self.product_id == ProductId.DA1469x_00:
            rev_list.extend(['{}{}'.format(SecurityConfig.KEY_REV_SYM_FW_DEC_PREFIX, str(k)) for k in
                             self.adm_key_revocations[SecurityConfig.KEY_REV_SYM_FW_DEC_TAG]])

        rev_list.extend(['{}{}'.format(SecurityConfig.KEY_REV_SYM_USER_DATA_PREFIX, str(k)) for k in
                         self.adm_key_revocations[SecurityConfig.KEY_REV_SYM_USER_DATA_TAG]])

        return ' '.join(rev_list)

    # Convert key revocation list get from script parameter
    @staticmethod
    def parse_revocation_list(revocation_list):
        revocation_dict = {
            SecurityConfig.KEY_REV_PUB_TAG : [],
            SecurityConfig.KEY_REV_SYM_USER_DATA_TAG : [],
            SecurityConfig.KEY_REV_SYM_FW_DEC_TAG : []
        }

        if not revocation_list:
            return revocation_dict

        for item in revocation_list:
            if item[0] == SecurityConfig.KEY_REV_SYM_FW_DEC_PREFIX:
                revocation_dict[SecurityConfig.KEY_REV_SYM_FW_DEC_TAG].append(int(item[1:]))
            elif item[0] == SecurityConfig.KEY_REV_SYM_USER_DATA_PREFIX:
                revocation_dict[SecurityConfig.KEY_REV_SYM_USER_DATA_TAG].append(int(item[1:]))
            else:
                revocation_dict[SecurityConfig.KEY_REV_PUB_TAG].append(int(item))

        return revocation_dict


def prepare_configuration_str(security_config, product_keys):
    unknown = '[UNKNOWN]'

    try:
        asym_key = product_keys.asymmetric_keys[security_config.security_pub_key_idx]
    except IndexError:
        asym_key = unknown

    configuration_str = \
        'Product ID: ' + str(security_config.product_id.value) + '\n'

    configuration_str += \
        'Current FW version: ' + str(security_config.current_version) + '\n'

    if security_config.product_id == ProductId.DA1469x_00:
        configuration_str += \
            'Enable secure boot in CS: ' + str(security_config.cs_enable_secure_boot) + '\n'

    configuration_str += \
        'Security:\n' \
        '\tPublic key index: ' + str(security_config.security_pub_key_idx) + '\n' \
        '\tPrivate key: ' + str(getattr(asym_key, 'private', unknown)) + '\n' \
        '\tPublic key: ' + str(getattr(asym_key, 'public', unknown)) + '\n'

    if security_config.product_id == ProductId.DA14683_00:
        elliptic_curve = getattr(getattr(asym_key, 'elliptic_curve', unknown), 'value', unknown)
        configuration_str += \
            '\tElliptic curve: ' + str(elliptic_curve) + '\n' \
            '\tHash method: ' + str(security_config.security_hash_method.value) + '\n'
    elif security_config.product_id == ProductId.DA1469x_00:
        try:
            sym_key = product_keys.symmetric_fw_dec_keys[security_config.security_sym_key_idx]
        except IndexError:
            sym_key = unknown
        configuration_str += \
            '\tSymmetric key index: ' + str(security_config.security_sym_key_idx) + '\n' \
            '\tSymmetric key: ' + str(sym_key) + '\n'

    configuration_str += \
        'Administration:\n' + \
        '\tKey revocations: \n' + \
        '\t\tSignature key: ' + str(security_config.adm_key_revocations[
                                                SecurityConfig.KEY_REV_PUB_TAG] or '') + '\n' \
        '\t\tUser data key: ' + str(security_config.adm_key_revocations[
                                        SecurityConfig.KEY_REV_SYM_USER_DATA_TAG] or '') + '\n'

    if security_config.product_id == ProductId.DA1469x_00:
        configuration_str += \
            '\t\tFW decryption key: ' + str(security_config.adm_key_revocations[
                                        SecurityConfig.KEY_REV_SYM_FW_DEC_TAG] or '') + '\n'
    elif security_config.product_id == ProductId.DA14683_00:
        configuration_str += '\tMinimal version: ' + str(security_config.adm_minimal_version or '')
    elif security_config.product_id == ProductId.DA1459x_00:
        configuration_str += '\tMinimal version: ' + str(security_config.adm_minimal_version or '')
        configuration_str += \
            '\t\tFW decryption key: ' + str(security_config.adm_key_revocations[
                                        SecurityConfig.KEY_REV_SYM_FW_DEC_TAG] or '') + '\n'

    return configuration_str


def ask_key_rev(index_list, text, keys):
    rows = []
    for index in index_list:
        key = '' if index >= len(keys) else getattr(keys[index], 'public', keys[index])
        rows.append('{0: <3} | {1: <15}'.format(str(index), key))

    rev_dict = ui.fill_checkbox_list(OrderedDict.fromkeys(rows, False), text)

    if not rev_dict:
        # User canceled script
        raise ExecutionCanceled()

    return sorted([row.split('|')[0].strip() for (row, state) in rev_dict.items() if state])


def ask_sticky_bits(index_list, text):
    rows = []
    for index in index_list:
        rows.append(index)

    rev_dict = ui.fill_checkbox_list(OrderedDict.fromkeys(rows, False), text)

    if not rev_dict:
        # User canceled script
        raise ExecutionCanceled()

    return sorted([row.split('|')[0].strip() for (row, state) in rev_dict.items() if state])


def secure_keys_cfg(configuration_file=None, keys_file=None, prod_id=None, pub_key_idx=None,
                    elliptic_curve=None, hash_method=None, key_revocations=None, min_version=None,
                    sym_key_idx=None, nonce=None, enable_secure_boot=None):
    supported_prod_id = [ProductId.DA1459x_00.value]

    if configuration_file is None:
        configuration_file = DEFAULT_CONFIGURATION_FILE

    if keys_file is None:
        keys_file = DEFAULT_PRODUCT_KEYS_FILE

    if not os.path.exists(keys_file):
        if ui.ask(text='Would you like to create product keys file?'):
            if not prod_id:
                prod_id = ui.select_item(text='Select Product ID', item_list=supported_prod_id)
                if not prod_id:
                    raise ExecutionCanceled()

            generate_keys(keys_file, elliptic_curve=elliptic_curve, prod_id=prod_id)
        else:
            return

    ui.print_message('Using product keys file: ' + os.path.normpath(keys_file))

    security_config = SecurityConfig(configuration_file)
    product_keys = ProductKeys(keys_file)

    if os.path.exists(configuration_file) and security_config.is_valid():
        configuration_str = prepare_configuration_str(security_config, product_keys)

        if not ui.ask(text='Would you like to change existing configuration?\n' + configuration_str,
                      confirmation='Change', denial='Keep'):
            return

    # Could be passed or already selected
    if not prod_id:
        prod_id = ui.select_item(text='Select Product ID', item_list=supported_prod_id)
        if not prod_id:
            raise ExecutionCanceled()

    # Use Enum object instead of string
    try:
        product_id = ProductId(prod_id)
    except ValueError:
        raise RuntimeError('Invalid Product ID passed. Execution aborted.')

    if not product_keys.is_valid(product_id):
        raise RuntimeError('Invalid product keys file. Execution aborted.')

    if (product_id == ProductId.DA1469x_00):
        if enable_secure_boot is None:
            enable_secure_boot = ui.ask(text='Would you like to enable secure boot in CS?')
        elif enable_secure_boot == 'enable':
            enable_secure_boot = True
        elif enable_secure_boot == 'disable':
            enable_secure_boot = False
        else:
            raise RuntimeError('No valid option for secure boot enabling in CS. Execution aborted.')

    if (product_id == ProductId.DA1459x_00):
        security_config.enable_sticky_bits = ui.ask(text='Would you like to configure sticky bits in CS?')
        if security_config.enable_sticky_bits is True:
            sticky_bits_list = ask_sticky_bits(sb.sticky_bits_list, 'Select sticky bits to be enabled in CS.')

    # Set public key index if not passed
    if pub_key_idx is None:
        items = ['{0: <3} | {1: <15} | {2}'.format(str(index), key.elliptic_curve.value, key.public)
                 for index, key in enumerate(product_keys.asymmetric_keys)]

        selected = ui.select_item(item_list=items, text='Select public key index')
        if not selected:
            raise ExecutionCanceled()

        pub_key_idx = next((e.strip() for e in selected.split('|')))

    if product_id == ProductId.DA1469x_00:
        if not nonce and ui.ask(text='Would you like to add nonce?'):
            nonce = ui.ask_value(value_name='nonce', regex='[A-Fa-f0-9]{16}')

        if sym_key_idx is None:
            items = ['{0: <3} | {1}'.format(str(index), str(key)) for index, key in
                     enumerate(product_keys.symmetric_fw_dec_keys)]

            selected = ui.select_item(item_list=items,
                                      text='Select index of the FW decryption symmetric key')
            if not selected:
                raise ExecutionCanceled()

            sym_key_idx = next((e.strip() for e in selected.split('|')))

    if product_id == ProductId.DA14683_00:
        if not hash_method:
            hash_method = ui.select_item(item_list=[e.value for e in ALLOWED_HASH_METHODS],
                                         text='Select hash method', default=DEFAULT_HASH_METHOD_IDX)
            if not hash_method:
                raise ExecutionCanceled()

        hash_method = HashMethod(hash_method)

    if key_revocations is None and ui.ask(text='Would you like to add key revocations?'):
        asym_key_rev_list = list(range(4)) if product_id == ProductId.DA14683_00 else list(range(8))
        sym_key_rev_list = list(range(8))
        sym_fw_dec_rev_list = [] if product_id == ProductId.DA14683_00 or product_id == ProductId.DA1459x_00 else list(range(8))

        # Remove indexes of keys which will be used in image's signature verification/decryption
        asym_key_rev_list = list(filter(lambda x: x != int(str(pub_key_idx), 0), asym_key_rev_list))

        # Ask for key revocation in each group
        key_revocations = dict()
        key_revocations[SecurityConfig.KEY_REV_PUB_TAG] = \
            ask_key_rev(asym_key_rev_list, 'Select public key indexes which should be revoked.',
                        product_keys.asymmetric_keys)

        if sym_fw_dec_rev_list:
            key_revocations[SecurityConfig.KEY_REV_SYM_FW_DEC_TAG] = \
                ask_key_rev(sym_fw_dec_rev_list, 'Select FW decryption key indexes which should be '
                                                 'revoked.', product_keys.symmetric_fw_dec_keys)

        key_revocations[SecurityConfig.KEY_REV_SYM_USER_DATA_TAG] = \
            ask_key_rev(sym_key_rev_list, 'Select user data decryption key indexes which should be '
                                          'revoked.', product_keys.symmetric_keys)
    elif key_revocations is not None:
        # Don't remove public key and FW decryption key indexes which will be used for creating
        # image - leave all values which were given in command line call
        key_revocations = SecurityConfig.parse_revocation_list(key_revocations)

    security_config.product_id = product_id

    # Current version
    fw_version = ui.ask_value(value_name='Please provide current version of the firmware (in decimal).')
    try:
        fw_version_int = int(fw_version)
    except:
        fw_version = ui.ask_value(value_name='Value is incorrect. Please try again.')
        # This time script will raise an error if value passed is incorrect.
        fw_version_int = int(fw_version)

    # Minimal version
    if (product_id == ProductId.DA14683_00 or product_id == ProductId.DA1459x_00):
        if not min_version and ui.ask(text='Would you like to add minimal version?'):
            min_version = ui.ask_value(value_name='Please provide minimal version of the firmware (in decimal).')
            try:
                min_version_int = int(min_version)
            except:
                min_version = ui.ask_value(value_name='Value is incorrect. Please try again.')
                # This time script will raise an error if value passed is incorrect.
                min_version_int = int(min_version)

            if fw_version_int < min_version_int:
                ui.info('Warning! Current version is lower than minimal version!')

    security_config.security_pub_key_idx = pub_key_idx
    if key_revocations:
        security_config.adm_key_revocations = key_revocations
    else:
        # Keep empty dictionary of lists. This one from constructor could have been altered.
        security_config.adm_key_revocations = {
            SecurityConfig.KEY_REV_PUB_TAG : [],
            SecurityConfig.KEY_REV_SYM_USER_DATA_TAG : [],
            SecurityConfig.KEY_REV_SYM_FW_DEC_TAG : []
        }

    # Set only required field for specified product - rest will stay as None
    security_config.current_version = fw_version
    if product_id == ProductId.DA14683_00:
        security_config.security_hash_method = hash_method
        security_config.adm_minimal_version = min_version
    elif product_id == ProductId.DA1469x_00:
        security_config.cs_enable_secure_boot = enable_secure_boot
        security_config.security_sym_key_idx = sym_key_idx
        security_config.security_nonce = nonce
    elif product_id == ProductId.DA1459x_00:
        security_config.adm_minimal_version = min_version
        if security_config.enable_sticky_bits:
            security_config.enable_sticky_bits = True
            if "PROT_INFO_PAGE" in sticky_bits_list:
                security_config.cs_prot_info_page = True
            if "DISABLE_AUDIO_CODEC_ACC" in sticky_bits_list:
                security_config.cs_disable_audio_codec_acc = True
            if "DISABLE_QSPI_CONTROLLER" in sticky_bits_list:
                security_config.cs_disable_qspi_controller = True
            if "SECURE_BOOT" in sticky_bits_list:
                security_config.cs_enable_secure_boot = True
            if "FORCE_CMAC_DEBUGGER_OFF" in sticky_bits_list:
                security_config.cs_force_cmac_debugger_off = True
            if "FORCE_M33_DEBUGGER_OFF" in sticky_bits_list:
                security_config.cs_for_m33_debugger_off = True
            if "ENABLE_DCDC" in sticky_bits_list:
                security_config.cs_enable_dcdc = True
            if "PROT_USER_APP_CODE" in sticky_bits_list:
                security_config.cs_prot_user_app_code = True
            if "PROT_VALID_KEY" in sticky_bits_list:
                security_config.cs_prot_valid_key = True
            if "PROT_APP_KEY" in sticky_bits_list:
                security_config.cs_prot_app_key = True
            if "PROT_CONFIG_SCRIPT" in sticky_bits_list:
                security_config.cs_prot_config_script = True

    security_config.save()
    ui.print_message('Configuration saved to %s' % configuration_file)


def add_device_args(parser):
    supported_revisions = [ProductId.DA1459x_00, ProductId.DA14683_00, ProductId.DA1469x_00]
    revision_arg = parser.add_revision_args(revisions=supported_revisions)
    revision_arg.help += " Combined with '-h/--help' flag will display product specific parameters."

    args = \
        parser.parse_known_args(filter(lambda arg: arg != '-h' and arg != '--help', sys.argv))[0]

    if not hasattr(args, 'prod_id') or args.prod_id is None:
        return

    if args.prod_id == ProductId.DA14683_00:
        group = parser.add_argument_group(args.prod_id.value,
                                          "Parameters available only for {}".format(args.prod_id.value))
        group.add_argument('-ec', '--elliptic_curve',
                           choices=[e.value for e in ALLOWED_ELLIPTIC_CURVES], dest='elliptic_curve',
                           help='elliptic curve (used only when product keys file does not exist)')
        group.add_argument('--hash_method', choices=[e.value for e in ALLOWED_HASH_METHODS],
                           dest='hash_method', help='hash method')
        group.add_argument('--min_version', help='minimal version', nargs='?', default='')
    elif args.prod_id == ProductId.DA1469x_00:
        group = parser.add_argument_group(args.prod_id.value,
                                          "Parameters available only for {}".format(args.prod_id.value))
        group.add_argument('--sym_key_idx', help='index of symmetric key used in image encryption')
        group.add_argument('--nonce', help='nonce used in image encryption (AES CTR mode)')
        group.add_argument('--cs_sec_boot', choices=['enable', 'disable'], help='secure boot in CS')
    elif args.prod_id == ProductId.DA1459x_00:
        group.add_argument('--hash_method', choices=[e.value for e in ALLOWED_HASH_METHODS],
                           dest='hash_method', help='hash method')
        group.add_argument('--cs_sec_boot', choices=['enable', 'disable'], help='secure boot in CS')
    else:
        raise RuntimeError('No valid Product ID selected. Execution aborted.')


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('-cfg', metavar='<configuration_file>', dest='configuration_file',
                        help='configuration file')
    parser.add_argument('--product_keys', help='product keys file')
    parser.add_argument('--pub_key_idx', help='index of public key used in image signature '
                                              'verification')
    parser.add_argument('--key_revocations', nargs='+', help='indexes of keys to revoke')
    add_device_args(parser)
    args = parser.parse_args()

    return args.configuration_file, args.product_keys, args.prod_id, args.pub_key_idx, \
           getattr(args, 'elliptic_curve', None), getattr(args, 'hash_method', None), \
           args.key_revocations, getattr(args, 'min_version', None), \
           getattr(args, 'sym_key_idx', None), getattr(args, 'nonce', None), \
           getattr(args, 'cs_sec_boot', None)


if __name__ == '__main__':
    ui.print_header('SECURE IMAGE CONFIGURATOR')
    run_script(secure_keys_cfg, parse_args)
    ui.print_footer('FINISHED')
