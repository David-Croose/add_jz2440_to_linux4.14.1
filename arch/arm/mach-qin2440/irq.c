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

static int get_child(unsigned int hwirq, struct irq_domain **child_dom,
						unsigned int *child_hwirq);

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

	IRQITEM(IRQ_ADC, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_RTC, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_SPI1, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_UART0, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_IIC, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_USBH, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_USBD, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_NFCON, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_UART1, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_SPI0, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_SDI, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_DMA3, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_DMA2, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_DMA1, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_DMA0, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_LCD, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_UART2, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER4, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER3, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER2, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER1, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TIMER0, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_WDT_AC97, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_TICK, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_BATT_FLT, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_INT_CAM, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT8_23, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT4_7, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT3, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT2, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT1, INVALIRQ, &irq_parent),
	IRQITEM(IRQ_EINT0, INVALIRQ, &irq_parent),
};

struct dom_priv_data irq_parent = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __INTPND,
    .reg_mask = __INTMSK,
    .irq_start = IRQ_EINT0,
    .parent = NULL,
	.get_child = get_child,
};

struct dom_priv_data irq_subeint = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __EINTPEND,
    .reg_mask = __EINTMASK,
    .irq_start = IRQ_EINT4,
    .parent = &irq_parent,
	.get_child = NULL,
};

struct dom_priv_data irq_submisc = {
    .domain = NULL,    /* will be implemented in the latter */
    .reg_pending = __SUBSRCPND,
    .reg_mask = __INTSUBMSK,
    .irq_start = IRQ_UART0_RXD,
    .parent = &irq_parent,
	.get_child = NULL,
};

static int get_child(unsigned int hwirq, struct irq_domain **child_dom,
						unsigned int *child_hwirq)
{
	unsigned int pnd, msk;
	unsigned int rel;

	if (hwirq == IRQ_EINT4_7 || hwirq == IRQ_EINT8_23) {
		pnd = __raw_readl(__EINTPEND);
		if (!pnd)		/* if no sub irqs */
			return -2;

		msk = __raw_readl(__EINTMASK);
		pnd &= ~msk;
		rel = __ffs(pnd);
		*child_hwirq = rel - 4;
		*child_dom = irq_subeint.domain;
		return 0;
	}

	if (hwirq == IRQ_INT_CAM || hwirq == IRQ_WDT_AC97
		|| hwirq == IRQ_UART2 || hwirq == IRQ_UART1
		|| hwirq == IRQ_UART0 || hwirq == IRQ_ADC) {
		pnd = __raw_readl(__SUBSRCPND);
		if (!pnd)		/* if no sub irqs */
			return -3;

		msk = __raw_readl(__INTSUBMSK);
		pnd &= ~msk;
		*child_hwirq = __ffs(pnd);
		*child_dom = irq_submisc.domain;
		return 0;
	}

	return -1;
}

static void s3c_irq_mask(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = readl_relaxed(d->more->reg_mask);
	mask |= 1UL << d->hwirq;
	writel_relaxed(mask, d->more->reg_mask);

    if (d->more->parent) {
        mask = readl_relaxed(d->more->parent->reg_mask);
        mask |= 1UL << d->parent_hwirq;
        writel_relaxed(mask, d->more->parent->reg_mask);
    }
}

static void s3c_irq_unmask(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);
    unsigned long mask;

	mask = readl_relaxed(d->more->reg_mask);
	mask &= ~(1UL << d->hwirq);
	writel_relaxed(mask, d->more->reg_mask);

    if (d->more->parent) {
        mask = readl_relaxed(d->more->parent->reg_mask);
        mask &= ~(1UL << d->parent_hwirq);
        writel_relaxed(mask, d->more->parent->reg_mask);
    }
}

static inline void s3c_irq_ack(struct irq_data *data)
{
    struct virq_priv_data *d = irq_data_get_irq_chip_data(data);

    if (d->more->parent)
    	writel_relaxed(1UL << d->parent_hwirq, d->more->parent->reg_pending);

    writel_relaxed(1UL << d->hwirq, d->more->reg_pending);
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
	struct virq_priv_data *d = irq_desc_get_chip_data(desc);
	unsigned int virq;
	struct irq_domain *child_dom;
	unsigned int child_hwirq;

    if (!d->more->get_child) {
        return;
	}

	chained_irq_enter(chip, desc);

	while (!d->more->get_child(d->hwirq, &child_dom, &child_hwirq)) {
		virq = irq_find_mapping(child_dom, d->more->irq_start + child_hwirq);
		generic_handle_irq(virq);
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

        handle_domain_irq(irq_parent.domain, irq_parent.irq_start + offset, regs);
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

void __init qin2440_init_irq(void)
{
    struct irq_domain *parent_domain;
    struct irq_domain *subeint_domain;
    struct irq_domain *submisc_domain;
    unsigned int virq;
    unsigned int hwirq;

    /*
     * setup irq domain
     */
    parent_domain = irq_domain_add_legacy(NULL, PARENTIRQ_TOTAL,
									16,
									IRQ_EINT0,
									&s3c24xx_irq_ops, NULL);

    subeint_domain = irq_domain_add_legacy(NULL, SUBIRQ_EINT_TOTAL,
									16 + PARENTIRQ_TOTAL,
									IRQ_EINT4,
									&s3c24xx_irq_ops, NULL);

    submisc_domain = irq_domain_add_legacy(NULL, SUBIRQ_MISC_TOTAL,
									16 + PARENTIRQ_TOTAL + SUBIRQ_EINT_TOTAL,
									IRQ_UART0_RXD,
									&s3c24xx_irq_ops, NULL);
    irq_parent.domain = parent_domain;
    irq_subeint.domain = subeint_domain;
    irq_submisc.domain = submisc_domain;

    /*
     * fill the irq chip, handler and chain-handler
     */
    for (hwirq = IRQ_EINT0; hwirq <= IRQ_ADC; hwirq++) {
        virq = irq_find_mapping(parent_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[hwirq]);

        /* setup the chain handler */
        if (hwirq == IRQ_EINT4_7 || hwirq == IRQ_EINT8_23
			|| hwirq == IRQ_INT_CAM || hwirq == IRQ_WDT_AC97
			|| hwirq == IRQ_UART2 || hwirq == IRQ_UART1
            || hwirq == IRQ_UART0 || hwirq == IRQ_ADC) {
        	irq_set_chained_handler(virq, s3c_irq_demux);
        }
    }
    for (hwirq = IRQ_EINT4; hwirq <= IRQ_EINT23; hwirq++) {
        virq = irq_find_mapping(subeint_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[hwirq]);
    }
    for (hwirq = IRQ_UART0_RXD; hwirq <= IRQ_WDT_AC97_AC97; hwirq++) {
        virq = irq_find_mapping(submisc_domain, hwirq);
        irq_set_chip_and_handler(virq, &s3c_irq_level_chip, handle_level_irq);
        irq_set_chip_data(virq, &virq_tbl[hwirq]);
    }

    /*
     * setup the irq entry
     */
    set_handle_irq(s3c24xx_handle_irq);

	////////////////////////////////////////////////////////////////
	int m;
	for (m = 0; m < 10; m++) {
		while(!((*(volatile unsigned int *)__UTRSTAT0) & (1 << 2)));
		*(volatile unsigned char *)__UTXH0 = 'i';
	}
	////////////////////////////////////////////////////////////////
}
