struct uart volatile *const uart = (void *)UARTADDR;
struct prci volatile *const prci = (void *)PRCIADDR;

/*
 * Let's take the 16 MHz oscillator and transform as follows:
 * pllr {1}:  16MHz/ 2 -> 8MHz
 * pllf {39}:  8MHz*80 -> 640MHz
 * pllq {1}: 640MHz/ 2 -> 320MHz
 *
 * Actually, for now let's try:
 * pllr {1}: 16MHz/2 ->  8MHz
 * pllf {3}:  8MHz*8 -> 64MHz
 * pllq {1}: 64MHz/2 -> 32MHz
 *
 * To view output on the serial port,
 * run with baudrate 230400 = 2*115200.
 */
void main(void) {
	prci->pllsel = 0;
	prci->pllr = 1;
	prci->pllf = 3;
	prci->pllq = 1;
	prci->pllsel = 1;
	prci->pllbypass = 0;
	for (;; sleep(16e6)) {
		printcycle();
		printchar('\n');
	}
}
