/*
 * Want to replicate external interrupt working, it's failing for some reason
 * in the ultrasonic code.
 * Here I just want to get a button press to work.
 * I've got a button sitting on dig 2, which corresponds to GPIO 18.
 * First let's just make sure it works as an input without any interrupts.
 */
#define BUTTON D2

#define SOURCE (8+BUTTON)

#define PRIORITYPTR (u32 volatile *)(0x0c000000 + 4*SOURCE)
#define ENABLEPTR   ((u32 volatile *)0x0c002000)
#define CLAIMPTR    ((u32 volatile *)0x0c200004)
#define PENDINGPTR  ((u32 volatile *)0x0c001000)

struct uart volatile *const uart = (void *)UART0ADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

void gpioinit(void) {
	gpio->input_en = BIT(BUTTON);
	gpio->pue      = BIT(BUTTON);
}

u64 start = 0;
__attribute__ ((noinline)) INTERRUPT(eisr) {
	u32 claim = *CLAIMPTR;
	print("button press\n");
	*CLAIMPTR = claim;
}

void __attribute__ ((naked, aligned(64))) vector(void) {
	__asm__ volatile ( ".skip 0x2c");
	eisr();
}

/*
 * + enable GPIO input on that pin
 * + enable GPIO interrupt on rising AND falling edge
 * - place an external interrupt handler at appropriate address
 *   - remember to set (clear?) the rise_ip bit
 * + set mtvec, enabling vectored mode
 * + enable machine external interrupts
 * + enable global interrupts
 *
 * Apparently I need to configure PLIC too.
 * + set the priority to a nonzero value for source 21 (GPIO 13/DIG 19)
 * + set bit 21 in the interrupt enable register
 * - in the interrupt handler:
 *   - read claim register in the beginning
 *   - write same value to claim register at the end of the routine
 *   - does the gpio interrupt pending bit also need to be cleared/set?
 *
 * I think I'm seeing that the hardware interrupt is not getting triggered.
 * Let's go through a systematic check for what's causing the issue.
 */
int main(void) {
	gpioinit();

	*PRIORITYPTR = 7;
	*ENABLEPTR = 0;
	*(ENABLEPTR+1) = 0;
	*ENABLEPTR |= BIT(SOURCE);

	/*
	miewr(mierd() | BIT(11));
	mtvecwr((u32)vector | 1);
	mstatuswr(mstatusrd() | BIT(3));
	*/

	for (;;) {
		printword(gpio->input_val);
		printchar('\n');
		sleep(16e6);
	}
}
