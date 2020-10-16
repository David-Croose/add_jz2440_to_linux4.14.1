#ifndef _ASM_ARCH_IRQ_H_
#define _ASM_ARCH_IRQ_H_

#include <linux/compiler.h>

//===============================================================================
// hardware irq

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

#define NR_IRQS			  (IRQ_WDT_AC97_AC97 - IRQ_EINT0 + 1 + 16)

/*
 * some handy infomation
 */
#define PARENTIRQ_TOTAL  	  (IRQ_ADC - IRQ_EINT0 + 1)
#define SUBIRQ_MISC_TOTAL     (IRQ_WDT_AC97_AC97 - IRQ_UART0_RXD + 1)
#define SUBIRQ_EINT_TOTAL     (IRQ_EINT23 - IRQ_EINT4 + 1)

//===============================================================================
// virtual irq

#define HWIRQ2VIRQ(x)		((x) + 16)
#define VIRQ2HWIRQ(x)		((x) - 16)

#define VIRQ_WDT_AC97_AC97 HWIRQ2VIRQ(IRQ_WDT_AC97_AC97)
#define VIRQ_WDT_AC97_WDT  HWIRQ2VIRQ(IRQ_WDT_AC97_WDT)
#define VIRQ_CAM_P         HWIRQ2VIRQ(IRQ_CAM_P)
#define VIRQ_CAM_C         HWIRQ2VIRQ(IRQ_CAM_C)
#define VIRQ_ADC_S         HWIRQ2VIRQ(IRQ_ADC_S)
#define VIRQ_ADC_TC        HWIRQ2VIRQ(IRQ_ADC_TC)
#define VIRQ_UART2_ERR     HWIRQ2VIRQ(IRQ_UART2_ERR)
#define VIRQ_UART2_TXD     HWIRQ2VIRQ(IRQ_UART2_TXD)
#define VIRQ_UART2_RXD     HWIRQ2VIRQ(IRQ_UART2_RXD)
#define VIRQ_UART1_ERR     HWIRQ2VIRQ(IRQ_UART1_ERR)
#define VIRQ_UART1_TXD     HWIRQ2VIRQ(IRQ_UART1_TXD)
#define VIRQ_UART1_RXD     HWIRQ2VIRQ(IRQ_UART1_RXD)
#define VIRQ_UART0_ERR     HWIRQ2VIRQ(IRQ_UART0_ERR)
#define VIRQ_UART0_TXD     HWIRQ2VIRQ(IRQ_UART0_TXD)
#define VIRQ_UART0_RXD     HWIRQ2VIRQ(IRQ_UART0_RXD)

#define VIRQ_EINT23        HWIRQ2VIRQ(IRQ_EINT23)
#define VIRQ_EINT22        HWIRQ2VIRQ(IRQ_EINT22)
#define VIRQ_EINT21        HWIRQ2VIRQ(IRQ_EINT21)
#define VIRQ_EINT20        HWIRQ2VIRQ(IRQ_EINT20)
#define VIRQ_EINT19        HWIRQ2VIRQ(IRQ_EINT19)
#define VIRQ_EINT18        HWIRQ2VIRQ(IRQ_EINT18)
#define VIRQ_EINT17        HWIRQ2VIRQ(IRQ_EINT17)
#define VIRQ_EINT16        HWIRQ2VIRQ(IRQ_EINT16)
#define VIRQ_EINT15        HWIRQ2VIRQ(IRQ_EINT15)
#define VIRQ_EINT14        HWIRQ2VIRQ(IRQ_EINT14)
#define VIRQ_EINT13        HWIRQ2VIRQ(IRQ_EINT13)
#define VIRQ_EINT12        HWIRQ2VIRQ(IRQ_EINT12)
#define VIRQ_EINT11        HWIRQ2VIRQ(IRQ_EINT11)
#define VIRQ_EINT10        HWIRQ2VIRQ(IRQ_EINT10)
#define VIRQ_EINT9         HWIRQ2VIRQ(IRQ_EINT9)
#define VIRQ_EINT8         HWIRQ2VIRQ(IRQ_EINT8)
#define VIRQ_EINT7         HWIRQ2VIRQ(IRQ_EINT7)
#define VIRQ_EINT6         HWIRQ2VIRQ(IRQ_EINT6)
#define VIRQ_EINT5         HWIRQ2VIRQ(IRQ_EINT5)
#define VIRQ_EINT4         HWIRQ2VIRQ(IRQ_EINT4)

#define VIRQ_ADC           HWIRQ2VIRQ(IRQ_ADC)        /* can't be requested */
#define VIRQ_RTC           HWIRQ2VIRQ(IRQ_RTC)
#define VIRQ_SPI1          HWIRQ2VIRQ(IRQ_SPI1)
#define VIRQ_UART0         HWIRQ2VIRQ(IRQ_UART0)      /* can't be requested */
#define VIRQ_IIC           HWIRQ2VIRQ(IRQ_IIC)
#define VIRQ_USBH          HWIRQ2VIRQ(IRQ_USBH)
#define VIRQ_USBD          HWIRQ2VIRQ(IRQ_USBD)
#define VIRQ_NFCON         HWIRQ2VIRQ(IRQ_NFCON)
#define VIRQ_UART1         HWIRQ2VIRQ(IRQ_UART1)      /* can't be requested */
#define VIRQ_SPI0          HWIRQ2VIRQ(IRQ_SPI0)
#define VIRQ_SDI           HWIRQ2VIRQ(IRQ_SDI)
#define VIRQ_DMA3          HWIRQ2VIRQ(IRQ_DMA3)
#define VIRQ_DMA2          HWIRQ2VIRQ(IRQ_DMA2)
#define VIRQ_DMA1          HWIRQ2VIRQ(IRQ_DMA1)
#define VIRQ_DMA0          HWIRQ2VIRQ(IRQ_DMA0)
#define VIRQ_LCD           HWIRQ2VIRQ(IRQ_LCD)
#define VIRQ_UART2         HWIRQ2VIRQ(IRQ_UART2)      /* can't be requested */
#define VIRQ_TIMER4        HWIRQ2VIRQ(IRQ_TIMER4)
#define VIRQ_TIMER3        HWIRQ2VIRQ(IRQ_TIMER3)
#define VIRQ_TIMER2        HWIRQ2VIRQ(IRQ_TIMER2)
#define VIRQ_TIMER1        HWIRQ2VIRQ(IRQ_TIMER1)
#define VIRQ_TIMER0        HWIRQ2VIRQ(IRQ_TIMER0)
#define VIRQ_WDT_AC97      HWIRQ2VIRQ(IRQ_WDT_AC97)   /* can't be requested */
#define VIRQ_TICK          HWIRQ2VIRQ(IRQ_TICK)
#define VIRQ_BATT_FLT      HWIRQ2VIRQ(IRQ_BATT_FLT)
#define VIRQ_INT_CAM       HWIRQ2VIRQ(IRQ_INT_CAM)    /* can't be requested */
#define VIRQ_EINT8_23      HWIRQ2VIRQ(IRQ_EINT8_23)   /* can't be requested */
#define VIRQ_EINT4_7       HWIRQ2VIRQ(IRQ_EINT4_7)    /* can't be requested */
#define VIRQ_EINT3         HWIRQ2VIRQ(IRQ_EINT3)
#define VIRQ_EINT2         HWIRQ2VIRQ(IRQ_EINT2)
#define VIRQ_EINT1         HWIRQ2VIRQ(IRQ_EINT1)
#define VIRQ_EINT0         HWIRQ2VIRQ(IRQ_EINT0)

#endif
