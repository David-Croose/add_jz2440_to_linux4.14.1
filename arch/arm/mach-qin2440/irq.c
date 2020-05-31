#include <mach/irq.h>



struct irq_priv_data {
    struct irq_domain *domain;
    void __iomem *reg_srcpending;
    void __iomem *reg_pending;
    void __iomem *reg_mask;
    unsigned int irq_start;
    struct irq_priv_data *parent;
    struct irq_priv_data *child;
};

static struct irq_priv_data irq_parent_nosub = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_srcpending = __SRCPND,
    .reg_pending = __INTPND,
    .reg_mask = __INTMSK,
    .irq_start = IRQ_EINT0,
    .parent = NULL,
    .child = NULL,
};

static struct irq_priv_data irq_parent_eintsub = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_srcpending = __SRCPND,
    .reg_pending = __INTPND,
    .reg_mask = __INTMSK,
    .irq_start = IRQ_EINT0,
    .parent = NULL,
    .child = &irq_subeint;
};

static struct irq_priv_data irq_parent_miscsub = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_srcpending = __SRCPND,
    .reg_pending = __INTPND,
    .reg_mask = __INTMSK,
    .irq_start = IRQ_EINT0,
    .parent = NULL,
    .child = &irq_submisc;
};

static struct irq_priv_data irq_subeint = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_srcpending = NULL,
    .reg_pending = __EINTPEND,
    .reg_mask = __EINTMASK,
    .irq_start = IRQ_EINT4,
    .parent = &irq_parent_eintsub,
    .child = NULL,
};

static struct irq_priv_data irq_submisc = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_srcpending = NULL,
    .reg_pending = __SUBSRCPND,
    .reg_mask = __INTSUBMSK,
    .irq_start = IRQ_UART0_RXD,
    .parent = &irq_parent_miscsub,
    .child = NULL,
};

static void s3c_irq_mask(struct irq_data *data)
{
    struct irq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = readl_relaxed(d->reg_mask);
	mask |= 1UL << d->hwirq;
	writel_relaxed(mask, d->reg_mask);

    if (d->parent) {
        mask = readl_relaxed(d->parent->reg_mask);
        mask |= 1UL << d->parent->hwirq;
        writel_relaxed(mask, d->parent->reg_mask);
    }
}

static void s3c_irq_unmask(struct irq_data *data)
{
    struct irq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = readl_relaxed(d->reg_mask);
	mask &= ~(1UL << d->hwirq);
	writel_relaxed(mask, d->reg_mask);

    if (d->parent) {
        mask = readl_relaxed(d->parent->reg_mask);
        mask &= ~(1UL << d->parent->hwirq);
        writel_relaxed(mask, d->parent->reg_mask);
    }
}

static inline void s3c_irq_ack(struct irq_data *data)
{
    struct irq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long bitval = 1UL << d->hwirq;

    if (d->reg_srcpending)
        writel_relaxed(bitval, d->reg_srcpending);

    writel_relaxed(bitval, d->reg_pending);
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

static void s3c_irq_demux(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct irq_priv_data *d = irq_desc_get_chip_data(desc);
	unsigned int n, virq;
	unsigned long src, msk;

    if (!d->child)
        return;

	chained_irq_enter(chip, desc);

	src = readl_relaxed(d->child->reg_pending);
	msk = readl_relaxed(d->child->reg_mask);

	src &= ~msk;
	/// src &= 1 << d->hwirq;

	while (src) {
		n = __ffs(src);
		src &= ~(1 << n);
		virq = irq_find_mapping(d->child->domain, d->child->irq_start + n);
		generic_handle_irq(irq);
	}

	chained_irq_exit(chip, desc);
}

asmlinkage void __exception_irq_entry s3c24xx_handle_irq(struct pt_regs *regs)
{
    int pnd;
    int offset;

	do {
        pnd = readl_relaxed(__INTPND);
        if (!pnd)
            continue;

        writel_relaxed(__SRCPND);

        /* We have a problem that the INTOFFSET register does not always
        * show one interrupt. Occasionally we get two interrupts through
        * the prioritiser, and this causes the INTOFFSET register to show
        * what looks like the logical-or of the two interrupt numbers.
        *
        * Thanks to Klaus, Shannon, et al for helping to debug this problem
        */
        offset = readl_relaxed(__INTOFFSET);

        /* Find the bit manually, when the offset is wrong.
        * The pending register only ever contains the one bit of the next
        * interrupt to handle.
        */
        if (!(pnd & (1 << offset)))
            offset =  __ffs(pnd);

        handle_domain_irq(&irq_parent_nosub.domain, IRQ_EINT0 + offset, regs);
        break;
	} while (1);
}

static struct irq_chip s3c_irq_level_chip = {
	.name		= "s3c-level",
	.irq_mask	= s3c_irq_mask,
	.irq_unmask	= s3c_irq_unmask,
	.irq_ack	= s3c_irq_ack,
	.irq_set_type	= s3c_irq_type,
};

static const struct irq_domain_ops s3c24xx_irq_ops = {
	.xlate = irq_domain_xlate_twocell,
};

void qin2440_init_irq(void)
{
    struct irq_domain *parent_domain;
    struct irq_domain *subeint_domain;
    struct irq_domain *submisc_domain;
    struct irq_priv_data *d;
    unsigned int virq;
    unsigned int  hwirq;

    /*
     * setup irq domain
     */
    parent_domain = irq_domain_add_legacy(NULL, PARENTIRQ_TOTAL,
                                            16,
                                            IRQ_EINT0,
                                            &s3c24xx_irq_ops);

    subeint_domain = irq_domain_add_legacy(NULL, SUBIRQ_EINT_TOTAL,
                                            16 + PARENTIRQ_TOTAL,
                                            IRQ_EINT4,
                                            &s3c24xx_irq_ops);

    submisc_domain = irq_domain_add_legacy(NULL, SUBIRQ_MISC_TOTAL,
                                            16 + PARENTIRQ_TOTAL + SUBIRQ_EINT_TOTAL,
                                            IRQ_UART0_RXD,
                                            &s3c24xx_irq_ops);

    /*
     * fill the irq chip, handler and chain-handler
     */
    irq_parent_nosub.domain = parent_domain;
    irq_parent_eintsub.domain = parent_domain;
    irq_parent_miscsub.domain = parent_domain;
    irq_subeint.domain = subeint_domain;
    irq_submisc.domain = submisc_domain;
    for (hwirq = IRQ_EINT0; hwirq <= IRQ_ADC; hwirq++) {
        virq = irq_find_mapping(parent_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &irq_parent);

        /* setup the chain handler */
        if (hwirq == IRQ_EINT4_7 || hwirq == IRQ_EINT8_23 || hwirq == IRQ_INT_CAM
            || hwirq == IRQ_WDT_AC97 || hwirq == IRQ_UART2 || hwirq == IRQ_UART1
            || hwirq == IRQ_UART0 || hwirq == IRQ_ADC) {
                irq_set_chained_handler(virq, s3c_irq_demux);
        }
    }
    for (hwirq = IRQ_EINT4; hwirq <= IRQ_EINT23; hwirq++) {
        virq = irq_find_mapping(subeint_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &irq_subeint);
    }
    for (hwirq = IRQ_UART0_RXD; hwirq <= IRQ_WDT_AC97_AC97; hwirq++) {
        virq = irq_find_mapping(submisc_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &irq_submisc);
    }

    /*
     * setup the irq entry
     */
    set_handle_irq(s3c24xx_handle_irq);
}

