







        MEMORY
        {
                ROM (rx) : ORIGIN = (0x0), LENGTH = ((256 * 1024))
                RAM (rw) : ORIGIN = (0x20000000 + 0x200), LENGTH = ((96 * 1024 - ((1 << ((6) + 4))) - ((10 * 1024))) - 0x200)
        }
