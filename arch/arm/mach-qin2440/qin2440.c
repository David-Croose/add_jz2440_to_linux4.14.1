#include <linux/init.h>
#include <asm-arm/mach/arch.h>
#include <asm-arm/mach-types.h>

extern void qin2440_register_devices(void);

static void __init qin2440_init_machine(void)
{
	qin2440_register_devices();
}

extern void __init qin2440_map(void);
extern void __init qin2440_init_timer(void)
extern void __init qin2440_init_irq(void);

MACHINE_START(S3C2440, "QIN2440")
	.boot_params  = 0x30000000 + 0x100,
	.init_time	  = qin2440_init_timer,
	.map_io       = qin2440_map,
	.init_irq     = qin2440_init_irq,
	.init_machine = qin2440_init_machine,
MACHINE_END
