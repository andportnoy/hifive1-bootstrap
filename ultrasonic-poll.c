#define TRIGGER BIT(D18)
#define ECHO    BIT(D19)
#define LED     BIT(BLED)

#define SCALE 0x100

struct pwm  volatile *const pwm1 = (void *)PWM1ADDR;
struct pwm  volatile *const pwm2 = (void *)PWM2ADDR;
struct uart volatile *const uart = (void *)UARTADDR;
struct gpio volatile *const gpio = (void *)GPIOADDR;

int main(void) {
	gpio->iof_en  |= TRIGGER; /* use it for PWM  */
	gpio->iof_sel |= TRIGGER; /* PWM */

	gpio->input_en = ECHO;
	gpio->pue     |= ECHO;
	gpio->iof_en  &= ~ECHO;

	gpio->output_en |= LED;
	gpio->iof_en    |= LED;
	gpio->out_xor   |= LED;
	gpio->iof_sel   |= LED;

	pwm1->pwmcmp0 = SCALE;     /* length of PWM cycle */
	pwm1->pwmenalways = 1;     /* keep running PWM cycles */
	pwm1->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm1->pwmscale = 0;        /* go at full speed */
	pwm1->pwmsticky = 0;       /* let the signal deassert */

	pwm2->pwmcfg = 0;
	pwm2->pwmcmp0 = 6;         /* 1 out of 6 down, 0 out of 5 didn't work */
	pwm2->pwmcmp2 = 1;         /* on for 5*32 = 160 cycles */
	pwm2->pwmzerocmp = 1;      /* limit PWM cycle length to pwmcmp0 */
	pwm2->pwmscale = 5;        /* 32 cycles is a unit */
	pwm2->pwmsticky = 0;       /* let the signal deassert */

	for (;;) {
		pwm2->pwmenoneshot = 1;    /* fire a PWM pulse */

		while (!(gpio->input_val&ECHO));
		u64 start = cycle();
		while (gpio->input_val&ECHO);
		u64 end = cycle();

		u64 delta = end-start;

		pwm1->pwmcmp2 = delta>>6;
		sleep(16e5); /* 100 ms */
	}
}
