.globl _start

_start:
	jal	ra, fib
fib:
	addi	a4, zero, 8
	lui	a0, 0x80000
	lw	a1, (a0)
	addi	a0, a0, 4
	lw	a2, (a0)
loop:
# compute next value
	add	a3, a1, a2
# store
	addi	a0, a0, 4
	sw	a3, (a0)
# update for next iteration
	add	a1, zero, a2
	add	a2, zero, a3
	addi	a4, a4, -1
	bne	a4, zero, loop
# print to uart
	lui	a0, 0x10013
	addi	a1, zero, 1
	sw	a1, 0x8(a0)
	add	a1, zero, a3
	sb	a1, 0(a0)
	addi	a1, zero, 0xa
	sb	a1, 0(a0)
	addi	a1, zero, 0xd
	sb	a1, 0(a0)
	addi	a1, zero, 0xa
	sb	a1, 0(a0)
	addi	a1, zero, 0xd
	sb	a1, 0(a0)
	ebreak
.data
.word 1
.word 1
