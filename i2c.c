struct uart volatile *const uart0 = (void *)UART0ADDR;
struct i2c  volatile *const i2c   = (void *)I2CADDR;
struct gpio volatile *const gpio  = (void *)GPIOADDR;
struct prci volatile *const prci  = (void *)PRCIADDR;

enum { STA=BIT(7), STO=BIT(6), RD=BIT(5), WR=BIT(4), };

void i2cstatusprint(u8 sr) {
	print(sr&BIT(7)? "NACK ": "ACK ");
	print(sr&BIT(6)? "BUSY ": "FREE ");
	print(sr&BIT(5)? "ARB_LOST ": "");
	print(sr&BIT(1)? "TIP_PEND ": "");
	print(sr&BIT(0)? "INT_PEND": "");
	printchar('\n');
}

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
#define SLAVEADDR 0x28 /* BNO 055 */

void write(u8 devaddr, u8 regaddr, u8 *buf, int count) {
	PUT_ADDR_WR(devaddr);
	START();
	while (TIP);
	if (NACK) print("NACK\n");

	PUT(regaddr);
	WRITE();
	while (TIP);
	if (NACK) print("NACK\n");

}

void read(u8 devaddr, u8 regaddr, u8 *buf, int count) {
	PUT_ADDR_WR(devaddr);
	START();
	while (TIP);
	while (NACK);

	print("slave address sent, write\n");

	PUT(regaddr);
	WRITE();
	while (TIP);
	while (NACK);

	print("register address sent\n");

	PUT_ADDR_RD(devaddr);
	START();
	while (TIP);
	if (NACK) print("NACK\n");

	print("slave address sent, read\n");

	for (int i=0, n=count; i<n; ++i) {
		i2c->cr = RD | (i==n-1? STO|BIT(3): 0);
		while (TIP);
		buf[i] = GET();

		print("register read\n");
	}
}

int main(void) {
	gpio->iof_en  |=   BIT(12)|BIT(13);  /* enable HW function */
	gpio->iof_sel &= ~(BIT(12)|BIT(13)); /* select I2C */
	gpio->pue     |=   BIT(12)|BIT(13);  /* pull up enable */

#if 0
	1. Write slave address to TXR (shifted) ORed with 0, set STA and WR.
	2. Wait for TIP and check ACK.
	3. Write register address to TXR, set WR.
	4. Wait for TIP and check ACK.
	5. Write slave address to TXR (shifted) ORed with 1, set STA and RD.
	6. Wait for TIP and check ACK.
	7. Set the RD bit.
	8. Wait for TIP and IF.
	9. Read RXR.
	10. Repeat 7-9 if necessary.
	11. Send NACK.
	12. Generate STOP by setting STO.
#endif

	i2cprint(i2c);
	i2c->prerlo = 0xff;
	i2c->prerhi = 0x00;
	sleep(16e5);
	i2c->ctr = BIT(7);
	sleep(16e5);
	i2cprint(i2c);

	sleep(16e6);
	i2cstatusprint(i2c->sr);

	u8 data[8];
	read(SLAVEADDR, 0, data, sizeof data);
	for (int i=0, n=sizeof data; i<n; ++i) {
		printbyte(data[i]);
		printchar(' ');
	}
		printchar('\n');
	i2cstatusprint(i2c->sr);
}
