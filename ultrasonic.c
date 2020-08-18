/*
 * Give it a 10 us pulse, it responds with a pulse that is proportional in
 * length to the time between send/receive.
 * 1. How to make sure our pulse is 10 us?
 * 2. How to measure the duration of their pulse?
 *
 * If the clock is 16MHz, then 16 cycles is 1 us, so 160 cycles is 10 us.
 * Maybe we could set the pin, sleep for 160 cycles, then clear. I'm sure I'll
 * get higher accuracy if I increase the clock frequency...
 *
 * To measure the duration of the response pulse, should I instead try
 * interrupts? The good thing is the clock is actually in multiples of 1 MHz.
 * How long does it take to print the clock I wonder? Actually a long time
 * because UART is slow. Let's just measure then print.
 * Sure, the difference is 2 cycles because that's the distance between the two
 * instructions that read the low word of the cycle register.
 * Let's now insert a sleep and see how accurate the sleep is.
 * Not very accurate... It slept for 2500 cycles when I told it 160. When I
 * told it 3000 it slept 3015. So below that it's inaccurate.
 * I could use PWM to emit the pulses at most every second looks like. But it
 * should be pretty accurate. But then how do I read them?
 * What distance does sound cover in one cycle?
 * I've got about 16e6/(343*100) = 466 cycles per centimeter. That's not bad
 * actually. I need two interrupts to record the edge transition.
 * Ok, I need three things:
 * - configure a PWM pulse that lasts 160 cycles every second (for now)
 * - configure two external interrupts (one on rising edge, one on falling
 *   edge) that record the cycle counter into some global variable
 * - configure a timer interrupt to print the start/end difference
 *
 * I think these three things would not interfere much with each other. The PWM
 * pulse runs in the background, the external interrupts do very little and in
 * addition by default interrupts don't interrupt each other.
 * The timer interrupt would also need to check whether current data is valid.
 * I think the interrupt on the rising edge would need to invalidate it.
 * Ok, let's start with a PWM pulse. We can verify it using the builtin LED.
 */

#define TRIGGER BIT(D18)
#define ECHO    BIT(D19)

#define SOURCE (8+D19)

#define PRIORITYPTR (u32 volatile *)(0x0c000000 + 4*SOURCE)
#define ENABLEPTR   ((u64 volatile *)0x0c002000)
#define CLAIMPTR    ((u32 volatile *)0x0c200004)
#define THRESHPTR   ((u32 volatile *)0x0c200000)

struct pwm  volatile *const pwm2 = (void *)PWM2ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

void gpioinit(void) {
	/* GPIO input */
	gpio->input_en = ECHO;
	gpio->pue      |= ECHO;
	gpio->iof_en   &= ~ECHO;
	gpio->rise_ie  |= ECHO;
	gpio->fall_ie  |= ECHO;

	/* PWM output */
	gpio->iof_en   |= TRIGGER; /* use it for PWM  */
	gpio->iof_sel  |= TRIGGER; /* PWM */
}

u64 delta = 0;
__attribute__ ((noinline)) INTERRUPT(tisr) {
	mtimecmpwr(mtimecmprd() + 0x4000);
	if (delta) {
		printdword(delta);
		printchar('\n');
		delta = 0;
	}
}

u64 start = 0;
u32 count = 0;
__attribute__ ((noinline)) INTERRUPT(eisr) {
	u32 claim = *CLAIMPTR;
	if (gpio->rise_ip & ECHO) {
		start = cycle();
		gpio->rise_ip |= ECHO;
	}
	if (gpio->fall_ip & ECHO) {
		delta = cycle() - start;
		gpio->fall_ip |= ECHO;
	}
	*CLAIMPTR = claim;
}

void __attribute__ ((naked, aligned(64))) vector(void) {
	/*
	 * I tried calling ("j isr") from assembly, but I got
	 * "undefined reference". Therefore I made isr noinline and am calling
	 * it directly here. It does compile to single jump instruction.
	 */
	__asm__ volatile ( ".skip 0x1c");
	tisr();
	__asm__ volatile ( ".skip 0xc");
	eisr();
}

void pwminit(struct pwm volatile *pwm) {
	pwm->pwmcfg = 0;
	pwm->pwmcmp0 = 50000;     /* half a second if a unit is 160 cycles */
	pwm->pwmcmp2 = 50000-5;   /* on for 5*32 = 160 cycles every 0.1 s */
	pwm->pwmenalways = 1;     /* keep running PWM cycles */
	pwm->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm->pwmscale = 5;        /* 32 cycles is a unit */
	pwm->pwmsticky = 0;       /* let the signal deassert */
}

int main(void) {
	gpioinit();
	pwminit(pwm2);

	/* this needs clean up */
	*PRIORITYPTR = 7;
	*ENABLEPTR = BIT(SOURCE);
	*THRESHPTR = 0;

	/* Why are these initialized to ~0 otherwise? */
	start = 0;
	delta = 0;

	miewr(mierd() | BIT(7) | BIT(11)); /* enable timer + external */
	mtimecmpwr(0);                     /* run timer interrupt ASAP */
	mtvecwr((u32)vector | 1);          /* vectored mode */
	mstatuswr(mstatusrd() | BIT(3));   /* global interrupt enable */

	for (;;)
		__asm__ volatile ("wfi");
}
