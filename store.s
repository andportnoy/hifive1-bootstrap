/*
 * Ok, the goal here is to write a value to memory, then read it from that
 * location using JTAG.
 * Steps:
 * - put an address into a register
 *   - which register? I think the a*'s are good candidates.
 *   - what is a good memory address? memory starts at 0x80000000
 * - put a value into a register
 * - store the value into the address
 */
.globl _start
_start:
	lui a0, 0x80000 # a0[11:0] will be zeroed anyway
	lui a1, 0xdeadc
	addi a1, a1, -0x111
	sw a1, 0(a0)
