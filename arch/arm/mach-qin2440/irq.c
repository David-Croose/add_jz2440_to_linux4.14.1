#include <mach/irqs.h>
#include <mach/virt_addr.h>
#include <linux/compiler.h>
#include <linux/irq.h>
#include <asm-generic/io.h>
#include <linux/printk.h>
#include <asm/bitops.h>
#include <linux/irqdomain.h>
#include <linux/irqdesc.h>
#include <linux/linkage.h>
#include <asm/ptrace.h>
#include <asm/exception.h>
#include <linux/irqchip/chained_irq.h>

#define NOPARENTIRQ		(unsigned int)(-1)

#define IRQITEM(son, parent, dom)   \
	[(son)] = {                     \
		.hwirq = (son),             \
		.parent_hwirq = (parent),   \
		.more = (dom),              \
	}

struct dom_priv_data {
    struct irq_domain *domain;
    void __iomem *reg_pending;
    void __iomem *reg_mask;
    struct dom_priv_data *parent;
};

struct virq_priv_data {
	unsigned int hwirq;
	unsigned int parent_hwirq;
	struct dom_priv_data *more;
};

static struct dom_priv_data irq_parent = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __INTPND,
    .reg_mask = __INTMSK,
    .parent = NULL,
};

static struct dom_priv_data irq_subeint = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __EINTPEND,
    .reg_mask = __EINTMASK,
    .parent = &irq_parent,
};

static struct dom_priv_data irq_submisc = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __SUBSRCPND,
    .reg_mask = __INTSUBMSK,
    .parent = &irq_parent,
};

static struct virq_priv_data virq_tbl[] = {
	IRQITEM(IRQ_WDT_AC97_AC97, IRQ_WDT_AC97, &irq_submisc),
	IRQITEM(IRQ_WDT_AC97_WDT, IRQ_WDT_AC97, &irq_submisc),
	IRQITEM(IRQ_CAM_P, IRQ_INT_CAM, &irq_submisc),
	IRQITEM(IRQ_CAM_C, IRQ_INT_CAM, &irq_submisc),
	IRQITEM(IRQ_ADC_S, IRQ_ADC, &irq_submisc),
	IRQITEM(IRQ_ADC_TC, IRQ_ADC, &irq_submisc),
	IRQITEM(IRQ_UART2_ERR, IRQ_UART2, &irq_submisc),
	IRQITEM(IRQ_UART2_TXD, IRQ_UART2, &irq_submisc),
	IRQITEM(IRQ_UART2_RXD, IRQ_UART2, &irq_submisc),
	IRQITEM(IRQ_UART1_ERR, IRQ_UART1, &irq_submisc),
	IRQITEM(IRQ_UART1_TXD, IRQ_UART1, &irq_submisc),
	IRQITEM(IRQ_UART1_RXD, IRQ_UART1, &irq_submisc),
	IRQITEM(IRQ_UART0_ERR, IRQ_UART0, &irq_submisc),
	IRQITEM(IRQ_UART0_TXD, IRQ_UART0, &irq_submisc),
	IRQITEM(IRQ_UART0_RXD, IRQ_UART0, &irq_submisc),

	IRQITEM(IRQ_EINT23, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT22, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT21, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT20, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT19, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT18, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT17, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT16, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT15, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT14, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT13, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT12, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT11, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT10, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT9, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT8, IRQ_EINT8_23, &irq_subeint),
	IRQITEM(IRQ_EINT7, IRQ_EINT4_7, &irq_subeint),
	IRQITEM(IRQ_EINT6, IRQ_EINT4_7, &irq_subeint),
	IRQITEM(IRQ_EINT5, IRQ_EINT4_7, &irq_subeint),
	IRQITEM(IRQ_EINT4, IRQ_EINT4_7, &irq_subeint),

	IRQITEM(IRQ_ADC, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_RTC, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_SPI1, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_UART0, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_IIC, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_USBH, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_USBD, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_NFCON, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_UART1, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_SPI0, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_SDI, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_DMA3, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_DMA2, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_DMA1, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_DMA0, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_LCD, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_UART2, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER4, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER3, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER2, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER1, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER0, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_WDT_AC97, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_TICK, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_BATT_FLT, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_INT_CAM, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT8_23, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT4_7, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT3, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT2, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT1, NOPARENTIRQ, &irq_parent),
	IRQITEM(IRQ_EINT0, NOPARENTIRQ, &irq_parent),
};

