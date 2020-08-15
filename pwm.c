#define SLEEP 0x4000

struct gpio volatile *const gpio = (void *)GPIOADDR;

INTERRUPT(isr) {
	mtimecmpwr(mtimecmprd() + SLEEP);
	gpio->output_val ^= BIT(GLED);
}

int main(void) {
	gpio->output_en |= BIT(GLED);
	timerinit(isr);

	for (;;)
		__asm__ ("wfi"); /* wait for interrupt */
}
