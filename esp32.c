#define SPI1ADDR 0x10024000

struct uart volatile *const uart = (void *)UARTADDR;

u32 volatile *tx = (u32 *)(SPI1ADDR+0x48);
u32 volatile *rx = (u32 *)(SPI1ADDR+0x4c);

void spiputc(char c) {
	while (*tx>>31)
		;
	*tx = c;
	printchar(c);
}

char spigetc(void) {
	u32 v = *rx;
	if (!(v>>31))
		return v;
	else
		return 0;
}

char spichar(char c) {
	spiputc(c);
	return spigetc();
}

void spiprint(char *s) {
	while(*s)
		spichar(*s++);
}

void spiread(void) {
	for (;;) {
		u32 v = *rx;
		char c = v;
		if (c)
			printchar(c);
		if (v>>31)
			break;
	}
}
int main(void) {
	printword(*tx);
	printchar('\n');
	printword(*rx);
	printchar('\n');

#define CMD "AT\r\n"

	spiprint(CMD);
	spiread();
}
