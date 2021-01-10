struct uart volatile *const uart0 = (void *)UART0ADDR;
struct uart volatile *const uart1 = (void *)UART1ADDR;
struct gpio volatile *const gpio  = (void *)GPIOADDR;

#define TXFULL  (1<<31)
#define RXEMPTY (1<<31)


#define WRITE_SUCCESS             0x01
#define READ_FAIL                 0x02
#define WRITE_FAIL                0x03
#define REGMAP_INVALID_ADDRESS    0x04
#define REGMAP_WRITE_DISABLED     0x05
#define WRONG_START_BYTE          0x06
#define BUS_OVER_RUN_ERROR        0x07
#define MAX_LENGTH_ERROR          0x08
#define MIN_LENGTH_ERROR          0x09
#define RECEIVE_CHARACTER_TIMEOUT 0x0A
#define BAD_STATE                 0x0B
#define TRUNCATED_READ            0x0C

#define _(x) [x] = #x

char *errstr[] = {
	_(WRITE_SUCCESS),
	_(READ_FAIL),
	_(WRITE_FAIL),
	_(REGMAP_INVALID_ADDRESS),
	_(REGMAP_WRITE_DISABLED),
	_(WRONG_START_BYTE),
	_(BUS_OVER_RUN_ERROR),
	_(MAX_LENGTH_ERROR),
	_(MIN_LENGTH_ERROR),
	_(RECEIVE_CHARACTER_TIMEOUT),
	_(BAD_STATE),
	_(TRUNCATED_READ),
};

void uart1init(void) {
	uart1->div     = 0x8b; /* 115200 baud rate */
	uart1->txctrl |= 1;
	uart1->rxctrl |= 1;
}

void uart1write(u8 byte) {
	while (uart1->txdata & TXFULL)
		;
	uart1->txdata = byte;
}

u8 uart1read(void) {
	u32 rx = uart1->rxdata;
	while (rx & RXEMPTY)
		rx = uart1->rxdata;
	return rx;
}

int bnoread(u8 regaddr, u8 *data, u8 len) {
	u8 command[] = {0xaa, 0x01, regaddr, len};
	for (int i=0, n=sizeof command; i<n; ++i)
		uart1write(command[i]);

	u8 header = uart1read();

	switch (header) {
	case 0xbb: /* success */
	{
		u8 rlen = uart1read();
		if (rlen < len)
			return TRUNCATED_READ;
		for (int i=0, n=rlen; i<n; ++i)
			data[i] = uart1read();
		return 0;
	}
	case 0xee: /* error */
		return uart1read();
	default:
		return BAD_STATE;
	}
}

int bnowrite(u8 regaddr, const u8 *data, u8 len) {
	u8 command[] = {0xaa, 0x00, regaddr, len};
	for (int i=0, n=sizeof command; i<n; ++i)
		uart1write(command[i]);
	for (int i=0, n=len; i<n; ++i)
		uart1write(data[i]);

	u8 header = uart1read();

	if (header != 0xee)
		return BAD_STATE;

	u8 status = uart1read();

	if (status == WRITE_SUCCESS)
		return 0;
	else
		return status;
}

void printbytes(u8 *data, int len) {
	for (int i=0, n=len; i<n; ++i) {
		if (i)
			printchar(' ');
		printbyte(data[i]);
	}
	printchar('\n');
}


#define CHECK(status) \
	do { \
		int s = (status); \
		if (s) { \
			print(errstr[s]); \
			printchar('\n'); \
			return 1; \
		} \
	} while (0)

int main(void) {
	/* GPIO 18 = DIG 2 = host TX = device RX (SCL) = ORANGE wire
	 * GPIO 23 = DIG 7 = host RX = device TX (SDA) = GREEN  wire
	 */
	gpio->iof_en  |=   BIT(18)|BIT(23);  /* enable HW function */
	gpio->iof_sel &= ~(BIT(18)|BIT(23)); /* select UART1 */

	uart1init();

	/* record original data */
	u8 data0[4];
	CHECK(bnoread(0x55, data0, sizeof data0));
	printbytes(data0, sizeof data0);

	/* write new data */
	u8 data1[4] = {0xde, 0xad, 0xbe, 0xef};
	CHECK(bnowrite(0x55, data1, sizeof data1));

	/* verify that new data has been written */
	u8 data2[4];
	CHECK(bnoread(0x55, data2, sizeof data2));
	printbytes(data2, sizeof data2);

	/* write original data */
	CHECK(bnowrite(0x55, data0, sizeof data0));

	/* verify that original data has been restored */
	u8 data3[4];
	CHECK(bnoread(0x55, data3, sizeof data3));
	printbytes(data3, sizeof data3);

	return 0;
}
