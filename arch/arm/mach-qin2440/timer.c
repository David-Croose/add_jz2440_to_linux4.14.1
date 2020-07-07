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

//====================================================================
/*
 * watch-dog-timer initialization.
 * @param pre: prescaler value. range: [0, 256)
 * @param div: clock division factor. range: 16, 32, 64, 128
 * @param cnt: timer counter. range: [0, 65536)
 * @note:
 *      * the interrupt of wdt is disable after this function.
 *      * the wdt start counts and will reset the soc in @cnt/(@PCLK/(@pre+1)/@div) seconds
 *        after this function.
 *      * an example call: wdt_init(199, 128, 60000) while PCLK=50MHz, then the wdt will reset
 *        soc in 60000/(50000000/(199+1)/128)=30.72s.
 */
void wdt_init(unsigned char pre, unsigned char div, unsigned short cnt)
{
    unsigned int tmp;

    /*
     * disable watch dog tiemr first
     */
    *(volatile unsigned int *)__WTCON &= ~(1 << 5);

    /*
     * set count
     */
    *(volatile unsigned int *)__WTCNT = cnt;
    *(volatile unsigned int *)__WTDAT = 0x8000 | cnt;

    /*
     * init wdt
     */
    tmp = *(volatile unsigned int *)__WTCON;

    // [15:8]: Prescaler value
    tmp &= ~(0xFF << 8);
    tmp |= pre << 8;

    // [7:6] must be zero
    tmp &= ~(3 << 6);

    // [5] Enable or disable bit of Watchdog timer
    tmp |= 1 << 5;

    // [4:3] Determine the clock division factor
    tmp &= ~(3 << 3);
    tmp |= div << 3;

    // [2] Enable or disable bit of the interrupt
    tmp &= ~(1 << 2);

    // [1] must be zero
    tmp &= ~(1 << 1);

    // [0] Enable or disable bit of Watchdog timer output for reset signal
    tmp |= 1;

    *(volatile unsigned int *)__WTCON = tmp;
}
//====================================================================

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
	////////////////////////////////////////////////////////////////

	s3c2440_clock_init();

	/// time_event_device.cpumask = cpumask_of(0);
	clockevents_config_and_register(&time_event_device,
						TICKRATE, 1, 0xFFFF);

	setup_irq(virq, &timer_irq);

	// for debug
	wdt_init(199, 128, 60000);
}
