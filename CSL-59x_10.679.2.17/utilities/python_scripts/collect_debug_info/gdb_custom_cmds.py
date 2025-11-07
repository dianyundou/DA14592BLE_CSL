#!/usr/bin/env python

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


import gdb
import re

REG_ADDRESS = 'ADDRESS'
REG_FIELDS = 'FIELDS'

def bitrange_str_to_bitrange_list(bitrange_str):
    end_start_bit_str = bitrange_str.split(':')
    return [int(end_start_bit_str[0].split('[')[1]), int(end_start_bit_str[1].split(']')[0])]


def bitrange_list_to_mask(bitrange_list):
    end_bit = bitrange_list[0]
    start_bit = bitrange_list[1]
    assert end_bit >= start_bit
    return ((1 << (end_bit - start_bit + 1)) - 1) << start_bit


def count_trailing_zeros(value):
    if value == 0:
        return 0
    else:
        num = 0
        while (value & 1) == 0:
            value = value >> 1
            num = num + 1
        return num


def remove_trailing_zeros(value):
    return value >> (count_trailing_zeros(value))


def regval_raw_getf(regval, mask):
    return remove_trailing_zeros(regval & mask)


def regval_raw_setf(regval, mask, val):
    return (regval & ~mask ) | (mask & (val << count_trailing_zeros(mask)))


def read_word(addr):
    return int(gdb.execute("x /1xw 0x{:08x}".format(addr), to_string=True).split(':', 1)[1].strip(), 0)


def write_word(addr, val):
    gdb.execute("monitor memU32 0x{:08x} = 0x{:08x}".format(addr, val), to_string=True)


def reg_raw_getf(addr, mask):
    return regval_raw_getf(read_word(addr), mask)


def reg_raw_setf(addr, mask, val):
    write_word(addr, regval_raw_setf(read_word(addr), mask, val))


def reg_getf(reg, field):
    return reg_raw_getf(reg[REG_ADDRESS], reg[REG_FIELDS][field])


def reg_setf(reg, field, val):
    reg_raw_setf(reg[REG_ADDRESS],reg[REG_FIELDS][field], val)


CM_CTRL3_REG = {
    REG_ADDRESS: 0x40002008,
    REG_FIELDS: {
        'MCPU_CACHE_RAM_MUX': 0x4,
    }
}

SYS_STAT_REG = {
    REG_ADDRESS: 0x50000028,
    REG_FIELDS: {
        'RAD_IS_UP': 0x2,
    }
}

PMU_CTRL_REG = {
    REG_ADDRESS: 0x50000020,
    REG_FIELDS: {
        'RETAIN_CMAC_CACHE': 0x800,
        'RADIO_SLEEP': 0x2,
    }
}

CLK_RADIO_REG = {
    REG_ADDRESS: 0x50000010,
    REG_FIELDS: {
        'CMAC_CLK_ENABLE': 0x0,
    }
}


def power_up_rad():
    if reg_getf(SYS_STAT_REG,'RAD_IS_UP') == 0:
        gdb.execute("echo INFO: PD_RAD was not already up.\n")
        if reg_getf(PMU_CTRL_REG, 'RETAIN_CMAC_CACHE') == 1:
            gdb.execute("echo Attempting to switch PD_RAD on manually.\n")
            reg_setf(PMU_CTRL_REG, 'RADIO_SLEEP', 0)
            while reg_getf(SYS_STAT_REG, 'RAD_IS_UP') == 0:
                pass
            gdb.execute("echo INFO: PD_RAD was activated successfully.\n")
            return True
        else:
            gdb.execute("echo Cache was not retained! Since PD_RAD was off, no data can be recovered!\n")
            return False
    else:
        gdb.execute("echo INFO: PD_RAD is already up.\n")
        return True


def enable_cmac_cache_clock():
    if reg_getf(CLK_RADIO_REG, 'CMAC_CLK_ENABLE') == 1:
        gdb.execute("echo INFO: CMAC clock was not enabled. "
                         "INFO: Attempting to enable it manually..\n")
        reg_setf(CLK_RADIO_REG, 'CMAC_CLK_ENABLE', 0)
        gdb.execute("echo INFO: CMAC clock was enabled successfully.\n")
    else:
        gdb.execute("echo INFO: CMAC clock is already enabled.\n")


