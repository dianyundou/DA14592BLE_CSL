CLI programmer {#cli_programmer}
================================

## Overview

`cli_programmer` is a command line tool for reading & writing to embedded FLASH (eFLASH), RAM and
QSPI FLASH/RAM. It also provides some extra functions like loading & executing an image from RAM.
The tool communicates with the target device over UART port or JTAG interface.
It runs on Windows and Linux platforms.

## Usage

To run cli_programmer, user has to specify interface (serial port (UART) or GDB server (JTAG)) and
requested command.

    cli_programmer [<options>] <interface> <command> [<args>]

### Interface

* the serial port file name as presented by the operating system e.g. \b `COM40` (Windows),
\b `/dev/ttyUSB0` (Linux), or
* \b `gdbserver`, if JTAG interface is to be used (J-Link debugger with the GDB server).

### Commands and arguments

    read <address> <file> <size>

Reads `size` bytes from the RAM memory, starting at `address` into `file`. If `file` is specified as
either '-' or '--', data is output to stdout as hexdump. The hexdump is either 16-bytes (-) or
32-bytes (--) wide.

    write <address> <file> [<size>]

Writes up to `size` bytes of `file` into the RAM memory at `address`. If `size` is omitted, the
complete `file` is written.

    read_eflash <address> <file> <size>

Reads `size` bytes from the eFLASH memory, starting at `address` into `file`. If `file` is specified
as either '-' or '--', data is output to stdout as hexdump. The hexdump is either 16-bytes (-) or
32-bytes (--) wide.

    write_eflash <address> <file> [<size>]

Writes up to `size` bytes of `file` into the eFLASH at `address`. If `size` is omitted, the complete
file is written.

    write_eflash_bytes <address> <data1> [<data2> [...]]

Writes `size` bytes specified on command line into the eFLASH at `address`.

    copy_eflash <address_ram> <address_eflash> <size>

Copies `size` bytes from the RAM memory, starting at `address_ram` to eFLASH at `address_eflash`.
This is an advanced command and is not needed by the end user.

    is_empty_eflash [<start_address> <size>]

Checks if eFLASH contains only 0xFF values. If no arguments are specified, starting address is 0 and
size is 256KB.
Command prints whether eflash is empty and if not, the offset of the first non-empty byte.

    erase_eflash <address> <size>

Erases `size` bytes of the eFLASH, starting at `address`.
> Note: The actual area erased may be larger, due to the size of the erase block.

    chip_erase_eflash

Erases the whole QSPI FLASH.

    read_qspi <address> <file> <size>

Reads `size` bytes from the QSPI FLASH/RAM memory, starting at `address` into `file`. If `file` is
specified as either '-' or '--', data is output to stdout as hexdump. The hexdump is either
16-bytes (-) or 32-bytes (--) wide.

    write_qspi <address> <file> [<size>]

Writes up to `size` bytes of `file` into the QSPI FLASH/RAM at `address`. If `size` is omitted, the
complete file is written.

    write_qspi_bytes <address> <data1> [<data2> [...]]

Writes bytes specified on command line into the QSPI FLASH/RAM at `address`.

    copy_qspi <address_ram> <address_qspi> <size>

Copies `size` bytes from the RAM memory, starting at `address_ram` to FLASH/RAM at `address_qspi`.
This is an advanced command an is not needed by end user.

    is_empty_qspi [<start_address> <size>]

Checks if QSPI FLASH contains only 0xFF values. If no arguments are specified, starting address is 0
and size is 1MB.
Command prints whether flash is empty and if not, the offset of the first non-empty byte.

    erase_qspi <address> <size>

Erases `size` bytes of the QSPI FLASH, starting at `address`.
> Note: The actual area erased may be larger, due to the size of the erase block.

    chip_erase_qspi

Erases the whole QSPI FLASH.

    read_partition_table

Reads the partition table (if any exists) and prints its contents.

    read_partition <part_name|part_id> <address> <file> <size>

Reads `size` bytes from partition, selected by `part_name` or `part_id` according to the below
table, starting at `address` into `file`. If `file` is specified as either '-' or '--', data is
output to stdout as hexdump. The hexdump is either 16-bytes (-) or 32-bytes (--) wide.

|         part_name         | part_id |
|---------------------------|---------|
|NVMS_FIRMWARE_PART         |    1    |
|NVMS_PARAM_PART            |    2    |
|NVMS_BIN_PART              |    3    |
|NVMS_LOG_PART              |    4    |
|NVMS_GENERIC_PART          |    5    |
|NVMS_PLATFORM_PARAMS_PART  |    15   |
|NVMS_PARTITION_TABLE       |    16   |
|NVMS_FW_EXEC_PART          |    17   |
|NVMS_FW_UPDATE_PART        |    18   |
|NVMS_PRODUCT_HEADER_PART   |    19   |
|NVMS_IMAGE_HEADER_PART     |    20   |

    write_partition <part_name|part_id> <address> <file> [<size>]

Writes up to `size` bytes of `file` into NVMS partition, selected by `part_name` or `part_id`
according to the above table, at `address`.
If `size` is omitted, the complete file is written.
If `file` is specified as either '-' or '--', data is output to stdout as hexdump. The hexdump is
either 16-bytes (-) or 32-bytes (--) wide.

    write_partition_bytes <part_name|part_id> <address> <data1> [<data2> [...]]

Writes bytes specified on command line into the NVMS partition, selected by `part_name` or `part_id`
according to the above table, at `address`.

    read_otp <address> <length>

Reads `length` 32-bit words from the write-protectable area of eFLASH (that can be used as an
One-Time Programmable (OTP) memory) at `address`.
> Note: The `address` should be provided in 32-bit word units and the data to be read should reside
inside one of the write-protectable sectors.

    write_otp <address> <length> [<data> [<data> [...]]]

Writes `length` words to the write-protectable area of eFLASH at `address`. `data` are 32-bit words
to be written. If less than `length` words are specified, remaining words are assumed to be 0x00.
> Note: The address range to be programmed should reside inside the write-protectable sectors.
However, no check is made in advance of whether the write protection in respective sector(s)
has been enabled or not. If it is enabled, an error will be returned. If the protection is not
enabled write will complete successfully but no other action will be performed. It is
responsibility of the user to enable the write protection manually in the end by writing the
proper words in the Configuration Section (this will disable writing again in the respective
sectors permanently).

    boot <binary_file>

Boots application binary using the 1st stage bootloader (ROM booter).
If the application is too big (more than 64KB) and serial interface is used, then the 'run' command
should be executed instead.

    run <binary_file>

Runs application binary using 2nd stage bootloader (uartboot).
It supports bigger application binaries than the 'boot' command.
The limitation is the size of '.inputbuffer' section in the 'uartboot' application.

    get_product_info

Returns device classification and production information. The product information can serve as a
unique identifier that is readable and not editable by the application. The information is a
combination of device classification attributes (family, variant, chip ID, version) as stored in
designated device registers, production layout (package, wafer number, die coordinates) and testing
information as stored in the device's OTP memory.

    read_flash_info

Reads manufacturer ID as well as device type and density of all the available flash memories.

### General options

    -h

Prints help screen and exits.

    --save-ini

Saves CLI programmer configuration to the `cli_programmer.ini` file and exits.

    -b <file>

Filename of 2nd stage bootloader or application binary. In GDB Server interface mode, this could be
also the 'attach' keyword. This keyword omits platform reset and loading of bootloader binary.

    --trc <cmd>

Target reset command. May be used if there is a need to replace the default localhost reset command.
This option shouldn't be used with the '--check-booter-load' option - in such a case, it is ignored.

    --prod-id <id>
Returns the chip product id (in the form of DAxxxxx-yy). This command is not supported from DA1459x devices.

### GDB server specific options

    -p <port_num>

TCP port number that GDB server listens to. The default value is 2331.

    -r <host>

GNU server host. The default is \`localhost\`.

    --no-kill [mode]

Don't stop running GDB Server instances. Available modes:
                                \'0\': Stop GDB Server instances during initialization and closing
                                \'1\': Don't stop GDB Server during initialization
                                \'2\': Don't stop GDB Server during closing
                                \'3\' or none: Don't stop any GDB Server instance (default)


    --gdb-cmd <cmd>

GDB server command used for executing and passing the right parameters to GDB server.
Without this parameter, no GDB server instance will be started or stopped. As GDB server command
line can be quite long, it is recommended that it is stored in cli_programmer.ini file using
--save-ini command line option.

    --check-booter-load

Don't force bootloader loading if it is running on the platform already. This option shouldn't be
used with the '--trc' option - in such a case, the '--trc' option is ignored.

### Serial port specific options

    -s <baudrate>

Baud rate used for UART by uartboot. The parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). It can be 9600, 19200, 57600, 115200, 230400, 500000,
1000000 (default).

    -i <baudrate>

Initial baud rate used for uploading the `uartboot` or a user-supplied binary. This depends on
the rate used by the bootloader of the device. The default behavior is to use the value passed by
'-s' or its default, if the parameter is not given. The argument is ignored by the `boot` command.
'-s' option should be used in this case.

    --tx-port <port_num>

GPIO port used for UART Tx by the `uartboot`. This parameter is patched to the uploaded uartboot
binary (in that way passed as a parameter). The default value is 0. This argument is ignored when
the `boot` command is given.

    --tx-pin <pin_num>

GPIO pin used for UART Tx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 13. The argument is ignored when the
`boot` command is given.

    --rx-port <port_num>

GPIO port used for UART Rx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 0. The argument is ignored when the `boot`
command is given.

    --rx-pin <pin_num>

GPIO pin used for UART Rx by uartboot. This parameter is patched to the uploaded uartboot binary
(in that way passed as a parameter). The default value is 15. The argument is ignored when the
`boot` command is given.

    -w <timeout>

Serial port communication timeout is used only during download of uartboot binary. If the device
does not respond during this time, cli_programmer exits with timeout error.

### Configuration file

When cli_programmer is executed, it first tries to read cli_programmer.ini file which may contain
various cli_programmer options.
Instead of creating this file manually, user should use --save-ini command line option.
Format of cli_programmer.ini adheres to standard Windows ini file syntax.
The cli_programmer looks for ini file in the following locations:

- current directory
- home directory
- cli_programmer executable directory


## Usage examples

Write binary data from local file to eFLASH.

    cli_programmer COM40 write_eflash 0x0 data_i

Write binary data from local file to QSPI FLASH/RAM.

    cli_programmer /dev/ttyUSB0 write_qspi 0x0 data_i

Read data from eFLASH to local file.

    cli_programmer COM40 read_eflash 0x0 data_o 0x100

Download custom binary `test_api.bin` to RAM and execute it.

    cli_programmer -b test_api.bin COM40 boot

Write data specified in command line to eFLASH at specified location.

    cli_programmer COM40 write_eflash_bytes 0x00006000 0x11 0x22 0x33

Run a few commands with uartboot, using UART Tx/Rx P0_13/P0_15 at baud rate 115200
(initial rate for uartboot uploading is 9600).

    cli_programmer -i 9600 -s 115200 --tx-port 0 --tx-pin 13 --rx-port 0 --rx-pin 15 COM40 write_eflash 0x0 data_i
    cli_programmer -i 9600 -s 115200 --tx-port 0 --tx-pin 13 --rx-port 0 --rx-pin 15 COM40 read_eflash 0x0 data_o 0x100

Read QSPI FLASH/RAM contents (10 bytes at address 0x0).

    cli_programmer gdbserver read_qspi 0 -- 10

Write settings to the `cli_programmer.ini` file. Long bootloader path is passed with -b option and
command line to start GDB server is passed with --gdb-cmd.
In this example, GDB server command line contains arguments and path to executable has space, so
whole command line is put in quotes and the quotes required by Windows path are additionally
escaped.

    cli_programmer -b C:\Users\user\SDK\sdk\bsp\system\loaders\uartboot\Release\uartboot.bin --save-ini --gdb-cmd "\"C:\Program Files (x86)\SEGGER\JLink_V722b\JLinkGDBServerCL.exe\" -if SWD -device Cortex-M33 -singlerun -silent -speed auto"

Write OTP (i.e. write-protectable area of eFLASH) at address 0x0 with the following contents: B0:0x00, B1:0x01, B2:0x02, B3:0x03, B4:0x04, B5:0x05, B6:0x06, B7:0x07

     cli_programmer gdbserver write_otp 0x0 2 0x03020100 0x07060504

Read the first two words of OTP at address 0x0.

     cli_programmer gdbserver read_otp 0x0 2

     If written with the contents from above write example, it should return the following:
     00   00 01 02 03   ........
     01   04 05 06 07   ........

Read 16 bytes from eFLASH at address 0x10.

     cli_programmer gdbserver read_eflash 0x00000010 -- 16


## Building cli_programmer

- cli_programmer makes use of the 'libprogrammer' library which implements the underlying
functionality on the host side. 'cli_programmer' can be linked either statically or dynamically with
'libprogrammer'.

- cli_programmer uses the 'uartboot' application (the respective project is located under
`sdk/bsp/system/loaders/uartboot`), which acts as a secondary bootloader that cli_programmer
downloads to the target for performing the read/write operations.

- Build configurations:

  * Debug_static_linux		- Debug version linked with a static version of libprogrammer -
                            recommended for Linux.
  * Debug_dynamic_linux		- Debug version for Linux linking dynamically to libprogrammer.
  * Debug_static_win64		- Debug 64bit version for Windows linked with a static version of
                            libprogrammer.
  * Debug_dynamic_win64		- Debug 64bit version for Windows linking dynamically to libprogrammer.

  * Release_static_linux	- Release version linked with a static version of libprogrammer -
                            recommended for Linux.
  * Release_dynamic_linux	- Release version for Linux linking dynamically to libprogrammer.
  * Release_static_win32	- Release 32bit version for Windows linked with a static version of
                            libprogrammer.
  * Release_static_win64	- Release 64bit version for Windows linked with a static version of
                            libprogrammer.
  * Release_dynamic_win64	- Release 64bit version for Windows linking dynamically to libprogrammer.

- Build instructions
  * Import `libprogrammer`, `cli_programmer` and `uartboot` into SmartSnippets Studio.
  * Build `libprogrammer` , `cli_programmer` and `uartboot` in `Release_static` configuration
  (recommended).
  * Run `cli_programmer` with proper parameters, as described in `Usage` and
  `Commands and arguments` sections.

> Notes:
> * A prebuilt version of cli_programmer can be found under SDK's `binaries` folder.
> * Building cli_programmer updates SDK's `binaries` folder (the new binaries are copied there).
> * Linux `cli_programmer` binaries built using the dynamic build configurations search for the
    library file `libprogrammer.so` explicitly in the `binaries` folder.
