struct uart volatile *const uart = (void *)UARTADDR;

int volatile test = 0xdeadbeef;
int main(void) {
	printword(test);
}
