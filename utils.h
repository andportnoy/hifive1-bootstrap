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
	/* forums.sifive.com/t/gpio-pins-pull-up-issue-in-hifive-1-rev-b/3419 */
	u32 iof_en;
	u32 iof_sel;
	u32 out_xor;
};

/* map pins as marked on the board to internal GPIO pin numbers */
enum {
	D0=16, D1, D2, D3, D4, D5, D6, D7,
	D8=0, D9, D10, D11, D12, D13, /* D14 is ground */
	D15=9, D16, D17, D18, D19,
};

void uartinit(void);
void print(char *s);
void printword(u32 w);
void printchar(char c);
u64 cycle(void);
void sleep(u32 cycles);
void printcycle(void);
