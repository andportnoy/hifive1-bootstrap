struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

enum {
	rs=BIT(D16), e=BIT(D17),
	d0=BIT(D10),d1=BIT(D9), d2=BIT(D7), d3=BIT(D6),
	d4=BIT(D5), d5=BIT(D4), d6=BIT(D3), d7=BIT(D2),
};

#define MASK (rs|e|d0|d1|d2|d3|d4|d5|d6|d7)

void pulse(void) {
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
		pulse();
		sleep(16e4);
	}
}

char *asciidec(u32 val, char *buf, int n) {
	buf[--n] = 0;
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
	pulse();
	gpio->output_val = val;
}

int main(void) {
	/* give the lcd some time */
	sleep(16e5);

	gpio->iof_en &= ~MASK;
	gpio->output_en = MASK;
	gpio->output_val = 0;

	lcdbyte(0x38);
	pulse();
	sleep(16e5);

	/* screen on, no cursor, no cursor position */
	lcdbyte(0x0c);
	pulse();
	sleep(16e5);

	lcdbyte(0x01);
	pulse();
	sleep(16e5);

	gpio->output_val = rs;
	lcdprint("hello");
	sleep(16e6);

	char buf[16] = {0};
	for (int i=0; i<1000; ++i) {
		lcdclear();
		sleep(16e4);
		lcdprint(asciidec(i, buf, sizeof buf));
		sleep(8e6);
	}
}
