#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/kernel.h>
#include <asm-arm/io.h>
#include <asm/arch/virt_addr.h>
#include <asm/arch/clock.h>
#include <asm/arch/more_config.h>

static void qin2440_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	if(cmd == NAND_CMD_NONE) {
		return;
	}

	if(ctrl & NAND_CLE) {
		__raw_writeb(cmd, __NFCMD);
	} else {
		__raw_writeb(cmd, __NFADDR);
	}
}

static void qin2440_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;

	for(i = 0; i < len; i++) {
		buf[i] = __raw_readb(__NFDATA);
	}
}

static void qin2440_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int i;

	for(i = 0; i < len; i++) {
		__raw_writeb(buf[i], __NFDATA);
	}
}

static void qin2440_nand_select_chip(struct mtd_info *mtd, int chip)
{
	unsigned int tmp;

	if(chip == -1) { /* if deselect */
		tmp = __raw_readl(__NFCONT);
		tmp |= 1 << 1;
		__raw_writel(tmp, __NFCONT);
	} else { /* if select */
		tmp = __raw_readl(__NFCONT);
		tmp &= ~(1 << 1);
		__raw_writel(tmp, __NFCONT);
	}
}

void qin2440_nand_cmdfunc(struct mtd_info *mtd, unsigned command, int column,
								 int page_addr)
{
	volatile unsigned int i;

	__raw_writeb(command, __NFCMD);
	for(i = 0; i < 10; i ++);

	if(column != -1) {
		__raw_writeb(column & 0xff, __NFADDR);               /* Column Address A0~A7 */
		for(i = 0; i < 10; i++);
		__raw_writeb((column >> 8) & 0x0f, __NFADDR);        /* Column Address A8~A11 */
		for(i = 0; i < 10; i++);
	}

	if(page_addr != -1) {
		__raw_writeb(page_addr & 0xff, __NFADDR);            /* Row Address A12~A19 */
		for(i = 0; i < 10; i++);
		__raw_writeb((page_addr >> 8) & 0xff, __NFADDR);     /* Row Address A20~A27 */
		for(i = 0; i < 10; i++);
		__raw_writeb((page_addr >> 16) & 0x03, __NFADDR);    /* Row Address A28~A29 */
		for(i = 0; i < 10; i++);
	}
}

static int qin2440_nand_devready(struct mtd_info *mtd)
{
	return __raw_readb(__NFSTAT) & 1;
}

static int __init qin2440_nand_probe(struct platform_device *pdev)
{
	struct mtd_info *mtd;
	struct nand_chip *nc;
	int ret = 0;
	unsigned int tmp;
#ifdef CONFIG_MTD_PARTITIONS
	struct qin2440_mtd_info *qin2440_mtd_info = pdev->dev.platform_data;
#endif

	nc = kzalloc(sizeof(struct nand_chip) + sizeof(struct mtd_info), GFP_KERNEL);
	if(!nc) {
		printk(KERN_ERR "QIN2440 Nand: failed to allocate device structure.\n");
		ret = -ENOMEM;
		goto no_res;
	}
	mtd = (struct mtd_info *)(nc + 1);

	/* nand hardware init */
	{
		/*
		 * enable the nand clock
		 */
		periphral_clock_enable(CLKSRC_NAND);

		/*
		 * init nand timming
		 * the mpll clock rate must be 400MHz !
		 */
		__raw_writel((2 << 12) | (6 << 8) | (2 << 4), __NFCONF);

		/*
		 * enable nand
		 */
		tmp = __raw_readl(__NFCONT);
		tmp |= 1;
		__raw_writel(tmp, __NFCONT);
	}

	mtd->priv = nc;
	mtd->owner = THIS_MODULE;

	nc->IO_ADDR_R	= (void __iomem *)__NFDATA;
	nc->IO_ADDR_W	= (void __iomem *)__NFDATA;
	nc->cmd_ctrl	= qin2440_nand_cmd_ctrl;
	nc->read_buf	= qin2440_nand_read_buf;
	nc->write_buf	= qin2440_nand_write_buf;
	nc->dev_ready	= qin2440_nand_devready;
	nc->select_chip = qin2440_nand_select_chip;
	/*
	 * don't put this line: "nc->cmdfunc = qin2440_nand_cmdfunc;" unless you
	 * really know what it should do.
	 * leave this empty, linux will give itself a better, more complex cmdfunc,
	 * called "nand_command_lp" in nand_base.c
	 */
	nc->ecc.mode	= NAND_ECC_SOFT;
	nc->chip_delay	= 50;

	platform_set_drvdata(pdev, mtd);

	if(nand_scan(mtd, 1)) {
		ret = -ENXIO;
		goto no_dev;
	}

#ifdef CONFIG_MTD_PARTITIONS
	if(qin2440_mtd_info->total <= 0 || !qin2440_mtd_info->partition) {
		goto no_dev;
	}

	ret = add_mtd_partitions(mtd, qin2440_mtd_info->partition, qin2440_mtd_info->total);
#else
	ret = add_mtd_device(mtd);
#endif

	if(ret) {
		nand_release(mtd);
		goto no_dev;
	}

	return 0;

no_dev:
	platform_set_drvdata(pdev, NULL);
no_res:
	kfree(nc);
	printk("QIN2440 Nand: register failed\n");

	return ret;
}

static int __devexit qin2440_nand_remove(struct platform_device *pdev)
{
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct nand_chip *nc = mtd->priv;
	unsigned int tmp;

	/*
	 * disable the nand clock
	 */
	periphral_clock_disable(CLKSRC_NAND);

	/*
	 * disable nand
	 */
	tmp = __raw_readl(__NFCONT);
	tmp &= ~1;
	__raw_writel(tmp, __NFCONT);

	platform_set_drvdata(pdev, NULL);
	nand_release(mtd);
	kfree(nc);

	return 0;
}

static struct platform_driver qin2440_nand_driver = {
	.remove		= __devexit_p(qin2440_nand_remove),
	.driver		= {
		.name	= "qin2440_nand",
		.owner	= THIS_MODULE,
	},
};

static int __init qin2440_nand_init(void)
{
	return platform_driver_probe(&qin2440_nand_driver, qin2440_nand_probe);
}

static void __exit qin2440_nand_exit(void)
{
	platform_driver_unregister(&qin2440_nand_driver);
}

module_init(qin2440_nand_init);
module_exit(qin2440_nand_exit);
MODULE_LICENSE("GPL");
