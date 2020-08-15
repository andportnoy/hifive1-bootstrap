struct uart volatile *const uart = (void *)UARTADDR;

extern u8 _ramstart, _ramend, _raminit;
int volatile test = 0xdeadbeef;
int main(void) {
	memcpy(&_ramstart, &_raminit, &_ramend-&_ramstart);
	printword(&_ramend - &_ramstart);
	printchar('\n');
	printword(test);
	printchar('\n');
}
