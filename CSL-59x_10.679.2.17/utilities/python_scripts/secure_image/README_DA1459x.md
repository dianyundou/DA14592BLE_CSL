Secure image programming procedure {#secure_image}
==================================================

### Prerequisites

- All scripts require *Python 3*.
- If Python supports GUI mode (*Tkinter* package is required), the whole configuration will be done using popup-windows.
- Otherwise, scripts will use the simple command line interpreter.


### 1. Check tools

The tools required for secure image programming are the same as the tools
required for typical flash memory programming, i.e. `cli_programmer` and `mkimage`.
Both tools must be located in the `binaries/` directory.

### 2. Import python_scripts project to IDE

This project includes a collection of `python scripts` intended for being executed from a command line, as well
as some launchers intended for use with IDE as `External Tools`. Both methods achieve the same
results. In the first is chosen this step in out of scope.

### 3. Create Embedded Flash programming configuration file

Execute the `utilities/python_scripts/eflash/program_eflash_config.py` script or run the `program_eflash_config` launcher, 
and provide the following options:

- Product ID: DA1459x-00
- Flash Configuration: EFLASH
- active FW image address: 0x2000 (default) or another specified address.
- update FW image address: 0x2000 (default) or another specified address.

Upon execution, the `program_eflash.xml` file will be generated in the directory `utilities/python_scripts/eflash`, 
and the following log message will be printed out:

````
........................................................................................................................
..
.. PROGRAM EFLASH CONFIGURATOR
..
........................................................................................................................
.
........................................................................................................................
..
.. Active FW Image Address

..
........................................................................................................................
........................................................................................................................
..
.. Update FW Image Address

..
........................................................................................................................
.
........................................................................................................................
..
.. FINISHED
..
........................................................................................................................
````

### 4. Create product keys and secure image configuration files

Execute the `utilities/python_scripts/secure_image/secure_keys_cfg.py` script or run the `secure_config` launcher
and provide the following options:

- Would you like to create product keys file?: Yes
- Product ID: DA1459x-00
- Select elliptic curve used for asymmetric keys: EDWARDS25519
- Would you like to configure sticky bits in CS?: Yes/No
- Select sticky bits to be enabled in CS:
    + PROT_INFO_PAGE
    + SECURE_BOOT
    + FORCE_CMAC_DEBUGGER_OFF
    + FORCE_M33_DEBUGGER_OFF
    + PROT_USER_APP_CODE
    + PROT_VALID_KEY
    + PROT_APP_KEY
    + PROT_CONFIG_SCRIPT
- Select public key index: One of the provided options
- Would you like to add key revocations?: No/Yes
- Please provide current version of the firmware (in decimal): 
- Would you like to add minimal version?: No/Yes

Upon execution, the `product_keys.xml` and `secure_cfg.xml` file will be generated in the directory 
`utilities/python_scripts/secure_image`, and an identical to the following log message will be printed out:


````
........................................................................................................................
..
.. SECURE IMAGE CONFIGURATOR
..
........................................................................................................................
.
. Writing keys to product_keys.xml
Generating 8 keys (256-bits)...
    #1: 0037FA922E1A281B6CB0FC256432623A357421F373214D85680E5169CF9242CF
    #2: C93C61F7568A13C23A0FE89F424AD977BEFA6B321BB8B784C608ED959A2F6463
    #3: 6BC65BC2506E848A7D6C29BFB6033775FDA2A7195A5E9D21668AB601BA1B6425
    #4: E1BFE7312D6CBBABD8E56A8FE8A104E543ABFE9E099BBF6F267570E090D50571
    #5: 94EDA2C2595E6D3143D7C02B79C410BC6F0F5A78AA19E8D08F58B01F2DB691C2
    #6: A33384FC91F12DD4C8EEFF41B210FE221F589AC972829A01DB4A200800B1CAA3
    #7: E54E9F763FCD4B08BB4A496D5A478F79A02A4312ACDD138727339028E55ACBCA
    #8: A96B40E8388BF0F3D63A603081F0AA211AED33C6CA464EF17ADE195F38E529E1
Generating 8 keys on EDWARDS25519 elliptic curve...
    #1 (private key length: 32, public key length: 32):
        PRIVATE KEY: 6F1DCAA21CDBA048C6876B0DDECDA7A94FEB5B5F671C85CA8960337BADE8A01C
        PUBLIC KEY:  7F858469416548F5D05FBC71332508E6634DA7CD7DA2FCC92EB010894BEF7F85
    #2 (private key length: 32, public key length: 32):
        PRIVATE KEY: BCB5504818187368A8C84406AFA10876ED729CF11B5136413CD96C67D4834E90
        PUBLIC KEY:  33D18AB2DE1D7C3DA4FB5015520B8EEE0E5F57FD2D3D1E133646E4A17BED5668
    #3 (private key length: 32, public key length: 32):
        PRIVATE KEY: F773428BEDB41D6229C8A47D8B6CCCB66E64EE931B8649B1E0B9702B35DD7F2C
        PUBLIC KEY:  2987190B7A3AD892D9CF6AFFD4417212D2E5AAA0778ADAACFFF8A3DEBF2C893B
    #4 (private key length: 32, public key length: 32):
        PRIVATE KEY: 5F5740CE335BFB5C316FEFE6145B5F49802CF03B7723F169D2BC9CBB22ABE681
        PUBLIC KEY:  112D8370533F769A93C66155ADF8C5C63EED64B8BE3B3FDB0D3CD9A0FA713362
    #5 (private key length: 32, public key length: 32):
        PRIVATE KEY: E39CB6723908F27DA8E5E3D0763E9941780EA43963C6397F0DCDD78666C3BE49
        PUBLIC KEY:  7B10B5D55F3BA15217633BE894FB13A75B6C16D4D140B814AAC7EB492EC6F204
    #6 (private key length: 32, public key length: 32):
        PRIVATE KEY: D5643B08C7E8D42087868EA1056B3BA66D463B2F012712EB5B5061C27EA6AF53
        PUBLIC KEY:  9E225C0D2D685AC72F263D4CE41A264681F3228326E83A5CD2FA507BE33F338B
    #7 (private key length: 32, public key length: 32):
        PRIVATE KEY: 8DD9A8A8C1651855A4620D7F34F2B2EB0C5D13B7446361091E2726F6F3FF6D80
        PUBLIC KEY:  8F58249F0466B4B39D38D0172D562970DE6E3173040D3CCABB9457B783C05C40
    #8 (private key length: 32, public key length: 32):
        PRIVATE KEY: 77257C3A278AE22701751087A9D2A93AAFDEDA06C114D7EC73D4C2EA54ACE7CB
        PUBLIC KEY:  89DA51D9CBF4B9ECAAA855DDDA0DEB7D10F513F516B8D0C69ECE9FF2DE7CDB84
. Using product keys file: product_keys.xml
. Configuration saved to secure_cfg.xml
.
........................................................................................................................
..
.. FINISHED
..
........................................................................................................................

````

> _Note_: Ensure that you do not remove/misplace the product keys file. Without it, it will be impossible to update 
> the software on a device with programmed keys and enabled secure boot. The same configuration/product keys file can
> be utilized for programming multiple devices of the same type.

### 5. Build application binary

Build the application you intend to program into the device's embedded flash memory. Make sure to choose either a Release 
or Debug build configuration, such as `DA1459x-00-Release_eFLASH` or `DA1459x-00-Debug_eFLASH`.

> _Note_: Please note that the secure image feature is only supported by the `pxp_reporter` and `ble_adv` applications provided by the SDK.

### 6. Program application image

To generate a signed image from the application binary file, created at step 5, and program it to the embedded flash memory, execute the
`utilities/python_scripts/secure_image/secure_img_prog.py` script via JLink or Serial interface, or run the `secure_keys_img_jtag`/
`secure_keys_img_serial` launcher. In the first case, if the `product_keys.xml` and `secure_cfg.xml` files are not located in the default
directory, their paths need also to be specified, and in turns, the path of the binary file has to be passed as input argument to the
script. In the second case, ensure that you have selected the application project before running the launcher. This step is crucial for
the launcher to locate the necessary *.xml and binary files correctly.

### 7. Program the Configuration Script and the Product Keys

During this step, the Configuration Script (CS) and the Product Keys, as specified at step 4, will be written to
their respective locations within the Embedded Flash (CS: 0x000 - 0xFFF - User Application Keys: 0x800 - 0xBFF -
Signature Keys: 0xC00 - 0xFFF). Depending on whether the mentioned fields are already written or not, and if they 
are, whether the new values match the existing ones, the script will execute specific actions accordingly.

To initiate the operation, execute the `utilities/python_scripts/secure_image/secure_keys_prog.py` script, via JLink 
or Serial interface, specifying the paths to the `product_keys.xml` and `secure_cfg.xml` files, if they are in different 
directory than the default location, or run the `secure_keys_prog_jtag`/`secure_keys_prog_serial` launcher.

Upon execution, a log message  identical to the following will be printed out, informing the user about the actions
that have been peformed:

````
........................................................................................................................
..
.. PROGRAM PRODUCT KEYS AND CS (eFlash)
..
........................................................................................................................
.
. Using product keys file: <SDK_ROOT_PATH>/utilities/python_scripts/secure_image/product_keys.xml
. Using secure configuration file: <SDK_ROOT_PATH>/utilities/python_scripts/secure_image/secure_cfg.xml
........................................................................................................................
..
.. Programming product keys
..
........................................................................................................................
cli_programmer 1.26
Copyright (c) 2015-2023 Dialog Semiconductor

bootloader file not specified, using internal uartboot.bin

Uploading boot loader/application executable...
Executable uploaded.

0300   8D 14 82 F9
0301   C1 C2 6E B7
0302   B5 56 4C F5
0303   DC CE B3 06
0304   4A D9 81 81
0305   60 A0 E2 60
0306   7A D1 37 EA
0307   2F CB 77 32
0308   AF 87 15 6F
0309   AF 88 8D BF
030A   39 3F 76 74
030B   BF F0 BD 6D
030C   2B 35 CD C5
030D   DB A5 B0 C5
030E   74 7F D6 C3
030F   C6 83 5B 4F
0310   EE E6 AC 86
0311   03 05 05 E4
0312   29 7D 02 77
0313   DE 03 13 BD
0314   AB D1 85 00
0315   AA F2 3E 82
0316   13 F7 1C BE
0317   80 73 66 82
0318   A2 BD F5 24 
0319   BF 16 C5 FB
031A   F8 A4 86 2C
031B   91 9B B8 54
031C   58 2F 11 FE
031D   7D 03 59 3A
031E   5B 16 0B C6
031F   52 B0 83 D2
0320   36 C2 EC FD
0321   90 C5 BB 59
0322   A9 41 C4 15
0323   19 43 E2 E0
0324   55 DD 70 8F
0325   F9 B6 DF D5
0326   65 D0 B4 EC
0327   C8 26 3F B1
0328   2C 32 73 70
0329   D0 26 A7 84
032A   03 D3 26 A6
032B   29 9D F4 AD
032C   EA B4 90 BE
032D   BD 7E D4 0A
032E   34 18 FF AA
032F   0F 17 C8 8A
0330   57 45 D5 A6
0331   7E 37 D0 55
0332   B6 80 AE 64
0333   CB 48 8E 1A
0334   85 BD D9 47
0335   3E CD 3B AF
0336   99 00 B3 DF
0337   8C 32 D2 56
0338   56 24 2D 94
0339   65 F7 12 EB
033A   F9 AB 20 5D
033B   4F 24 14 90
033C   E1 B9 04 72
033D   1D BA 22 D5
033E   18 9C E8 37
033F   64 6E 04 33

done.

cli_programmer 1.26
Copyright (c) 2015-2023 Dialog Semiconductor

bootloader file not specified, using internal uartboot.bin

Uploading boot loader/application executable...
Executable uploaded.

0200   00 00 00 00   ....
0201   00 00 00 00   ....
0202   00 00 00 00   ....
0203   00 00 00 00   ....
0204   00 00 00 00   ....
0205   00 00 00 00   ....
0206   00 00 00 00   ....
0207   00 00 00 00   ....
0208   00 00 00 00   ....
0209   00 00 00 00   ....
020A   00 00 00 00   ....
020B   00 00 00 00   ....
020C   00 00 00 00   ....
020D   00 00 00 00   ....
020E   00 00 00 00   ....
020F   00 00 00 00   ....
0210   00 00 00 00   ....
0211   00 00 00 00   ....
0212   00 00 00 00   ....
0213   00 00 00 00   ....
0214   00 00 00 00   ....
0215   00 00 00 00   ....
0216   00 00 00 00   ....
0217   00 00 00 00   ....
0218   00 00 00 00   ....
0219   00 00 00 00   ....
021A   00 00 00 00   ....
021B   00 00 00 00   ....
021C   00 00 00 00   ....
021D   00 00 00 00   ....
021E   00 00 00 00   ....
021F   00 00 00 00   ....
0220   00 00 00 00   ....
0221   00 00 00 00   ....
0222   00 00 00 00   ....
0223   00 00 00 00   ....
0224   00 00 00 00   ....
0225   00 00 00 00   ....
0226   00 00 00 00   ....
0227   00 00 00 00   ....
0228   00 00 00 00   ....
0229   00 00 00 00   ....
022A   00 00 00 00   ....
022B   00 00 00 00   ....
022C   00 00 00 00   ....
022D   00 00 00 00   ....
022E   00 00 00 00   ....
022F   00 00 00 00   ....
0230   00 00 00 00   ....
0231   00 00 00 00   ....
0232   00 00 00 00   ....
0233   00 00 00 00   ....
0234   00 00 00 00   ....
0235   00 00 00 00   ....
0236   00 00 00 00   ....
0237   00 00 00 00   ....
0238   00 00 00 00   ....
0239   00 00 00 00   ....
023A   00 00 00 00   ....
023B   00 00 00 00   ....
023C   00 00 00 00   ....
023D   00 00 00 00   ....
023E   00 00 00 00   ....
023F   00 00 00 00   ....
done.
........................................................................................................................
..
.. Checking key revocation status
..
........................................................................................................................
cli_programmer 1.26
Copyright (c) 2015-2023 Dialog Semiconductor

bootloader file not specified, using internal uartboot.bin

Uploading boot loader/application executable...
Executable uploaded.

01F8   FF FF FF FF   ....
01F9   FF FF FF FF   ....
01FA   FF FF FF FF   ....
01FB   FF FF FF FF   ....
01FC   FF FF FF FF   ....
01FD   FF FF FF FF   ....
01FE   FF FF FF FF   ....
01FF   FF FF FF FF   ....
done.
cli_programmer 1.26
Copyright (c) 2015-2023 Dialog Semiconductor

bootloader file not specified, using internal uartboot.bin

Uploading boot loader/application executable...
Executable uploaded.

01F0   FF FF FF FF   ....
01F1   FF FF FF FF   ....
01F2   FF FF FF FF   ....
01F3   FF FF FF FF   ....
01F4   FF FF FF FF   ....
01F5   FF FF FF FF   ....
01F6   FF FF FF FF   ....
01F7   FF FF FF FF   ....
done.
........................................................................................................................
..
.. Script execution report
..
........................................................................................................................
.
. Checking configuration and product keys files... PASS
. Checking product ID... PASS
. Product keys verification... PASS
. Checking eFlash memory emptiness (keys area)... FAIL
. Matching the programmed eFlash keys with the ones in product keys file... FAIL
. Writing keys to the eFlash memory... NOT RUN
. Reading revocation info from eFlash... PASS
. 	Revoked signature keys: []
. 	Revoked user data keys: []
. Enabling secure boot feature in configuration script... NOT RUN
. 	Product keys from file do not match to the keys read from eFlash.
.
........................................................................................................................
..
.. FINISHED
..
........................................................................................................................

````

### 8. Reboot device

After completing all the previous steps successfully, press the Reset button on the daughterboard to initiate the
application, which should start running.


### Increase device security

To enhance device security, you can perform the following additional actions:

- Disable debugger
- Disable the CPU's ability to read and/or write the User Application Keys area.
- Disable the CPU's ability to read and/or write the Signature Keys.
- Disable the Development Mode in CS (this function disables the ability to receive application code from UART and debugger).

> _Note_: Please be aware that disabling the Debugger or the Development Mode means that the only option for updating the
> software on the device will be through Software Update over the Air (SUOTA) service. Hence, the application programmed
> in flash must support this feature to enable software updates remotely.

### Known restrictions and limitations

- The process in step 4 always generates 8 User Application and Signature Keys. If you wish to use fewer keys, you must manually
edit the `product_keys.xml` file. Please note that the script cannot write the remaining keys; instead, they need to be manually
written using `cli_programmer`.
- The `secure_keys_cfg.py` script and the `secure_config` launcher cannot overwrite the `product_keys.xml`. If you need to change 
keys, you must manually remove the file. In such a case, make sure to keep a backup of the previous keys file in a different location.
- The scripts do not verify which keys are already revoked (this information is only provided as a log). If a key has been revoked 
- and its revocation is requested again, no action will be taken for the specified key.

### Troubleshooting

When the programmed application image cannot be booted on the device, check the following:

| Possible Cause | Recommended Action |
| -- | -- |
| Secure boot support is not enabled in the configuration script. | Enable it and perform step 7 again. |
| The signature used for image generation does not match to the key that is stored in the eFlash memory. | Make sure that there is no other product keys file that might have been programmed to the eFlash memory earlier. Perform step 7 again and check the status of the relevant step in the short report ('Matching the programmed eFlash keys with the ones in product keys file..'). |
| Used signature key has been revoked by the previous image. | At least one key in each category must be valid. Try to use another key index. Perform step 7 again and check which keys are already revoked on the device. |
