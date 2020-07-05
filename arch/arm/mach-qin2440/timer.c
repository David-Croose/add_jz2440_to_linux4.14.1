#include <mach/virt_addr.h>
#include <mach/clock.h>
#include <mach/irqs.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/irq.h>
#include <asm/mach/time.h>
#include <asm-generic/param.h>
#include <linux/clockchips.h>

extern unsigned int pclk;

// [0,255]
#define CONFIG_PRESCALER 249    

// [2,4,8,16]
#define CONFIG_DIVIDER 8      	

#define TICKRATE (pclk / (CONFIG_PRESCALER + 1) / CONFIG_DIVIDER)

/**
 * timer4 initialization
 * @param counter: how many ticks triggers an interrupt
 * @param periodic: 1 --- periodic mode
 *					0 --- oneshot mode
 */
static void timer4_init(unsigned int counter, int periodic)
{
	unsigned int _divider;
	unsigned int tmp;

	peripheral_clock_enable(CLKSRC_PWMTIMER);

	if (pclk != 50000000) {
		return;
	}

	switch (CONFIG_DIVIDER) {
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

	// interrupt disable
	tmp = *(volatile unsigned int *)__INTMSK;
	tmp |= 1 << 14;
	*(volatile unsigned int *)__INTMSK = tmp;

	tmp = *(volatile unsigned int *)__TCFG0;
    tmp &= ~(0xFF << 8);
    tmp |= (CONFIG_PRESCALER & 0xFF) << 8;
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
	if (periodic)
		tmp |= 1 << 22;
	*(volatile unsigned int *)__TCON = tmp;

    tmp &= ~(1 << 21);
    tmp |= 1 << 20;		// start timer4
	*(volatile unsigned int *)__TCON = tmp;

	// interrupt enable
	tmp = *(volatile unsigned int *)__INTMSK;
	tmp &= ~(1 << 14);
	*(volatile unsigned int *)__INTMSK = tmp;
}

static int samsung_set_next_event(unsigned long cycles,
				 struct clock_event_device *evt)
{
	if (!cycles)
		cycles = 1;

	while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
	*(volatile unsigned char *)__UTXH0 = 'x';

	timer4_init(cycles, 0);
	return 0;
}

static int samsung_set_periodic(struct clock_event_device *evt)
{
	while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
	*(volatile unsigned char *)__UTXH0 = 'y';

	timer4_init(TICKRATE / HZ, 1);
	return 0;
}

static int samsung_shutdown(struct clock_event_device *evt)
{
	unsigned int tmp;

	// interrupt disable
	tmp = *(volatile unsigned int *)__INTMSK;
	tmp |= 1 << 14;
	*(volatile unsigned int *)__INTMSK = tmp;

    tmp &= ~(1 << 20);		// stop timer4
	return 0;
}

static struct clock_event_device time_event_device = {
	.name				= "samsung_event_timer",
	.features			= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.rating				= 200,
	.set_next_event		= samsung_set_next_event,
	.set_state_shutdown	= samsung_shutdown,
	.set_state_periodic	= samsung_set_periodic,
	.set_state_oneshot	= samsung_shutdown,
	.tick_resume		= samsung_shutdown,
	/// .resume				= samsung_clockevent_resume,
};

static irqreturn_t
timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &time_event_device;
	///////////////////////////////////////////////
	static unsigned int n;

	if (n < 90000) {
		*(volatile unsigned int *)__GPFDAT |= 1 << 4;	// off
	} else {
		*(volatile unsigned int *)__GPFDAT &= ~(1 << 4); // up
	}

	if (++n > 100000)
		n = 0;
	///////////////////////////////////////////////

	/// if (pwm.variant.has_tint_cstat) {
	/// 	u32 mask = (1 << pwm.event_id);
	/// 	writel(mask | (mask << 5), pwm.base + REG_TINT_CSTAT);
	/// }

	evt->event_handler(evt);

	////////////////////////////////////////////////
	/// *(volatile unsigned int *)__SRCPND = 0xFFFFFFFF;
	/// *(volatile unsigned int *)__INTPND = 0xFFFFFFFF;
	/// *(volatile unsigned int *)__SUBSRCPND = 0xFFFFFFFF;
	////////////////////////////////////////////////

	return IRQ_HANDLED;
}

static struct irqaction timer_irq = {
	.name		= "timer",
	.flags		= IRQF_TIMER | IRQF_IRQPOLL,
	/// .flags		= IRQF_TIMER,
	.handler	= timer_interrupt,
};

void __init qin2440_init_timer(void)
{
	unsigned int virq = irq_find_mapping(irq_parent.domain, IRQ_TIMER4);
	unsigned int tmp;		// for debug

	////////////////////////////////////////////////////////////////
	// GPF4 set output
	peripheral_clock_enable(CLKSRC_GPIO);
	tmp = *(volatile unsigned int *)__GPFCON;
	tmp &= ~((0x3 & 0x3) << (4 * 2));
	tmp |= (0x1 & 0x3) << (4 * 2);
	*(volatile unsigned int *)__GPFCON = tmp;
	/// *(volatile unsigned int *)__GPFDAT &= ~(1 << 4); // up
	*(volatile unsigned int *)__GPFDAT |= 1 << 4;	// off

	while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
	*(volatile unsigned char *)__UTXH0 = 't';
	////////////////////////////////////////////////////////////////

	s3c2440_clock_init();

	/// time_event_device.cpumask = cpumask_of(0);
	clockevents_config_and_register(&time_event_device,
						TICKRATE, 1, 0xFFFF);

	setup_irq(virq, &timer_irq);

}
