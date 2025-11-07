





        MEMORY
        {
                ROM (rx) : ORIGIN = (0x0), LENGTH = ((120 * 1024))
                RAM (rw) : ORIGIN = (0x20000000 + 0x200), LENGTH = ((96 * 1024 - ((1 << ((6) + 4))) - ((0))) - 0x200)
        }
