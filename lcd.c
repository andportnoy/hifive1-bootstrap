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

u16 read(void) {
	u32 outen = gpio->output_en;
	u32 inen = gpio->input_en;

	gpio->output_en = 0;
	gpio->input_en = MASK;

	u32 inval = gpio->input_val;

	gpio->output_en = outen;
	gpio->input_en = inen;

	return inval;
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

	lcdbyte(0x0f);
	pulse();
	sleep(16e5);

	lcdbyte(0x01);
	pulse();
	sleep(16e5);

	gpio->output_val = rs;
	sleep(16e4);
	lcdbyte('h');
	pulse();
	sleep(16e4);
	lcdbyte('e');
	pulse();
	sleep(16e4);
	lcdbyte('l');
	pulse();
	sleep(16e4);
	lcdbyte('l');
	pulse();
	sleep(16e4);
	lcdbyte('o');
	pulse();
	sleep(16e4);


	for (;;) {
		print("alive\n");
		sleep(16e6);
	}
}
