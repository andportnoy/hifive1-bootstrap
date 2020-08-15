#define SLEEP 0x4000

struct gpio volatile *const gpio = (void *)GPIOADDR;

enum { o0=BIT(D9) };

void ledtoggle(void) {
	gpio->output_val ^= o0;
}

INTERRUPT(isr) {
	mtimecmpwr(mtimecmprd() + SLEEP);
	ledtoggle();
}

void gpioinit(void) {
	gpio->output_en |= o0;
}

int main(void) {
	gpioinit();
	timerinit(isr);

	for (;;)
		__asm__ ("wfi"); /* wait for interrupt */
}