static void s3c_irq_mask(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;
    unsigned int hwirq_rel;

    if (d->hwirq != IRQ_TIMER4 && d->hwirq != IRQ_UART0 && d->hwirq != IRQ_UART0_TXD)
    	printk("=====> hwirq %d mask\n", d->hwirq);

    if (d->hwirq < IRQ_EINT4)
    	hwirq_rel = d->hwirq;
    else if (d->hwirq < IRQ_UART0_RXD)
    	hwirq_rel = d->hwirq - IRQ_EINT4 + 4;
    else
    	hwirq_rel = d->hwirq - IRQ_UART0_RXD;

	mask = readl_relaxed(d->more->reg_mask);
	mask |= 1UL << hwirq_rel;
	writel_relaxed(mask, d->more->reg_mask);

    /// if (d->more->parent) {
    ///     mask = readl_relaxed(d->more->parent->reg_mask);
    ///     mask |= 1UL << d->parent_hwirq;
    ///     writel_relaxed(mask, d->more->parent->reg_mask);
    /// }
}

static void s3c_irq_unmask(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;
    unsigned int hwirq_rel;

    if (d->hwirq != IRQ_TIMER4 && d->hwirq != IRQ_UART0 && d->hwirq != IRQ_UART0_TXD)
    	printk("=====> hwirq %d unmask\n", d->hwirq);

    if (d->hwirq < IRQ_EINT4)
    	hwirq_rel = d->hwirq;
    else if (d->hwirq < IRQ_UART0_RXD)
    	hwirq_rel = d->hwirq - IRQ_EINT4 + 4;
    else
    	hwirq_rel = d->hwirq - IRQ_UART0_RXD;

	mask = readl_relaxed(d->more->reg_mask);
	mask &= ~(1UL << hwirq_rel);
	writel_relaxed(mask, d->more->reg_mask);

    /// if (d->more->parent) {
    ///     mask = readl_relaxed(d->more->parent->reg_mask);
    ///     mask &= ~(1UL << d->parent_hwirq);
    ///     writel_relaxed(mask, d->more->parent->reg_mask);
    /// }
}

static inline void s3c_irq_ack(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned int hwirq_rel;

    /// if (d->more->parent)
    ///	writel_relaxed(1UL << d->parent_hwirq, d->more->parent->reg_pending);

    if (d->hwirq < IRQ_EINT4)
    	hwirq_rel = d->hwirq;
    else if (d->hwirq < IRQ_UART0_RXD)
    	hwirq_rel = d->hwirq - IRQ_EINT4 + 4;
    else
    	hwirq_rel = d->hwirq - IRQ_UART0_RXD;

    writel_relaxed(1UL << hwirq_rel, d->more->reg_pending);
}

static int s3c_irq_type(struct irq_data *data, unsigned int type)
{
	switch (type) {
	case IRQ_TYPE_NONE:
		break;
	case IRQ_TYPE_EDGE_RISING:
	case IRQ_TYPE_EDGE_FALLING:
	case IRQ_TYPE_EDGE_BOTH:
		irq_set_handler(data->irq, handle_edge_irq);
		break;
	case IRQ_TYPE_LEVEL_LOW:
	case IRQ_TYPE_LEVEL_HIGH:
		irq_set_handler(data->irq, handle_level_irq);
		break;
	default:
		pr_err("No such irq type %d", type);
		return -EINVAL;
	}

	return 0;
}

static int get_child(unsigned int hwirq, unsigned int *child_virq)
{
	unsigned int pnd, msk;
	unsigned int rel;

	if (hwirq == IRQ_INT_CAM || hwirq == IRQ_WDT_AC97
		|| hwirq == IRQ_UART2 || hwirq == IRQ_UART1
		|| hwirq == IRQ_UART0 || hwirq == IRQ_ADC) {
		pnd = __raw_readl(__SUBSRCPND);
		if (!pnd)		/* if no sub irqs */
			return -ENODATA;

		msk = __raw_readl(__INTSUBMSK);
		pnd &= ~msk;
		*child_virq = __ffs(pnd) + VIRQ_UART0_RXD;
		return 0;
	}

	if (hwirq == IRQ_EINT4_7 || hwirq == IRQ_EINT8_23) {
		pnd = __raw_readl(__EINTPEND);
		if (!pnd)		/* if no sub irqs */
			return -ENODATA;

		msk = __raw_readl(__EINTMASK);
		pnd &= ~msk;
		rel = __ffs(pnd);
		*child_virq = rel - 4 + VIRQ_EINT4;
		return 0;
	}

	return -EINVAL;
}

static void s3c_irq_demux(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct virq_priv_data *d = irq_desc_get_chip_data(desc);
	unsigned int child_virq;

    if (d->parent_hwirq != NOPARENTIRQ)
        return;

	chained_irq_enter(chip, desc);

	while (!get_child(d->hwirq, &child_virq))
		generic_handle_irq(child_virq);

	chained_irq_exit(chip, desc);
}

