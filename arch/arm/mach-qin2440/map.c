#include <asm-arm/page.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm-arm/mach/map.h>
#include <asm-arm/memory.h>
#include <asm-arm/sizes.h>
#include <asm-arm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/virt_addr.h>

static struct map_desc qin2440_iodesc[] __initdata = {
	[0] = {
		/* the BWSCON register */
		.virtual = __BWSCON,
		.pfn     = __phys_to_pfn(BWSCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[1] = {
		/* USB Host Controller */
		.virtual = __HcRevision,
		.pfn     = __phys_to_pfn(HcRevision),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[2] = {
		/* Interrupt Controller */
		.virtual = __SRCPND,
		.pfn     = __phys_to_pfn(SRCPND),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[3] = {
		/* DMA */
		.virtual = __DISRC0,
		.pfn     = __phys_to_pfn(DISRC0),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[4] = {
		/* Clock & Power Management */
		.virtual = __LOCKTIME,
		.pfn     = __phys_to_pfn(LOCKTIME),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[5] = {
		/* LCD Controller */
		.virtual = __LCDCON1,
		.pfn     = __phys_to_pfn(LCDCON1),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[6] = {
		/* NAND Flash */
		.virtual = __NFCONF,
		.pfn     = __phys_to_pfn(NFCONF),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[7] = {
		/* Camera Interface */
		.virtual = __CISRCFMT,
		.pfn     = __phys_to_pfn(CISRCFMT),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[8] = {
		/* UART */
		.virtual = __ULCON0,
		.pfn     = __phys_to_pfn(ULCON0),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[9] = {
		/* PWM Timer */
		.virtual = __TCFG0,
		.pfn     = __phys_to_pfn(TCFG0),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[10] = {
		/* USB Device */
		.virtual = __FUNC_ADDR_REG,
		.pfn     = __phys_to_pfn(FUNC_ADDR_REG),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[11] = {
		/* Watchdog Timer */
		.virtual = __WTCON,
		.pfn     = __phys_to_pfn(WTCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[12] = {
		/* IIC */
		.virtual = __IICCON,
		.pfn     = __phys_to_pfn(IICCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[13] = {
		/* IIS */
		.virtual = __IISCON,
		.pfn     = __phys_to_pfn(IISCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[14] = {
		/* I/O port */
		.virtual = __GPACON,
		.pfn     = __phys_to_pfn(GPACON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[15] = {
		/* RTC */
		.virtual = __RTCCON,
		.pfn     = __phys_to_pfn(RTCCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[16] = {
		/* A/D Converter */
		.virtual = __ADCCON,
		.pfn     = __phys_to_pfn(ADCCON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[17] = {
		/* SPI */
		.virtual = __SPCON0,
		.pfn     = __phys_to_pfn(SPCON0),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[18] = {
		/* SD Interface */
		.virtual = __SDICON,
		.pfn     = __phys_to_pfn(SDICON),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
	[19] = {
		/* AC97 Audio-CODEC Interface */
		.virtual = __AC_GLBCTRL,
		.pfn     = __phys_to_pfn(AC_GLBCTRL),
		.length  = SZ_512K,
		.type    = MT_DEVICE,
	},
};

void __init qin2440_map(void)
{
	iotable_init(qin2440_iodesc, ARRAY_SIZE(qin2440_iodesc));
}

