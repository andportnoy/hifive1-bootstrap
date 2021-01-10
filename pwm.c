#define SLEEP (0x8000 / 1000) /* every 1ms */
#define SCALE 0xff

struct pwm  volatile *const pwm1 = (void *)PWM1ADDR;
struct uart volatile *const uart = (void *)UART0ADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

void intensity(u8 value) {
	pwm1->pwmcmp2 = SCALE-value;
}

void pwminit(struct pwm volatile *pwm, u16 scale) {
	pwm->pwmcmp0 = scale;     /* length of PWM cycle */
	pwm->pwmenalways = 1;     /* keep running PWM cycles */
	pwm->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm->pwmscale = 0;        /* go at full speed */
	pwm->pwmsticky = 0;       /* let the signal deassert */
}

void gpioinit(void) {
	gpio->output_en  |= BIT(BLED);
	gpio->out_xor    |= BIT(BLED);
	gpio->iof_en     |= BIT(BLED);
	gpio->iof_sel    |= BIT(BLED);
}

u8 level = 1;
u8 increment = 1;
INTERRUPT(isr) {
	mtimecmpwr(mtimecmprd() + SLEEP);
	increment = level==SCALE? 0:
                    level==0?     1: increment;
	level = increment? level+1: level-1;
	intensity(level);
}

int main(void) {
	gpioinit();
	pwminit(pwm1, SCALE);
	timerinit(isr);

	intensity(0); /* start with LED off */

	for (;;)
		__asm__ volatile ("wfi"); /* wait for interrupt */
}
