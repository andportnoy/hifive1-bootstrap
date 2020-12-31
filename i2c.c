struct uart volatile *const uart = (void *)UARTADDR;
struct i2c  volatile *const i2c  = (void *)I2CADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

int main(void) {
	printword(gpio->iof_en);
	printchar('\n');
	printword(gpio->iof_sel);
	printchar('\n');

	gpio->iof_en  |=   BIT(12)|BIT(13);
	gpio->iof_sel &= ~(BIT(12)|BIT(13));
	gpio->pue     |=   BIT(12)|BIT(13);

	printword(gpio->iof_en);
	printchar('\n');
	printword(gpio->iof_sel);
	printchar('\n');

	i2cprint(i2c);
	i2c->ctr = BIT(7);
	i2c->prerlo = 0x80;
	i2c->prerhi = 0x00;
	i2c->cr = BIT(7) | BIT(5);
}
