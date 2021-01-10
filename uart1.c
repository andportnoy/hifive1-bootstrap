struct uart volatile *const uart0 = (void *)UART0ADDR;
struct uart volatile *const uart1 = (void *)UART1ADDR;
struct gpio volatile *const gpio  = (void *)GPIOADDR;

#define MS 16000

#define TXFULL  (1<<31)
#define RXEMPTY (1<<31)

#define REG_OPR_MODE 0x3d

#define WRITE_SUCCESS             0x01
#define READ_FAIL                 0x02
#define WRITE_FAIL                0x03
#define REGMAP_INVALID_ADDRESS    0x04
#define REGMAP_WRITE_DISABLED     0x05
#define WRONG_START_BYTE          0x06
#define BUS_OVER_RUN_ERROR        0x07
#define MAX_LENGTH_ERROR          0x08
#define MIN_LENGTH_ERROR          0x09
#define RECEIVE_CHARACTER_TIMEOUT 0x0a
#define BAD_STATE                 0x0b
#define TRUNCATED_READ            0x0c

#define MODE_CONFIG 0x0
#define MODE_NDOF   0xc

#define UART_TRIES 2

#define _(x) [x] = #x

#define CHECK(status) \
	do { \
		int s = (status); \
		if (s) { \
			print(errstr[s]); \
			printchar('\n'); \
		} \
	} while (0)

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

int bnoread_retry(u8 regaddr, u8 *data, u8 len, int tries) {
	u8 command[] = {0xaa, 0x01, regaddr, len};
	for (int i=0, n=sizeof command; i<n; ++i)
		uart1write(command[i]);

	u8 header = uart1read();
	u8 status;

	switch (header) {
	case 0xbb: /* success */
	{
		u8 rlen = uart1read();
		if (rlen < len)
			return TRUNCATED_READ;
		for (int i=0, n=rlen; i<n; ++i)
			data[i] = uart1read();
		status = 0;
		break;
	}
	case 0xee: /* error */
		status = uart1read();
		break;
	default:
		status = BAD_STATE;
		break;
	}

	if (tries>1 && status==BUS_OVER_RUN_ERROR)
		return bnoread_retry(regaddr, data, len, tries-1);

	return status;
}

int bnoread(u8 regaddr, u8 *data, u8 len) {
	return bnoread_retry(regaddr, data, len, UART_TRIES);
}

int bnowrite_retry(u8 regaddr, const u8 *data, u8 len, int tries) {
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
	else if (tries>1 && status==BUS_OVER_RUN_ERROR)
		return bnowrite_retry(regaddr, data, len, tries-1);
	else
		return status;
}

int bnowrite(u8 regaddr, const u8 *data, u8 len) {
	return bnowrite_retry(regaddr, data, len, UART_TRIES);
}

void printbytes(u8 *data, int len) {
	for (int i=0, n=len; i<n; ++i) {
		if (i)
			printchar(' ');
		printbyte(data[i]);
	}
	printchar('\n');
}

u8 getmode(void) {
	u8 mode;
	CHECK(bnoread(REG_OPR_MODE, &mode, sizeof mode));
	return mode & 0xf;
}

void setmode(u8 mode) {
	CHECK(bnowrite(REG_OPR_MODE, &mode, sizeof mode));
	sleep(19*MS); /* 19 ms to switch between modes */
}

int main(void) {
	/* GPIO 18 = DIG 2 = host TX = device RX (SCL) = ORANGE wire
	 * GPIO 23 = DIG 7 = host RX = device TX (SDA) = GREEN  wire
	 */
	gpio->iof_en  |=   BIT(18)|BIT(23);  /* enable HW function */
	gpio->iof_sel &= ~(BIT(18)|BIT(23)); /* select UART1 */

	uart1init();

	/* sleep for a second, BNO 055 takes at least 650 ms to power on */
	sleep(16*1000*1000);

	switch (getmode()) {
	case MODE_CONFIG: print("CONFIG mode\n"); break;
	case MODE_NDOF:   print("NDOF mode\n");   break;
	}

	setmode(MODE_NDOF);

	switch (getmode()) {
	case MODE_CONFIG: print("CONFIG mode\n"); break;
	case MODE_NDOF:   print("NDOF mode\n");   break;
	}

	return 0;
}