def enable_cmac_cache_bypass():
    if reg_getf(CM_CTRL3_REG, 'MCPU_CACHE_RAM_MUX') == 1:
        gdb.execute("echo INFO: CMAC CACHE RAM bypass was not enabled. "
                         "INFO: Attempting to enable it manually..\n")
        reg_setf(CM_CTRL3_REG, 'MCPU_CACHE_RAM_MUX', 0)
        gdb.execute("echo INFO: CMAC CACHE RAM bypass was enabled successfully.\n")
    else:
        gdb.execute("echo INFO: CMAC CACHE RAM bypass is already enabled.\n")


class ReadCMACCacheRam(gdb.Command):
    def __init__(self):
        super(ReadCMACCacheRam, self).__init__("dmp-cmac_cache_memory", gdb.COMMAND_STATUS, gdb.COMPLETE_NONE)

    # noinspection PyUnusedLocal, PyMethodMayBeStatic
    def invoke(self, arg, from_tty):
        if power_up_rad():
            enable_cmac_cache_clock()
            enable_cmac_cache_bypass()
            cmac_cache_ram_start = 0x40108000
            cmac_cache_ram_size_in_words = 0x2000 / 4
            gdb.execute("echo INFO: Dumping CMAC CACHE RAM content:\n")
            gdb.execute("x/" + str(cmac_cache_ram_size_in_words) + "x " + str(cmac_cache_ram_start) + "\n")

ReadCMACCacheRam()


class IgnoreErrorsCommand (gdb.Command):

        def __init__ (self):
                super (IgnoreErrorsCommand, self).__init__ ("ignore-errors",
                                                            gdb.COMMAND_OBSCURE,
                                                            gdb.COMPLETE_COMMAND)
        def invoke (self, arg, from_tty):
                try:
                        gdb.execute (arg, from_tty)
                except:
                        pass
IgnoreErrorsCommand()

class IterateFreeRtosListCommand(gdb.Command):

        MAX_ITER = 40

        def __init__(self):
                super(IterateFreeRtosListCommand, self).__init__("iterate-freertos-list",
                                                                 gdb.COMMAND_DATA,
                                                                 gdb.COMPLETE_SYMBOL)

        ### TODO FPU registers are not taken into account.
        def regs_stack_to_index(self, regname):
                regs_dict = {'xpcr':15,
                             'pc':14,
                             'lr':13,
                             'r12':12,
                             'r3':11,
                             'r2':10,
                             'r1':9,
                             'r0':8, # start of HW stacking
                             'r11':7,
                             'r10':6,
                             'r9':5,
                             'r8':4,
                             'r7':3,
                             'r6':2,
                             'r5':1,
                             'r4':0  # start of SW stacking see xPortPendSVHandler() implementation of FreeRTOS
                             }

                return regs_dict[regname]

        def invoke(self, argument, from_tty):
                args = gdb.string_to_argv(argument)
                expr = args[0]
                List_t = gdb.parse_and_eval(expr)
                #print List_t
                TCB_t_p = gdb.lookup_type("TCB_t").pointer() #TCB_t *
                uint32_p = gdb.lookup_type("uint32_t").pointer() #uint32_t *

                if len(args) == 2:
                        max_iter = int(args[1])
                else:
                        max_iter = self.MAX_ITER

                pxCurrentTCB = gdb.parse_and_eval("pxCurrentTCB")


                uxNumberOfItems = List_t["uxNumberOfItems"]

                #this is the our sentinel to iterate the list (marker called in FreeRTOS)
                xListEnd = List_t["xListEnd"]

                item = xListEnd["pxNext"]
                #print xListEnd.address
                #print xListEnd["pxNext"]

                i = 1

                # print "pxCurrentTCB %s" % (pxCurrentTCB.dereference())
                while xListEnd.address != item:
                        print "node@%s: %s #%d" % (item, item.dereference(), i)

                        if (uxNumberOfItems == 0):
                                print "List corruption: Iterated  %d times uxNumberOfItems %d" % (i, uxNumberOfItems)
                                break

                        if (i == max_iter):
                                print "Max iterations %d reached % (i)"
                                break

                        #print the owner of the list casted as TCB_t
                        tcb = item["pvOwner"].cast(TCB_t_p).dereference()
                        pxTopOfStack = tcb["pxTopOfStack"]
                        pc = int((pxTopOfStack + self.regs_stack_to_index('pc')).cast(uint32_p).dereference())
                        lr = int((pxTopOfStack + self.regs_stack_to_index('lr')).cast(uint32_p).dereference())
                        print "pvOwner@%s: %s" % (item["pvOwner"], tcb)
                        print "PC @ 0x%x ---> %s" % (pc, gdb.find_pc_line(pc))
                        print "LR @ 0x%x ---> %s" % (lr, gdb.find_pc_line(lr))
                        print "--------------------------------------------------------------------"
                        uxNumberOfItems -= 1
                        item = item["pxNext"]
                        i += 1

                #at this point the list has been iterated so there must be 0 items left
                if (uxNumberOfItems):
                        print "List corruption: Iterated  %d times uxNumberOfItems %d" % (i, uxNumberOfItems)

