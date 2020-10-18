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

struct priv_irq_chip_data {
    void __iomem *reg_pending;
    void __iomem *reg_mask;
};

static struct priv_irq_chip_data parent_irq_chipdata = {
	.reg_pending = __INTPND,
	.reg_mask = __INTMSK,
};

static struct priv_irq_chip_data subeint_irq_chipdata = {
	.reg_pending = __EINTPEND,
	.reg_mask = __EINTMASK,
};

static struct priv_irq_chip_data submisc_irq_chipdata = {
	.reg_pending = __SUBSRCPND,
	.reg_mask = __INTSUBMSK,
};

static struct irq_domain *parent_domain;

static void s3c_irq_mask(struct irq_data *data)
{
    struct priv_irq_chip_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = __raw_readl(d->reg_mask);
	mask |= 1UL << data->hwirq;
	__raw_writel(mask, d->reg_mask);
}

static void s3c_irq_unmask(struct irq_data *data)
{
    struct priv_irq_chip_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = __raw_readl(d->reg_mask);
	mask &= ~(1UL << data->hwirq);
	__raw_writel(mask, d->reg_mask);
}

static inline void s3c_irq_ack(struct irq_data *data)
{
    struct priv_irq_chip_data *d = irq_data_get_irq_chip_data(data);
    unsigned int pnd;

    pnd = 1UL << data->hwirq;
    __raw_writel(pnd, d->reg_pending);
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
	unsigned int virq = irq_desc_get_irq(desc);		/* TODO  does the virq belong to parent irq? */
	unsigned int child_virq;

	chained_irq_enter(chip, desc);

	if (!get_child(VIRQ2HWIRQ(virq), &child_virq))
		generic_handle_irq(child_virq);

	chained_irq_exit(chip, desc);
}

asmlinkage void __exception_irq_entry s3c24xx_handle_irq(struct pt_regs *regs)
{
    int pnd;
    int offset;

    __raw_readl(__SRCPND);
    pnd = __raw_readl(__INTPND);

	if (pnd) {
		offset = __ffs(pnd);

        handle_domain_irq(parent_domain, offset, regs);

        __raw_writel(1 << offset, __INTPND);
		__raw_writel(1 << offset, __SRCPND);
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
	if (VIRQ_EINT0 <= virq && virq <= VIRQ_ADC) {
        irq_set_chip_data(virq, &parent_irq_chipdata);
    	irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);

        /* setup the chain handler */
        if (virq == VIRQ_EINT4_7 || virq == VIRQ_EINT8_23
			|| virq == VIRQ_INT_CAM || virq == VIRQ_WDT_AC97
			|| virq == VIRQ_UART2 || virq == VIRQ_UART1
            || virq == VIRQ_UART0 || virq == VIRQ_ADC)
        	irq_set_chained_handler(virq, s3c_irq_demux);
	}

	if (VIRQ_EINT4 <= virq && virq <= VIRQ_EINT23) {
        irq_set_chip_data(virq, &subeint_irq_chipdata);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
	}

	if (VIRQ_UART0_RXD <= virq && virq <= VIRQ_WDT_AC97_AC97) {
        irq_set_chip_data(virq, &submisc_irq_chipdata);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
	}

	return 0;
}

static const struct irq_domain_ops s3c24xx_irq_ops = {
	.map = qin2440_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

void __init qin2440_init_irq(void)
{
	struct irq_domain *subeint_domain;
	struct irq_domain *submisc_domain;
	int i;

	/* all interrupts disable */
	__raw_writel(0xFFFFFFFF, __INTMSK);
	__raw_writel(0xFFFFFFFF, __INTSUBMSK);
	__raw_writel(0xFFFFFFFF, __EINTMASK);

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

    /*
     * setup the irq entry
     */
    set_handle_irq(s3c24xx_handle_irq);
}
