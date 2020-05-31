#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <mach/more_config.h>

/******************************************************************
						the led
******************************************************************/

/******************************************************************
						the button
******************************************************************/

/******************************************************************
						the uart
******************************************************************/
static struct resource uart0_resource[] = {
	/* nothing to occupy */
};

static struct resource uart1_resource[] = {
	/* nothing to occupy */
};

static struct resource uart2_resource[] = {
	/* nothing to occupy */
};

static struct platform_device uart0_device = {
	.name = "qin2440_uart",
	.id = 0,
	.resource = uart0_resource,
	.num_resources = ARRAY_SIZE(uart0_resource),
};

static struct platform_device uart1_device = {
	.name = "qin2440_uart",
	.id = 1,
	.resource = uart1_resource,
	.num_resources = ARRAY_SIZE(uart1_resource),
};

static struct platform_device uart2_device = {
	.name = "qin2440_uart",
	.id = 2,
	.resource = uart2_resource,
	.num_resources = ARRAY_SIZE(uart2_resource),
};

/******************************************************************
						the nand flash
******************************************************************/
#ifdef CONFIG_MTD_PARTITIONS
static struct mtd_partition __initdata qin2440_nand_partition[] = {
	/*
	 * 100ask mtd layout style
	 */
	[0] = {
		.name	= "bootloader",
		.offset = 0,
		.size	= 0x40000,
	},
	[1] = {
		.name	= "params",
		.offset = 0x40000,
		.size	= 0x20000,
	},
	[2] = {
		.name 	= "kernel",
		.offset	= 0x60000,
		.size	= 0x200000,
	},
	[3] = {
		.name	= "root",
		.offset	= 0x260000,
		.size	= MTDPART_SIZ_FULL,
	}
};
#endif

static struct qin2440_mtd_info mtd_info = {
	.total = ARRAY_SIZE(qin2440_nand_partition),
	.partition = qin2440_nand_partition,
};

static struct resource nand_resource[] = {
	/* nothing to occupy */
};

static struct platform_device nand_device = {
	.name = "qin2440_nand",
	.id = 0,
	.resource = nand_resource,
	.num_resources = ARRAY_SIZE(nand_resource),
	.dev = {
#ifdef CONFIG_MTD_PARTITIONS
		.platform_data = &mtd_info,
#endif
	}
};

/******************************************************************
						all of the devices
******************************************************************/
static struct platform_device *qin2440_devices[] __initdata = {
	&uart0_device,
	&uart1_device,
	&uart2_device,
	&nand_device,
};

void qin2440_register_devices(void)
{
	platform_add_devices(qin2440_devices, ARRAY_SIZE(qin2440_devices));
}
