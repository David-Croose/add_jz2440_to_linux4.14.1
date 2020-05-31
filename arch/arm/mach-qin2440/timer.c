
/**
 * Timer input clock Frequency = PCLK / {prescaler value+1} / {divider value}
 * {prescaler value} = 0~255
 * {divider value} = 2, 4, 8, 16
 *
 * @param nms : million seconds to count
 */
static void timer4_init(unsigned int nms)
{
	unsigned int prescaler;    // [0,255]
	unsigned int divider;      // [2,4,8,16]
	unsigned int _divider;
	unsigned int counter;
	unsigned int tmp;

	clock_enable(CLKSRC_PWMTIMER);

	if(pclk != 50000000)
	{
		return;
	}

	switch(nms)
	{
	case 0:
		return;

	case 1:
		prescaler = 249;
		divider = 8;
		counter = 25;
		break;

	case 5:
		prescaler = 249;
		divider = 8;
		counter = 125;
		break;

	case 10:
		prescaler = 249;
		divider = 8;
		counter = 250;
		break;

	case 20:
		prescaler = 249;
		divider = 8;
		counter = 500;
		break;

	case 50:
		prescaler = 249;
		divider = 8;
		counter = 1250;
		break;

	case 100:
		prescaler = 249;
		divider = 8;
		counter = 2500;
		break;

	case 500:
		prescaler = 249;
		divider = 8;
		counter = 12500;
		break;

	case 1000:
		prescaler = 99;
		divider = 16;
		counter = 31250;
		break;

	case 5000:
		prescaler = 249;
		divider = 16;
		counter = 62500;
		break;

	default:
		return;
	}

	switch(divider)
	{
	case 2:
		_divider = 0;
		break;
	case 4:
		_divider = 1;
		break;
	case 8:
		_divider = 2;
		break;
	case 16:
		_divider = 3;
		break;
	default:
		return;
	}

	tmp = *(volatile unsigned int *)__TCFG0;
    tmp &= ~(0xFF << 8);
    tmp |= (prescaler & 0xFF) << 8;
	*(volatile unsigned int *)__TCFG0 = tmp;

	tmp = *(volatile unsigned int *)__TCFG1;
    tmp &= ~(0xF << 16);
    tmp |= (_divider & 0xF) << 16;
	*(volatile unsigned int *)__TCFG1 = tmp;

	tmp = *(volatile unsigned int *)__TCNTB4;
    tmp = counter & 0xFFFF;
	*(volatile unsigned int *)__TCNTB4 = tmp;

	tmp = *(volatile unsigned int *)__TCON;
    tmp &= ~(7 << 20);
	tmp |= 1 << 21;
	tmp |= 1 << 22;
	*(volatile unsigned int *)__TCON = tmp;

    tmp &= ~(1 << 21);
    tmp |= 1 << 20;
	*(volatile unsigned int *)__TCON = tmp;

	tmp = *(volatile unsigned int *)__INTMSK;
	tmp &= ~(1 << 14);
	*(volatile unsigned int *)__INTMSK = tmp;
}

static irqreturn_t
timer_interrupt(int irq, void *dev_id)
{
	timer_tick();
	return IRQ_HANDLED;
}

static struct irqaction timer_irq = {
	.name		= "timer",
	.handler	= timer_interrupt
};

void __init qin2440_init_timer(void)
{
	setup_irq(IRQ_TIMER4, &timer_irq);
	timer4_init(1000 / HZ);
}

