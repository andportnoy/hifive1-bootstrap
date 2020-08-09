#include <stdint.h>

#define GPIOADDR 0x10012000
#define UARTADDR 0x10013000

#define BIT(i) (1<<i)

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

enum {
	D0=16, D1, D2, D3, D4, D5, D6, D7,
	D8=0, D9, D10, D11, D12, D13, /* D14 is ground */
	D15=9, D16, D17, D18, D19,
};

struct uart {
	u32 txdata;
	u32 rxdata;
	u32 txctrl;
	u32 rxctrl;
	u32 ie;
	u32 ip;
	u32 div;
} volatile *const uart = (void *)UARTADDR;

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
	/* forums.sifive.com/t/gpio-pins-pull-up-issue-in-hifive-1-rev-b/3419 */
	u32 iof_en;
	u32 iof_sel;
	u32 out_xor;
} volatile *const gpio = (void *)GPIOADDR;

void uartinit(void);
void print(char *s);
void printword(u32 w);
void printchar(char c);
void ledbyte(u8 byte);
u64 cycle(void);
void sleep(u32 cycles);
void printcycle(void);

enum {
	o0=BIT(D9), o1=BIT(D8), o2=BIT(D7), o3=BIT(D6),
	o4=BIT(D5), o5=BIT(D4), o6=BIT(D3), o7=BIT(D2)
};

enum {
	i0=BIT(D10),
};

void main(void) {
	gpio->output_en |= o7|o6|o5|o4|o3|o2|o1|o0;
	gpio->input_en  |= i0;
	gpio->pue       |= i0; /* internal pull up resistor enable */

	const int nap = 0x100000;
	int len = nap;
	int pressednow = 0, pressedbefore = 0;
	u8 byte = 1;
	ledbyte(byte);
	for (u64 start, end, diff=nap;; diff=end-start) {
		start = cycle();
		pressedbefore = pressednow;
		pressednow = !(gpio->input_val&i0)? pressednow+1: 0;
		int buttonpress = (!pressednow && pressedbefore) |
			          (pressednow > 6);
		if (buttonpress) {
			print("Button pressed!\n");
			byte = (byte<<1)|(byte>>7);
			ledbyte(byte);
		} else
			printchar('\n');
		len -= diff-nap+6 /* diff=end-start takes 6 cycles? */;
		sleep(len);
		end = cycle();
	}
}

u64 cycle(void) {
	u32 l, h;
	asm volatile (
		"csrrc %0, cycle,  x0\n\t"
		"csrrc %1, cycleh, x0"
		: "=r" (l), "=r" (h));
	return (u64)h<<32|l;
}

void sleep(u32 cycles) {
	u64 cur = cycle();
	while (cycle()-cur < cycles)
		;
}

void ledbyte(u8 byte) {
	gpio->output_val =  (gpio->output_val & ~(o7|o6|o5|o4|o3|o2|o1|o0)) |
			    (byte&BIT(0)? o0: 0) |
		            (byte&BIT(1)? o1: 0) |
		            (byte&BIT(2)? o2: 0) |
		            (byte&BIT(3)? o3: 0) |
		            (byte&BIT(4)? o4: 0) |
		            (byte&BIT(5)? o5: 0) |
		            (byte&BIT(6)? o6: 0) |
		            (byte&BIT(7)? o7: 0);
}

void printword(u32 w) {
	for (int i=0; i<32; i+=4) {
		u8 n = (w>>(28-i))&0xf;
		printchar(n<0xa? '0'+n: 'a'-0xa+n);
	}
}

void printcycle(void) {
	u64 cur = cycle();
	printword(cur>>32);
	printword(cur);
}

void uartinit(void) {
	uart->txctrl |= 1;
}

void printchar(char c) {
	while (uart->txdata>>31)
		;
	uart->txdata = c;
}

void print(char *s) {
	while (*s)
		printchar(*s++);
}
