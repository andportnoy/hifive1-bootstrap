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

struct pwm  volatile *const pwm2 = (void *)PWM2ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

void gpioinit(void) {
	/* GPIO input */
	gpio->input_en |= ECHO;
	gpio->iof_en   &= ~ECHO;
	gpio->pue      |= ECHO;
	gpio->rise_ie  |= ECHO;

	/* PWM output */
	gpio->iof_en   |= TRIGGER; /* use it for PWM  */
	gpio->iof_sel  |= TRIGGER; /* PWM */
}

__attribute__ ((noinline)) INTERRUPT(isr) {
	mtimecmpwr(mtimecmprd() + 0x1000);
	printcycle();
	printchar('\n');
}

void __attribute__ ((naked, aligned(64))) vector(void) {
	/*
	 * I tried calling ("j isr") from assembly, but I got
	 * "undefined reference". Therefore I made isr noinline and am calling
	 * it directly here. It does compile to single jump instruction.
	 */
	__asm__ volatile ( ".skip 0x1c");
	isr();
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
	miewr(mierd() | BIT(7));         /* enable machine timer interrupts */
	mtimecmpwr(0);                   /* set first interrupt at 3 seconds */
	mtvecwr((u32)vector | 1);        /* set interrupt handler address */
	mstatuswr(mstatusrd() | BIT(3)); /* global interrupt enable */

	/* I want a 160 cycle pulse every second.
	 * So I wait 16e6-160 cycles, then do a pulse for 160 seconds.
	 * I could scale this to take 160 cycles as a unit. So the scale value
	 * would be 0xa0. A second is how many of these? 1e5=100000, I think.
	 * So I need to wait for 99999 units, then go live for 1. Is that
	 * possible? Nope, I can wait for a maximum of 65535 units, unless I do
	 * some extra tricks. Ok, that's fine, let's do a pulse every half a
	 * second. So I need 50k units.
	 * I think the scale is powers of two, not literal scale. So the best I
	 * can do probably is scale=5, because then 160/2^5 = 5. So my pulse
	 * lasts 5 cycles. Ok, let's do that.
	 * 32 * 50000 is 16*100000=16e5, so 0.1 second. So I'l have to do a
	 * pulse once every 1/10th of a second. Ok, that's fine. Maybe too
	 * frequent for debugging purposes.
	 * Yeah, it's blinking. So that works fine. Does my pin support PWM?
	 * 19 digital, yes it does. Which pwm unit is that? 19 dig is GPIO 13,
	 * which corresponds to PWM2_PWM3. Alrighty. Let's test it with an
	 * external LED. Actually, I'll use echo for that, I need PWM2_PWM2.
	 * Ok, it's working. Maybe I'll need an external power supply because
	 * the voltage regulator is getting hot.
	 * Now let's try to get an external interrupt to work. We'll test it on
	 * a button.
	 *
	 * + enable GPIO input on that pin
	 * + enable pull up resistor on that pin
	 * + enable GPIO interrupt on rising edge
	 * - place an external interrupt handler at appropriate address
	 *   - remember to set (clear?) the rise_ip bit
	 * + set mtvec, enabling vectored mode
	 * + enable machine external interrupts
	 * + enable global interrupts
	 *
	 * Hmm, ok I need to place a jump instruction to the isr. How to do
	 * this? I basically want to place a specific piece of assembly code at
	 * a specific location.
	 * Actually, I'm overthinking it. I can place the table anywhere, it's
	 * just the jump needs to be at a specific position in the table.
	 * So I can just write the table in assembly as a naked function, then
	 * use the function pointer to set mtvec.
	 * Let's test this with a timer interrupt.
	 */


#if 0
	miewr(mierd() | BIT(11));        /* enable machine external interrupts */
	mtvecwr((u32)isr);               /* enable vectored mode */
	mstatuswr(mstatusrd() | BIT(3)); /* global interrupt enable */
#endif

	pwminit(pwm2);
	pwmcfgprint(pwm2);
}
