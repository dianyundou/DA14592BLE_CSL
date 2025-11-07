/* Linker script to configure memory regions.
 * Need modifying for a specific board.
 *   RAM.ORIGIN: starting address of read-write RAM area
 *   RAM.LENGTH: length of read-write RAM area
 *   BUFFER.ORIGIN: starting address of buffer area
 *   BUFFER.LENGTH: length of buffer area
 *   REBOOT.ORIGIN: place for function that moves buffer to beginning of RAM and starts binary
 *   REBOOT.LENGTH: length of reboot area
 */

MEMORY
{
        RAM (rwx)    : ORIGIN = 0x0,    LENGTH = 48K
        BUFFER (rw)  : ORIGIN = 48K,    LENGTH = 47K
        REBOOT (rwx) : ORIGIN = 95K,    LENGTH = 1K
}
