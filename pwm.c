#define SLEEP 0x4000

#define PWM1ADDR 0x10025000

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

void pwmcfgprint(struct pwm *v) {
	print("pwmscale: ");
	printword(v->pwmscale);
	printchar('\n');

	print("pwmsticky: ");
	printword(v->pwmsticky);
	printchar('\n');

	print("pwmzerocmp: ");
	printword(v->pwmzerocmp);
	printchar('\n');

	print("pwmdeglitch: ");
	printword(v->pwmdeglitch);
	printchar('\n');

	print("pwmenalways: ");
	printword(v->pwmenalways);
	printchar('\n');

	print("pwmenoneshot: ");
	printword(v->pwmenoneshot);
	printchar('\n');

	print("pwmcmp0center: ");
	printword(v->pwmcmp0center);
	printchar('\n');

	print("pwmcmp1center: ");
	printword(v->pwmcmp1center);
	printchar('\n');

	print("pwmcmp2center: ");
	printword(v->pwmcmp2center);
	printchar('\n');

	print("pwmcmp3center: ");
	printword(v->pwmcmp3center);
	printchar('\n');

	print("pwmcmp0gang: ");
	printword(v->pwmcmp0gang);
	printchar('\n');

	print("pwmcmp1gang: ");
	printword(v->pwmcmp1gang);
	printchar('\n');

	print("pwmcmp2gang: ");
	printword(v->pwmcmp2gang);
	printchar('\n');

	print("pwmcmp3gang: ");
	printword(v->pwmcmp3gang);
	printchar('\n');

	print("pwmcmp0ip: ");
	printword(v->pwmcmp0ip);
	printchar('\n');

	print("pwmcmp1ip: ");
	printword(v->pwmcmp1ip);
	printchar('\n');

	print("pwmcmp2ip: ");
	printword(v->pwmcmp2ip);
	printchar('\n');

	print("pwmcmp3ip: ");
	printword(v->pwmcmp3ip);
	printchar('\n');
}

struct pwm  volatile *const pwm1 = (void *)PWM1ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

INTERRUPT(isr) {
	mtimecmpwr(mtimecmprd() + SLEEP);
	print("pwmcount : ");
	printword(pwm1->pwmcount);
	printchar('\n');
	print("pwms     : ");
	printword(pwm1->pwms);
	printchar('\n');
	print("pwmscale : ");
	printword(pwm1->pwmscale);
	printchar('\n');
	print("cmps\n");
	printword(pwm1->pwmcmp0);
	printchar('\n');
	printword(pwm1->pwmcmp1);
	printchar('\n');
	printword(pwm1->pwmcmp2);
	printchar('\n');
	printword(pwm1->pwmcmp3);
	printchar('\n');
	print("ips\n");
	printword(pwm1->pwmcmp0ip);
	printchar('\n');
	printword(pwm1->pwmcmp1ip);
	printchar('\n');
	printword(pwm1->pwmcmp2ip);
	printchar('\n');
	printword(pwm1->pwmcmp3ip);
	printchar('\n');
	print("gangs\n");
	printword(pwm1->pwmcmp0gang);
	printchar('\n');
	printword(pwm1->pwmcmp1gang);
	printchar('\n');
	printword(pwm1->pwmcmp2gang);
	printchar('\n');
	printword(pwm1->pwmcmp3gang);
	printchar('\n');
	print("gpios\n");
	printword(pwm1->pwmcmp0ip & ~(pwm1->pwmcmp0gang|pwm1->pwmcmp1ip));
	printchar('\n');
	printword(pwm1->pwmcmp1ip & ~(pwm1->pwmcmp1gang|pwm1->pwmcmp2ip));
	printchar('\n');
	printword(pwm1->pwmcmp2ip & ~(pwm1->pwmcmp2gang|pwm1->pwmcmp3ip));
	printchar('\n');
	printword(pwm1->pwmcmp3ip & ~(pwm1->pwmcmp3gang|pwm1->pwmcmp0ip));
	printchar('\n');
	printchar('\n');
}

int main(void) {
	gpio->output_en  |= BIT(BLED);
	gpio->out_xor    |= BIT(BLED);
	gpio->output_val |= BIT(BLED);
#if 1
	gpio->iof_en     |= BIT(BLED);
	gpio->iof_sel    |= BIT(BLED);
#endif

	printword(pwm1->pwmcmp2);
	printchar('\n');

#if 1
	pwm1->pwmcmp0gang = 0;
	pwm1->pwmcmp1gang = 0;
	pwm1->pwmcmp2gang = 0;
	pwm1->pwmcmp3gang = 0;
	pwm1->pwmcmp0center = 0;
	pwm1->pwmcmp1center = 0;
	pwm1->pwmcmp2center = 0;
	pwm1->pwmcmp3center = 0;

	pwm1->pwmcmp0 = 0x20;
	pwm1->pwmcmp2 = 0x1f;
	pwm1->pwmenalways = 1;
	pwm1->pwmzerocmp = 1;
	pwm1->pwmscale = 0;
	pwm1->pwmsticky = 0;
#endif

	pwmcfgprint(pwm1);
#if 1
	timerinit(isr);
#endif
	printword(gpio->iof_en & BIT(BLED));
	printchar('\n');
	printword(gpio->iof_sel & BIT(BLED));
	printchar('\n');

	for (;;)
		__asm__ volatile ("wfi"); /* wait for interrupt */
}
