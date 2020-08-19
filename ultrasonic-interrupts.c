#define TRIGGER BIT(D18)
#define ECHO    BIT(D19)

#define SOURCE (8+D19)

#define PRIORITYPTR (u32 volatile *)(0x0c000000 + 4*SOURCE)
#define ENABLEPTR   ((u64 volatile *)0x0c002000)
#define CLAIMPTR    ((u32 volatile *)0x0c200004)
#define THRESHPTR   ((u32 volatile *)0x0c200000)

#define SCALE 0x100

struct pwm  volatile *const pwm1 = (void *)PWM1ADDR;
struct pwm  volatile *const pwm2 = (void *)PWM2ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

void intensity(u16 value) {
	pwm1->pwmcmp2 = value;
}

void pwm1init(void) {
	pwm1->pwmcmp0 = SCALE;     /* length of PWM cycle */
	pwm1->pwmenalways = 1;     /* keep running PWM cycles */
	pwm1->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm1->pwmscale = 0;        /* go at full speed */
	pwm1->pwmsticky = 0;       /* let the signal deassert */
}

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

	gpio->output_en  |= BIT(BLED);
	gpio->iof_en     |= BIT(BLED);
	gpio->out_xor    |= BIT(BLED);
	gpio->iof_sel    |= BIT(BLED);
}

u64 delta = 0;
__attribute__ ((noinline)) INTERRUPT(tisr) {
	mtimecmpwr(mtimecmprd() + 0x800);
	if (delta) {
		intensity(delta>>6);
		printdword(delta>>6);
		printchar('\n');
		delta = 0;
	}
}

u64 start = 0;
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

void pwm2init(void) {
	pwm2->pwmcfg = 0;
	pwm2->pwmcmp0 = 1000;     /* half a second if a unit is 160 cycles */
	pwm2->pwmcmp2 = 1000-5;   /* on for 5*32 = 160 cycles every 0.1 s */
	pwm2->pwmenalways = 1;     /* keep running PWM cycles */
	pwm2->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm2->pwmscale = 5;        /* 32 cycles is a unit */
	pwm2->pwmsticky = 0;       /* let the signal deassert */
}

int main(void) {
	gpioinit();
	pwm1init();
	pwm2init();

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
