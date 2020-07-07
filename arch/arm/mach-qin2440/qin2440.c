#include <linux/init.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <mach/clock.h>
#include <mach/virt_addr.h>

extern void qin2440_register_devices(void);

static void __init qin2440_init_machine(void)
{
	////////////////////////////////////////////////////////////////
	// GPF5 set output
	unsigned int tmp;
	peripheral_clock_enable(CLKSRC_GPIO);
	tmp = *(volatile unsigned int *)__GPFCON;
	tmp &= ~((0x3 & 0x3) << (5 * 2));
	tmp |= (0x1 & 0x3) << (5 * 2);
	*(volatile unsigned int *)__GPFCON = tmp;
	*(volatile unsigned int *)__GPFDAT |= 1 << 5;	// off
	////////////////////////////////////////////////////////////////

	qin2440_register_devices();
}

extern void qin2440_map(void);
extern void qin2440_init_timer(void);
extern void qin2440_init_irq(void);

MACHINE_START(S3C2440, "QIN2440")
	.atag_offset  = 0x100,
	.init_time	  = qin2440_init_timer,
	.map_io       = qin2440_map,
	.init_irq     = qin2440_init_irq,
	.init_machine = qin2440_init_machine,
MACHINE_END
