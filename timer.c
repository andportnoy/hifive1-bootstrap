struct uart volatile *const uart = (void *)UARTADDR;

__attribute__ ((interrupt, aligned(64))) void isr(void) {
	printw("hello!\n");
	*((u64 volatile*)MTIMECMPADDR) += 0x8000;
}

int main(void) {
	u32 mie;
	__asm__ volatile (
		"csrrs %0, mie, zero"
		: "=r" (mie));
	printword(mie);
	printchar('\n');
	mie |= BIT(7);
	__asm__ volatile (
		"csrrw zero, mie, %0"
		:
		: "r" (mie));
	__asm__ volatile (
		"csrrs %0, mie, zero"
		: "=r" (mie));
	printword(mie);
	printchar('\n');

	*((u64 volatile*)MTIMECMPADDR) = 3*0x8000; /* 3 seconds */
	u64 mtimecmp = *((u64 volatile*)MTIMECMPADDR);
	printword(mtimecmp>>32);
	printword(mtimecmp);
	printchar('\n');

	mtvecwr((u32)isr);
	u32 mtvec = mtvecrd();
	printword(mtvec);
	printchar('\n');

	u32 mstatus;
	__asm__ volatile(
		"csrrs %0, mstatus, zero"
		: "=r" (mstatus));
	printword(mstatus);
	printchar('\n');

	mstatus |= BIT(3); /* machine interrupt enable */
	__asm__ volatile(
		"csrrw zero, mstatus, %0"
		:
		: "r" (mstatus));
}
