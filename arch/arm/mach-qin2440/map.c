#include <linux/kernel.h>
#include <linux/init.h>
#include <mach/hardware.h>
#include <mach/virt_addr.h>
#include <asm/mach/map.h>

static struct map_desc qin2440_iodesc[] __initdata = {
	{
		/* the BWSCON register */
		.virtual = (unsigned int)__BWSCON,
		.pfn     = __phys_to_pfn(BWSCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* USB Host Controller */
		.virtual = (unsigned int)__HcRevision,
		.pfn     = __phys_to_pfn(HcRevision),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* Interrupt Controller */
		.virtual = (unsigned int)__SRCPND,
		.pfn     = __phys_to_pfn(SRCPND),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* DMA */
		.virtual = (unsigned int)__DISRC0,
		.pfn     = __phys_to_pfn(DISRC0),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* Clock & Power Management */
		.virtual = (unsigned int)__LOCKTIME,
		.pfn     = __phys_to_pfn(LOCKTIME),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* LCD Controller */
		.virtual = (unsigned int)__LCDCON1,
		.pfn     = __phys_to_pfn(LCDCON1),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* NAND Flash */
		.virtual = (unsigned int)__NFCONF,
		.pfn     = __phys_to_pfn(NFCONF),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* Camera Interface */
		.virtual = (unsigned int)__CISRCFMT,
		.pfn     = __phys_to_pfn(CISRCFMT),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* UART */
		.virtual = (unsigned int)__ULCON0,
		.pfn     = __phys_to_pfn(ULCON0),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* PWM Timer */
		.virtual = (unsigned int)__TCFG0,
		.pfn     = __phys_to_pfn(TCFG0),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* USB Device */
		.virtual = (unsigned int)__FUNC_ADDR_REG,
		.pfn     = __phys_to_pfn(FUNC_ADDR_REG),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* Watchdog Timer */
		.virtual = (unsigned int)__WTCON,
		.pfn     = __phys_to_pfn(WTCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* IIC */
		.virtual = (unsigned int)__IICCON,
		.pfn     = __phys_to_pfn(IICCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* IIS */
		.virtual = (unsigned int)__IISCON,
		.pfn     = __phys_to_pfn(IISCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* I/O port */
		.virtual = (unsigned int)__GPACON,
		.pfn     = __phys_to_pfn(GPACON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* RTC */
		.virtual = (unsigned int)__RTCCON,
		.pfn     = __phys_to_pfn(RTCCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* A/D Converter */
		.virtual = (unsigned int)__ADCCON,
		.pfn     = __phys_to_pfn(ADCCON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* SPI */
		.virtual = (unsigned int)__SPCON0,
		.pfn     = __phys_to_pfn(SPCON0),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* SD Interface */
		.virtual = (unsigned int)__SDICON,
		.pfn     = __phys_to_pfn(SDICON),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
	{
		/* AC97 Audio-CODEC Interface */
		.virtual = (unsigned int)__AC_GLBCTRL,
		.pfn     = __phys_to_pfn(AC_GLBCTRL),
		.length  = SZ_4K,
		.type    = MT_DEVICE,
	},
};

void __init qin2440_map(void)
{
	iotable_init(qin2440_iodesc, ARRAY_SIZE(qin2440_iodesc));

	////////////////////////////////////////////////////////////////
	while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
	*(volatile unsigned char *)__UTXH0 = 'p';
	////////////////////////////////////////////////////////////////
}