IterateFreeRtosListCommand()


class IterateFreeRtosHeapCommand(gdb.Command):

        def __init__(self):
                super(IterateFreeRtosHeapCommand, self).__init__("iterate-freertos-heap",
                                                                 gdb.COMMAND_DATA,
                                                                 gdb.COMPLETE_SYMBOL)
        def free_memblocks_stats(self):
                xStart = gdb.parse_and_eval("xStart") #start of free block list

                pxNextFreeBlock = xStart["pxNextFreeBlock"]

                found = False
                max_xBlockSize = -1
                sum_of_free_blocks = 0
                while pxNextFreeBlock != 0:
                        xBlockSize = int(pxNextFreeBlock["xBlockSize"])
                        sum_of_free_blocks += xBlockSize
                        if (xBlockSize > max_xBlockSize):
                                max_xBlockSize = xBlockSize
                        # print("pxNextFreeBlock@%s %s" % (pxNextFreeBlock, pxNextFreeBlock.dereference()))
                        pxNextFreeBlock = pxNextFreeBlock["pxNextFreeBlock"]

                fragmentation = 1 - max_xBlockSize / float(sum_of_free_blocks)
                print("Max size of a free block: %d bytes" % max_xBlockSize)
                print("Sum of free blocks: %d bytes" % (sum_of_free_blocks))
                print("Heap fragmentation percentage: %.2f%%" % (100 * fragmentation))

        def print_memblock(self, memblock, b_size):
                #TODO merge it in one print
                print "memblock@%s size %s {" % (memblock.address, b_size)
                print "  pxNextFreeBlock = %s," % memblock["pxNextFreeBlock"]
                print "  xBlockSize = 0x%x" % int(memblock["xBlockSize"])
                print "}"

        def invoke(self, argument, from_tty):
                args = gdb.string_to_argv(argument)
                expr = args[0]
                ucHeap =  gdb.parse_and_eval(expr)
                xBlockAllocatedBit = gdb.parse_and_eval("xBlockAllocatedBit")
                BlockLink_t = gdb.lookup_type("BlockLink_t") #BlockLink_t

                used_heap = 0

                #allocated blocks should be found in a aligned location
                for b in range(0, ucHeap.type.sizeof, 4):
                        memblock = ucHeap[b].cast(BlockLink_t)
                        is_alloc = bool(int(memblock["xBlockSize"]) & int(xBlockAllocatedBit))
                        is_alloc = is_alloc and bool(memblock["pxNextFreeBlock"] == 0x0)

                        if (is_alloc):
                                b_size =  int(memblock["xBlockSize"]) & ~int(xBlockAllocatedBit)
                                #does the block size look sane
                                # if (b * 4 + b_size < ucHeap.type.sizeof):
                                if (b_size < ucHeap.type.sizeof):
                                        used_heap = used_heap + b_size
                                        self.print_memblock(memblock, b_size)
                #Print FreeRTOS heap stats
                print("")
                print("Heap size: %d bytes" %ucHeap.type.sizeof)
                print("Sum of allocated heap blocks: %d bytes" % (used_heap))
                self.free_memblocks_stats()



