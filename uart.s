/*
 * The goal is to try to write something to the console over UART.
 * Steps:
 * - enable transmit (set the low bit at offset 0x8)
 * - write to transmit (offset 0x0)
 */

.globl _start
.text

_start:
	lui	a0, 0x10013
	addi	a1, zero, 1
	sw	a1, 0x8(a0)
	addi	a1, zero, 0x42
	sb	a1, 0(a0)
	sb	a1, 0(a0)
	sb	a1, 0(a0)
	sb	a1, 0(a0)
	addi	a1, zero, 0xa
	sb	a1, 0(a0)
	addi	a1, zero, 0xd
	sb	a1, 0(a0)
	addi	a1, zero, 0xa
	sb	a1, 0(a0)
	addi	a1, zero, 0xd
	sb	a1, 0(a0)
