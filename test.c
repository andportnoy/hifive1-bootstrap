struct uart volatile *const uart0 = (void *)UART0ADDR;

int volatile test = 0xdeadbeef;
int main(void) {
	printword(test);
	printchar('\n');
}
