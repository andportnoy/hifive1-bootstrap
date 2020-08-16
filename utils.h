#define CSR32RD_DECL(csr) u32 csr##rd(void)
#define CSR32RD_DEF(csr) \
	CSR32RD_DECL(csr) { \
		u32 v; \
		__asm__ volatile ("csrrs %0," #csr ",zero" : "=r" (v)); \
		return v; \
	}
#define CSR32WR_DECL(csr) void csr##wr(u32 v)
#define CSR32WR_DEF(csr) \
	CSR32WR_DECL(csr) { \
		__asm__ volatile ("csrrw zero," #csr ", %0" : : "r" (v)); \
	}

#define INTERRUPT(x) __attribute__ ((interrupt, aligned(64))) void x(void)

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

struct pwm {
	union {
	u32 pwmcfg;
	struct {
		u32 pwmscale      :4;
		u32               :4;
		u32 pwmsticky     :1;
		u32 pwmzerocmp    :1;
		u32 pwmdeglitch   :1;
		u32               :1;
		u32 pwmenalways   :1;
		u32 pwmenoneshot  :1;
		u32               :2;
		u32 pwmcmp0center :1;
		u32 pwmcmp1center :1;
		u32 pwmcmp2center :1;
		u32 pwmcmp3center :1;
		u32               :4;
		u32 pwmcmp0gang   :1;
		u32 pwmcmp1gang   :1;
		u32 pwmcmp2gang   :1;
		u32 pwmcmp3gang   :1;
		u32 pwmcmp0ip     :1;
		u32 pwmcmp1ip     :1;
		u32 pwmcmp2ip     :1;
		u32 pwmcmp3ip     :1;
	};
	};
	u32 :32;
	u32 pwmcount;
	u32 :32;
	u32 pwms;
	u32 :32;
	u32 :32;
	u32 :32;
	u32 pwmcmp0;
	u32 pwmcmp1;
	u32 pwmcmp2;
	u32 pwmcmp3;
};

/* map pins as marked on the board to internal GPIO pin numbers */
enum {
	D0=16, D1, D2, D3, GLED=D3, D4, D5, BLED=D5, D6, RLED=D6, D7,
	D8=0, D9, D10, D11, D12, D13, /* D14 is ground */
	D15=9, D16, D17, D18, D19,
};

void *memcpy(void *dest, const void *src, size_t n);

void uartinit(void);
void timerinit(void (*isr)(void));
void print(char *s);
void printword(u32 w);
void printdword(u64 dw);
void printchar(char c);
u64 cycle(void);
void sleep(u32 cycles);
void printcycle(void);
void prciprint(struct prci *prciptr);
void pwmcfgprint(struct pwm *v);
void mcauseprint(u32 v);
u64 mtimerd(void);
void mtimewr(u64 v);
u64 mtimecmprd(void);
void mtimecmpwr(u64 v);
CSR32RD_DECL(mtvec);
CSR32WR_DECL(mtvec);
CSR32RD_DECL(mie);
CSR32WR_DECL(mie);
CSR32RD_DECL(mstatus);
CSR32WR_DECL(mstatus);
CSR32RD_DECL(mcause);
CSR32WR_DECL(mcause);
CSR32RD_DECL(mtval);
CSR32WR_DECL(mtval);
