struct uart volatile *const uart = (void *)UARTADDR;
struct i2c  volatile *const i2c  = (void *)I2CADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;
struct prci volatile *const prci = (void *)PRCIADDR;

enum { STA=BIT(7), STO=BIT(6), RD=BIT(5), WR=BIT(4), ACK=BIT(3) };

int main(void) {
	gpio->iof_en  |=   BIT(12)|BIT(13);  /* enable HW function */
	gpio->iof_sel &= ~(BIT(12)|BIT(13)); /* select I2C */
	gpio->pue     |=   BIT(12)|BIT(13);  /* pull up enable */

#if 0
	1. Write slave address to TXR (shifted) ORed with 1, set STA and WR.
	2. Wait for TIP and check ACK.
	3. Write register address to TXR, set WR.
	4. Wait for TIP and check ACK.
	5. Write slave address to TXR (shifted) ORed with 0, set STA and RD.
	6. Wait for TIP and check ACK.
	7. Set the RD bit.
	8. Wait for TIP and IF.
	9. Read RXR.
	10. Repeat 7-9 if necessary.
	11. Send NACK.
	12. Generate STOP by setting STO.
#endif

	i2cprint(i2c);
	i2c->prerlo = 0x21;
	i2c->prerhi = 0x00;
	sleep(16e5);
	i2c->ctr = BIT(7);
	sleep(16e5);
	i2cprint(i2c);
	sleep(16e5);

	/* generate a START signal, writing to slave */
	i2c->txr = (0x28<<1)|0;
	i2c->cr = BIT(7) | BIT(4);

	/* wait for transfer to complete */
	while (i2c->sr&BIT(1))
		;

	print(i2c->sr&BIT(7)? "NACK\n": "ACK\n");
	i2c->cr = BIT(6);
}
