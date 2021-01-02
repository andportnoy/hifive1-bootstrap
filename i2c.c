struct uart volatile *const uart = (void *)UARTADDR;
struct i2c  volatile *const i2c  = (void *)I2CADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;
struct prci volatile *const prci = (void *)PRCIADDR;

enum { STA=BIT(7), STO=BIT(6), RD=BIT(5), WR=BIT(4), };

void i2cstatusprint(u8 sr) {
	print(sr&BIT(7)? "NACK": "ACK");
	print(sr&BIT(6)? " BUSY": " FREE");
	print(sr&BIT(5)? " ARB_LOST": " ARB_OK");
	print(sr&BIT(1)? " TIP_PEND": " TIP_DONE");
	print(sr&BIT(0)? " INT_PEND": " INT_DONE");
	printchar('\n');
}

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

#define START() (i2c->cr = STA | WR)
#define STOP() (i2c->cr = STO)
#define TIP   (i2c->sr&BIT(1))
#define NACK  (i2c->sr&BIT(7))
#define PUT_ADDR_WR(x) (i2c->txr = (x<<1))
#define PUT_ADDR_RD(x) (i2c->txr = ((x<<1)|1))
#define PUT(x)  (i2c->txr = x)
#define GET()   (i2c->rxr)
#define WRITE() (i2c->cr = WR)
#define READ()  (i2c->cr = RD)

	sleep(16e6);
	i2cstatusprint(i2c->sr);

	/* generate a START signal, writing to slave */
	PUT_ADDR_WR(0x28);
	START();
	while (TIP);
	if (NACK) print("NACK\n");

	STOP();
	while (TIP)
		;
	if (NACK) print("NACK\n");
	i2cstatusprint(i2c->sr);
}
