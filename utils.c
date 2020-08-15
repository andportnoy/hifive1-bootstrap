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

u64 mtimerd(void) {
	return *MTIMEPTR;
}

void mtimewr(u64 v) {
	*MTIMEPTR = v;
}

u64 mtimecmprd(void) {
	return *MTIMECMPPTR;
}

void mtimecmpwr(u64 v) {
	*MTIMECMPPTR = v;
}

CSR32RD_DEF(mtvec)
CSR32WR_DEF(mtvec)

CSR32RD_DEF(mie)
CSR32WR_DEF(mie)

CSR32RD_DEF(mstatus)
CSR32WR_DEF(mstatus)

CSR32RD_DEF(mcause)
CSR32WR_DEF(mcause)

CSR32RD_DEF(mtval)
CSR32WR_DEF(mtval)

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

void printdword(u64 dw) {
	printword(dw>>32);
	printword(dw);
}

void printcycle(void) {
	u64 cur = cycle();
	printword(cur>>32);
	printword(cur);
}

void uartinit(void) {
	uart->txctrl |= 1;
}

void timerinit(void (*isr)(void)) {
	miewr(mierd() | BIT(7));         /* enable machine timer interrupts */
	mtimecmpwr(0);                   /* trigger interrupt ASAP */
	mtvecwr((u32)isr);               /* set interrupt handler address */
	mstatuswr(mstatusrd() | BIT(3)); /* global interrupt enable */
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

void mcauseprint(u32 v) {
	if (v & BIT(31)) {
		switch (v & 0x3ff) {
		case  3: print("software interrupt\n");     break;
		case  7: print("timer interrupt\n");        break;
		case 11: print("external interrupt\n");     break;
		default: print("unknown interrupt type\n"); break;
		}
	} else {
		print("exception: ");
		switch (v & 0x3ff) {
		case  1: print("instruction access fault\n");     break;
		case  2: print("illegal instruction\n");          break;
		case  3: print("breakpoint\n");                   break;
		case  4: print("load address misaligned\n");      break;
		case  5: print("load access fault\n");            break;
		case  6: print("store/AMO address misaligned\n"); break;
		case  7: print("store/AMO access fault\n");       break;
		case  8: print("environment call from U-mode\n"); break;
		case 11: print("environment call from M-mode\n"); break;
		default: print("unknown exception type\n");       break;
		}
	}
}
