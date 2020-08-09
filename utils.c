extern struct uart volatile *const uart;

u64 cycle(void) {
	u32 l, h;
	asm volatile (
		"csrrc %0, cycle,  x0\n\t"
		"csrrc %1, cycleh, x0"
		: "=r" (l), "=r" (h));
	return (u64)h<<32|l;
}

void sleep(u32 cycles) {
	u64 cur = cycle();
	while (cycle()-cur < cycles)
		;
}

void printword(u32 w) {
	for (int i=0; i<32; i+=4) {
		u8 n = (w>>(28-i))&0xf;
		printchar(n<0xa? '0'+n: 'a'-0xa+n);
	}
}

void printcycle(void) {
	u64 cur = cycle();
	printword(cur>>32);
	printword(cur);
}

void uartinit(void) {
	uart->txctrl |= 1;
}

void printchar(char c) {
	while (uart->txdata>>31)
		;
	uart->txdata = c;
}

void print(char *s) {
	while (*s)
		printchar(*s++);
}
