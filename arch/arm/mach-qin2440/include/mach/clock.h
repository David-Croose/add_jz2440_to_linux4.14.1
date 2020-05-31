#ifndef _ASM_ARCH_CLOCK_
#define _ASM_ARCH_CLOCK_

/**
 * the clock source
 */
#define CLKSRC_AC97      (1 << 20)
#define CLKSRC_CAM       (1 << 19)
#define CLKSRC_SPI       (1 << 18)
#define CLKSRC_IIS       (1 << 17)
#define CLKSRC_IIC       (1 << 16)
#define CLKSRC_ADC       (1 << 15)
#define CLKSRC_RTC       (1 << 14)
#define CLKSRC_GPIO      (1 << 13)
#define CLKSRC_UART2     (1 << 12)
#define CLKSRC_UART1     (1 << 11)
#define CLKSRC_UART0     (1 << 10)
#define CLKSRC_SDI       (1 << 9)
#define CLKSRC_PWMTIMER  (1 << 8)
#define CLKSRC_USBD      (1 << 7)
#define CLKSRC_USBH      (1 << 6)
#define CLKSRC_LCD       (1 << 5)
#define CLKSRC_NAND      (1 << 4)
#define CLKSRC_SLEEP     (1 << 3)
#define CLKSRC_IDLE      (1 << 2)

void peripheral_clock_enable(unsigned int id);
void peripheral_clock_disable(unsigned int id);
void s3c2440_clock_init(void);

#endif

