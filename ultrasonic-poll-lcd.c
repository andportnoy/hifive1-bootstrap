#define TRIGGER BIT(D18)
#define ECHO    BIT(D19)

struct pwm  volatile *const pwm1 = (void *)PWM1ADDR;
struct pwm  volatile *const pwm2 = (void *)PWM2ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

enum {
	rs=BIT(D16), e=BIT(D17),
	d0=BIT(D10),d1=BIT(D9), d2=BIT(D7), d3=BIT(D6),
	d4=BIT(D5), d5=BIT(D4), d6=BIT(D3), d7=BIT(D2),
};

#define MASK (rs|e|d0|d1|d2|d3|d4|d5|d6|d7)

void lcdpulse(void) {
	gpio->output_val |= e;
	/* need 140 ns, one cycle is about 60 ns */
	__asm__ volatile ("addi zero,zero,0");
	__asm__ volatile ("addi zero,zero,0");
	__asm__ volatile ("addi zero,zero,0");
	__asm__ volatile ("addi zero,zero,0");
	__asm__ volatile ("addi zero,zero,0");
	gpio->output_val &= ~e;
}

void lcdbyte(char c) {
	gpio->output_val =  (gpio->output_val & ~(d7|d6|d5|d4|d3|d2|d1|d0)) |
			    (c&BIT(0)? d0: 0) |
		            (c&BIT(1)? d1: 0) |
		            (c&BIT(2)? d2: 0) |
		            (c&BIT(3)? d3: 0) |
		            (c&BIT(4)? d4: 0) |
		            (c&BIT(5)? d5: 0) |
		            (c&BIT(6)? d6: 0) |
		            (c&BIT(7)? d7: 0);
}

void lcdprint(char *s) {
	while (*s) {
		lcdbyte(*s++);
		lcdpulse();
		sleep(16e3);
	}
}

char *asciidec(u32 val, char *buf, int n) {
	do {
		buf[--n] = '0'+val%10;
		val /= 10;
	} while (val);
	return &buf[n];
}

void lcdclear(void) {
	u32 val = gpio->output_val;
	gpio->output_val &= ~rs;
	lcdbyte(1);
	lcdpulse();
	gpio->output_val = val;
}

void lcdinit(void) {
	lcdbyte(0x38);
	lcdpulse();
	sleep(16e4);

	/* screen on, no cursor, no cursor position */
	lcdbyte(0x0c);
	lcdpulse();
	sleep(16e4);

	lcdbyte(0x01);
	lcdpulse();
	sleep(16e4);

	/* data mode */
	gpio->output_val = rs;
	sleep(16e4);
}

int main(void) {
	gpio->iof_en &= ~MASK;
	gpio->output_en = MASK;
	gpio->output_val = 0;

	gpio->iof_en  |= TRIGGER; /* use it for PWM  */
	gpio->iof_sel |= TRIGGER; /* PWM */

	gpio->input_en = ECHO;
	gpio->pue     |= ECHO;
	gpio->iof_en  &= ~ECHO;

	lcdinit();

	pwm2->pwmcfg = 0;
	pwm2->pwmcmp0 = 6;         /* 1 out of 6 down, 0 out of 5 didn't work */
	pwm2->pwmcmp2 = 1;         /* on for 5*32 = 160 cycles */
	pwm2->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm2->pwmscale = 5;        /* 32 cycles is a unit */
	pwm2->pwmsticky = 0;       /* let the signal deassert */

	u32 prevcm = 0;
	for (;;) {
		pwm2->pwmenoneshot = 1;    /* fire a PWM pulse */

		while (!(gpio->input_val&ECHO));
		u64 start = cycle();
		while (gpio->input_val&ECHO);
		u64 end = cycle();

		u32 delta = end-start;

		/* 466 cycles per cm at 343 m/s, 16 MHz;
		 * times two for the round trip
		 */
		u32 cm = delta / (2*466);

		if (cm != prevcm) {
			char buf[] = {
				' ', ' ', ' ', ' ',
				' ', ' ', ' ', ' ',
				' ', ' ', ' ', ' ',
				'c', 'm', '\0'};
			(void)asciidec(cm, buf, 11);

			lcdclear();
			sleep(16e4);
			lcdprint(buf);

			prevcm = cm;
		}

		printword(delta);
		printchar('\n');
		sleep(8e6); /* 100 ms */
	}
}
