#include <stdint.h>
typedef uint8_t u8;
typedef uint32_t u32;

struct uart {
	u32 txdata;
	u32 rxdata;
	u32 txctrl;
	u32 rxctrl;
	u32 ie;
	u32 ip;
	u32 div;
};

struct gpio {
	u32 input_val;
	u32 input_en;
	u32 output_en;
	u32 output_val;
	u32 pue;
	u32 ds;
	u32 rise_ie;
	u32 rise_ip;
	u32 fall_ie;
	u32 fall_ip;
	u32 high_ie;
	u32 high_ip;
	u32 low_ie;
	u32 low_ip;
	u32 out_xor;
};

void uartinit(void);
void print(char *s);
void printword(u32 w);
void printchar(char c);

void main(void) {
	struct gpio *volatile gpio = (struct gpio *)0x10012000;
	gpio->output_en = gpio->output_en | (0xd << 19);
	printword(gpio->out_xor);
	print("\r\n\r\n");
}

void printword(u32 w) {
	print("0x");
	for (int i=0; i<32; i+=4) {
		u8 nibble = (w >> (28-i)) & 0xf;
		if (0 <= nibble && nibble <= 9)
			printchar('0'+nibble);
		else
			printchar('a'-0xa+nibble);
	}
}

void uartinit(void) {
	struct uart *volatile uart = (struct uart *)0x10013000;
	uart->txctrl = 1;
}

void printchar(char c) {
	struct uart *volatile uart = (struct uart *)0x10013000;
	uart->txdata = c;
}

void print(char *s) {
	struct uart *volatile uart = (struct uart *)0x10013000;
	while (*s)
		uart->txdata = *s++;
}
