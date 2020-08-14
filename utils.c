extern struct uart volatile *const uart;

void *memcpy(void *dest, const void *src, size_t n) {
	u8 *d = dest;
	const u8 *s = src;
	for (size_t i=0; i<n; ++i)
		d[i] = s[i];
	return dest;
}

u64 cycle(void) {
	u32 l, h;
	__asm__ volatile (
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

void prciprint(struct prci *prciptr) {
	struct prci val = *prciptr;

	print("ring oscillator:\n");

	print("divider: ");
	printword(val.hfroscdiv);
	printchar('\n');

	print("trim: ");
	printword(val.hfrosctrim);
	printchar('\n');

	print("enable: ");
	printword(val.hfroscen);
	printchar('\n');

	print("ready: ");
	printword(val.hfroscrdy);
	printchar('\n');

	printchar('\n');

	print("crystal oscillator:\n");

	print("enable: ");
	printword(val.hfxoscen);
	printchar('\n');

	print("ready: ");
	printword(val.hfxoscrdy);
	printchar('\n');

	printchar('\n');

	print("pll\n");

	print("pllr: ");
	printword(val.pllr);
	printchar('\n');

	print("pllf: ");
	printword(val.pllf);
	printchar('\n');

	print("pllq: ");
	printword(val.pllq);
	printchar('\n');

	print("pllsel: ");
	printword(val.pllsel);
	printchar('\n');

	print("pllrefsel: ");
	printword(val.pllrefsel);
	printchar('\n');

	print("pllbypass: ");
	printword(val.pllbypass);
	printchar('\n');

	print("plllock: ");
	printword(val.plllock);
	printchar('\n');

	print("plloutdiv: ");
	printword(val.plloutdivval);
	printchar('\n');

	print("plloutdivby1: ");
	printword(val.plloutdivby1);
	printchar('\n');
}
