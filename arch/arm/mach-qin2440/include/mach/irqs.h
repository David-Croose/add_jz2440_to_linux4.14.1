#ifndef _ASM_ARCH_IRQ_H_
#define _ASM_ARCH_IRQ_H_

#include <linux/compiler.h>

/*
 * sub misc irq
 */
#define IRQ_WDT_AC97_AC97 66
#define IRQ_WDT_AC97_WDT  65
#define IRQ_CAM_P         64
#define IRQ_CAM_C         63
#define IRQ_ADC_S         62
#define IRQ_ADC_TC        61
#define IRQ_UART2_ERR     60
#define IRQ_UART2_TXD     59
#define IRQ_UART2_RXD     58
#define IRQ_UART1_ERR     57
#define IRQ_UART1_TXD     56
#define IRQ_UART1_RXD     55
#define IRQ_UART0_ERR     54
#define IRQ_UART0_TXD     53
#define IRQ_UART0_RXD     52

/*
 * sub eint irq
 */
#define IRQ_EINT23        51
#define IRQ_EINT22        50
#define IRQ_EINT21        49
#define IRQ_EINT20        48
#define IRQ_EINT19        47
#define IRQ_EINT18        46
#define IRQ_EINT17        45
#define IRQ_EINT16        44
#define IRQ_EINT15        43
#define IRQ_EINT14        42
#define IRQ_EINT13        41
#define IRQ_EINT12        40
#define IRQ_EINT11        39
#define IRQ_EINT10        38
#define IRQ_EINT9         37
#define IRQ_EINT8         36
#define IRQ_EINT7         35
#define IRQ_EINT6         34
#define IRQ_EINT5         33
#define IRQ_EINT4         32

/*
 * parent irq
 *
 * note that if a parent irq has child irq, it
 * can not be requested
 */
#define IRQ_ADC           31   /* can't be requested */
#define IRQ_RTC           30
#define IRQ_SPI1          29
#define IRQ_UART0         28   /* can't be requested */
#define IRQ_IIC           27
#define IRQ_USBH          26
#define IRQ_USBD          25
#define IRQ_NFCON         24
#define IRQ_UART1         23   /* can't be requested */
#define IRQ_SPI0          22
#define IRQ_SDI           21
#define IRQ_DMA3          20
#define IRQ_DMA2          19
#define IRQ_DMA1          18
#define IRQ_DMA0          17
#define IRQ_LCD           16
#define IRQ_UART2         15   /* can't be requested */
#define IRQ_TIMER4        14
#define IRQ_TIMER3        13
#define IRQ_TIMER2        12
#define IRQ_TIMER1        11
#define IRQ_TIMER0        10
#define IRQ_WDT_AC97      9    /* can't be requested */
#define IRQ_TICK          8
#define IRQ_BATT_FLT      7
#define IRQ_INT_CAM       6    /* can't be requested */
#define IRQ_EINT8_23      5    /* can't be requested */
#define IRQ_EINT4_7       4    /* can't be requested */
#define IRQ_EINT3         3
#define IRQ_EINT2         2
#define IRQ_EINT1         1
#define IRQ_EINT0         0

#define NR_IRQS			  67

/*
 * some handy infomation
 */
#define PARENTIRQ_TOTAL  	  (IRQ_ADC - IRQ_EINT0 + 1)
#define SUBIRQ_MISC_TOTAL     (IRQ_WDT_AC97_AC97 - IRQ_UART0_RXD + 1)
#define SUBIRQ_EINT_TOTAL     (IRQ_EINT23 - IRQ_EINT4 + 1)

/*
 * user's stuff
 */
#define INVALIRQ		(unsigned int)(-1)

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
    unsigned int irq_start;
    struct dom_priv_data *parent;
    int (*get_child)(unsigned int hwirq, struct irq_domain **child_dom,
			unsigned int *child_hwirq);
};

struct virq_priv_data {
	unsigned int hwirq;
	unsigned int parent_hwirq;
	struct dom_priv_data *more;
};

#endif

