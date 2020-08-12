struct uart volatile *const uart = (void *)UARTADDR;
struct prci volatile *const prci = (void *)PRCIADDR;

void prciprint(void) {
	struct prci val = *prci;

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
}

void main(void) {
	for (;; sleep(16000000)) {
		printcycle();
		printchar('\n');
	}
}