IterateFreeRtosHeapCommand()

class PrintFreeRtosTaskStatusCommand(gdb.Command):

        def __init__(self):
                super(PrintFreeRtosTaskStatusCommand, self).__init__("print-freertos-task-status",
                                                                     gdb.COMMAND_DATA,
                                                                     gdb.COMPLETE_SYMBOL)
        def invoke(self, argument, from_tty):
                pxTaskStatusArray = gdb.parse_and_eval("pxTaskStatusArray")

                i = 0
                sum = 0
                print "Name   xTaskNumber    xHandle    eCurrentState  uxCurrentPriority uxBasePriority ulRunTimeCounter usStackHighWaterMark(words)  usStackHighWaterMark(bytes)\n"
                while pxTaskStatusArray[i]["pcTaskName"] != 0:
                        xHandle = pxTaskStatusArray[i]["xHandle"]
                        pcTaskName = str(pxTaskStatusArray[i]["pcTaskName"])
                        name = re.findall("(\".*\")", pcTaskName)
                        xTaskNumber = pxTaskStatusArray[i]["xTaskNumber"]
                        eCurrentState = pxTaskStatusArray[i]["eCurrentState"]
                        uxCurrentPriority = pxTaskStatusArray[i]["uxCurrentPriority"]
                        uxBasePriority =  pxTaskStatusArray[i]["uxBasePriority"]
                        ulRunTimeCounter = pxTaskStatusArray[i]["ulRunTimeCounter"]
                        usStackHighWaterMark = pxTaskStatusArray[i]["usStackHighWaterMark"]

                        print "%5s %5s %17s %14s %10s %15s %15s %20s %30s" % (name[0],
                                                                              xTaskNumber,
                                                                              xHandle,
                                                                              eCurrentState,
                                                                              uxCurrentPriority,
                                                                              uxBasePriority,
                                                                              ulRunTimeCounter,
                                                                              usStackHighWaterMark,
                                                                              usStackHighWaterMark * 4)

                        sum += usStackHighWaterMark
                        i += 1

                print ""
                print "                                                                                                  +                            +"
                print "                                                                                                  ---------------------------  ---------------------------\n"
                print "%110s %30s" % (sum, sum * 4)

PrintFreeRtosTaskStatusCommand()

# NVIC registers. The addresses noted here are the base addresses (i.e. the addresses of the
# first elements) of each register set/array.
NVIC_REGISTERS = {
    'ISER': 0xE000E100, # Interrupt Set Enable Registers
    'ISPR': 0XE000E180, # Interrupt Set Pending Registers
    'IABR': 0xE000E300, # Interrupt Active Bit Registers
    'IPR' : 0xE000E400, # Interrupt Priority Registers
}

# Currently, DA14xxx devices have less than 64 peripherals, so only two registers in each
# NVIC-Registers set are actually used.
nvic_array_length = 2

def fetch_reg(name, addr):
    reg_value = int(gdb.execute("x /1xw 0x{:08x}".format(addr), to_string=True).split(':', 1)[1].strip(), 0)
    gdb.write('{:40}'.format(name + ':') + '(address: 0x{:08x}) '.format(addr) + 'hex: 0x{:08x} '.format(reg_value) + ' bin: {0:032b}\n'.format(reg_value))


class NVICReadRegisters(gdb.Command):
    def __init__(self):
        super(NVICReadRegisters, self).__init__("nvic-reg", gdb.COMMAND_STATUS, gdb.COMPLETE_NONE)

    # noinspection PyUnusedLocal, PyMethodMayBeStatic
    def invoke(self, arg, from_tty):
        # First fetch the content of AIRCR (Application Interrupt and Reset Control Register) which
        # contains the PRIGROUP field.
        fetch_reg('AIRCR (PRIGROUP is in bits [10:8])', 0xE000ED0C)
        # Then, fetch all the NVIC registers.
        for name, addr in NVIC_REGISTERS.iteritems():
            for ii in range(nvic_array_length):
                fetch_reg(name + '[' + str(ii) + ']', addr + (ii * 4))

NVICReadRegisters()
