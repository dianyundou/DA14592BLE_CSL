/* Linker script to configure memory regions.
 * Needs modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 *   RAM_C0.ORIGIN: starting address of M33 cache area
 *   RAM_C0.LENGTH: length of M33 cache area
 *   RAM_C1.ORIGIN: starting address of CMAC cache area
 *   RAM_C1.LENGTH: length of CMAC cache area
 */

MEMORY
{
        RAM (rwx)    : ORIGIN = 0x0,        LENGTH = 3K
        BUFFER (rw)  : ORIGIN = 0xC00,      LENGTH = 93K
        RAM_C0 (rwx) : ORIGIN = 0x818000,   LENGTH = 8K
        RAM_C1 (rw)  : ORIGIN = 0x40108000, LENGTH = 8K
}
