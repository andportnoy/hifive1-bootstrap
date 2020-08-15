struct uart volatile *const uart = (void *)UARTADDR;

__attribute__ ((interrupt, aligned(64))) void isr(void) {
	printdword(mtimerd());
	printchar('\n');
	mtimecmpwr(mtimecmprd() + 0x8000);
}

int main(void) {
	miewr(mierd() | BIT(7));         /* enable machine timer interrupts */
	mtimecmpwr(3*0x8000);            /* set first interrupt at 3 seconds */
	mtvecwr((u32)isr);               /* set interrupt handler address */
	mstatuswr(mstatusrd() | BIT(3)); /* global interrupt enable */
}