asmlinkage void __exception_irq_entry s3c24xx_handle_irq(struct pt_regs *regs)
{
    int pnd;
    int offset;

	// while ((offset = readl_relaxed(__INTOFFSET)) != 0) {
	while ((pnd = readl_relaxed(__INTPND)) != 0) {
		offset = __ffs(pnd);

#if 0
		if (offset == IRQ_INT_CAM || offset == IRQ_WDT_AC97
			|| offset == IRQ_UART2 || offset == IRQ_UART1
            || offset == IRQ_UART0 || offset == IRQ_ADC)
        	handle_domain_irq(irq_submisc.domain, offset, regs);
        else if (offset == IRQ_EINT4_7 || offset == IRQ_EINT8_23)
        	handle_domain_irq(irq_subeint.domain, offset, regs);
		else
#endif
        handle_domain_irq(irq_parent.domain, offset, regs);

        writel_relaxed(1 << offset, __INTPND);
		writel_relaxed(1 << offset, __SRCPND);
	}
}

static struct irq_chip s3c_irq_level_chip = {
	.name		= "s3c-level",
	.irq_mask	= s3c_irq_mask,
	.irq_unmask	= s3c_irq_unmask,
	.irq_ack	= s3c_irq_ack,
	.irq_set_type	= s3c_irq_type,
};

static int qin2440_irq_map(struct irq_domain *h, unsigned int virq,
							irq_hw_number_t hw)
{
	return 0;
}

static const struct irq_domain_ops s3c24xx_irq_ops = {
	.map = qin2440_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

void __init qin2440_init_irq(void)
{
    struct irq_domain *parent_domain;
    struct irq_domain *subeint_domain;
    struct irq_domain *submisc_domain;
    unsigned int virq;
	int i;

	/* make sure the irq pending bits clear out */
	for (i = 0; i < 3; i++) {
		__raw_writel(__raw_readl(__SUBSRCPND), __SUBSRCPND);
		__raw_writel(__raw_readl(__SRCPND),    __SRCPND);
		__raw_writel(__raw_readl(__INTPND),    __INTPND);
		__raw_writel(__raw_readl(__EINTPEND),  __EINTPEND);
	}

    /*
     * setup irq domain
     */
    parent_domain = irq_domain_add_legacy(NULL, PARENTIRQ_TOTAL,
									16,
									0,
									&s3c24xx_irq_ops, "parent_domain");
    if (!parent_domain) {
    	printk("failed to add parent_domain\n");
    	return;
    }

    subeint_domain = irq_domain_add_legacy(NULL, SUBIRQ_EINT_TOTAL,
									16 + PARENTIRQ_TOTAL,
									0,
									&s3c24xx_irq_ops, "subeint_domain");
    if (!subeint_domain) {
    	printk("failed to add subeint_domain\n");
    	return;
    }

    submisc_domain = irq_domain_add_legacy(NULL, SUBIRQ_MISC_TOTAL,
									16 + PARENTIRQ_TOTAL + SUBIRQ_EINT_TOTAL,
									0,
									&s3c24xx_irq_ops, "submisc_domain");
    if (!submisc_domain) {
    	printk("failed to add submisc_domain\n");
    	return;
    }

    irq_parent.domain = parent_domain;
    irq_subeint.domain = subeint_domain;
    irq_submisc.domain = submisc_domain;

    /*
     * fill the irq chip, handler and chain-handler
     */
    for (virq = VIRQ_EINT0; virq <= VIRQ_ADC; virq++) {
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[VIRQ2HWIRQ(virq)]);

        /* setup the chain handler */
        if (virq == VIRQ_EINT4_7 || virq == VIRQ_EINT8_23
			|| virq == VIRQ_INT_CAM || virq == VIRQ_WDT_AC97
			|| virq == VIRQ_UART2 || virq == VIRQ_UART1
            || virq == VIRQ_UART0 || virq == VIRQ_ADC)
        	irq_set_chained_handler(virq, s3c_irq_demux);
    }
    for (virq = VIRQ_EINT4; virq <= VIRQ_EINT23; virq++) {
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[VIRQ2HWIRQ(virq)]);

        //printk("~~~~> subeint_domain: virq %d add chip\n", virq);
    }
    for (virq = VIRQ_UART0_RXD; virq <= VIRQ_WDT_AC97_AC97; virq++) {
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[VIRQ2HWIRQ(virq)]);

        //printk("~~~~> submisc_domain: virq %d add chip\n", virq);
    }

    /*
     * setup the irq entry
     */
    set_handle_irq(s3c24xx_handle_irq);
}
