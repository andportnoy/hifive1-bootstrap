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

struct prci {
	union {
		u32 hfrosccfg; /* ring oscillator */
		struct {
			u32 hfroscdiv  :6;
			u32            :10;
			u32 hfrosctrim :5;
			u32            :9;
			u32 hfroscen   :1;
			u32 hfroscrdy  :1;
		};
	};
	union {
		u32 hfxosccfg; /* crystal oscillator */
		struct {
			u32           :30;
			u32 hfxoscen  :1;
			u32 hfxoscrdy :1;
		};
	};
	union {
		u32 pllcfg;
		struct {
			u32 pllr      :3;
			u32           :1;
			u32 pllf      :6;
			u32 pllq      :2;
			u32           :4;
			u32 pllsel    :1;
			u32 pllrefsel :1;
			u32 pllbypass :1;
			u32           :12;
			u32 plllock   :1;
		};
	};
	union {
		u32 plloutdiv;
		struct {
			u32 plloutdivval :6;
			u32              :2;
			u32 plloutdivby1 :6;
		};
	};
	u8  pad[0xe0]; /* this struct is incomplete */
	u32 procmoncfg;
};

/* map pins as marked on the board to internal GPIO pin numbers */
enum {
	D0=16, D1, D2, D3, D4, D5, D6, D7,
	D8=0, D9, D10, D11, D12, D13, /* D14 is ground */
	D15=9, D16, D17, D18, D19,
};

void memcpy(void *dest, const void *src, size_t n);

void uartinit(void);
void print(char *s);
void printword(u32 w);
void printchar(char c);
u64 cycle(void);
void sleep(u32 cycles);
void printcycle(void);
void prciprint(struct prci *prciptr);
