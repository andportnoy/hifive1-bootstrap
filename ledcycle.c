struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

enum {
	o0=BIT(D9), o1=BIT(D8), o2=BIT(D7), o3=BIT(D6),
	o4=BIT(D5), o5=BIT(D4), o6=BIT(D3), o7=BIT(D2)
};

enum {
	i0=BIT(D10),
};

void ledbyte(u8 byte) {
	gpio->output_val =  (gpio->output_val & ~(o7|o6|o5|o4|o3|o2|o1|o0)) |
			    (byte&BIT(0)? o0: 0) |
		            (byte&BIT(1)? o1: 0) |
		            (byte&BIT(2)? o2: 0) |
		            (byte&BIT(3)? o3: 0) |
		            (byte&BIT(4)? o4: 0) |
		            (byte&BIT(5)? o5: 0) |
		            (byte&BIT(6)? o6: 0) |
		            (byte&BIT(7)? o7: 0);
}

__attribute__ ((interrupt, aligned(64))) void isr(void) {
	mcauseprint(mcauserd());
	printword(mtvalrd());
	printchar('\n');
}

int pressednow = 0, pressedbefore = 0;
int main(void) {
	mtvecwr((u32)isr);               /* set interrupt handler address */
	mstatuswr(mstatusrd() | BIT(3)); /* global interrupt enable */
	gpio->output_en |= o7|o6|o5|o4|o3|o2|o1|o0;
	gpio->input_en  |= i0;
	gpio->pue       |= i0; /* internal pull up resistor enable */

	const int nap = 0x100000;
	int len = nap;
	u8 byte = 1;
	ledbyte(byte);
	for (u64 start, end, diff=nap;; diff=end-start) {
		start = cycle();
		pressedbefore = pressednow;
		pressednow = !(gpio->input_val&i0)? pressednow+1: 0;
		int buttonpress = (!pressednow && pressedbefore) |
			          (pressednow > 6);
		if (buttonpress) {
			print("Button pressed!\n");
			byte = (byte<<1)|(byte>>7);
			ledbyte(byte);
		} else
			printchar('\n');
		len -= diff-nap+6 /* diff=end-start takes 6 cycles? */;
		sleep(len);
		end = cycle();
	}
}
