void main(void);
extern u8 _ramstart, _ramend, _raminit;
void raminit(void) {
	memcpy(&_ramstart, &_raminit, &_ramend-&_ramstart);
	main();
}
