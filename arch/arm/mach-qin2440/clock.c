#include <asm/arch/virt_addr.h>
#include <asm-arm/io.h>

/*
 * enable a specify peripheral clock
 * @param id: see 'CLKSRC_*' in include/mach/clock.h
 */
void peripheral_clock_enable(unsigned int id)
{
    unsigned int tmp;

    tmp = __raw_readl(__CLKCON);
    tmp |= id;
    __raw_writel(tmp, __CLKCON);
}

/*
 * disable a specify peripheral clock
 * @param id: see 'CLKSRC_*' in include/mach/clock.h
 */
void peripheral_clock_disable(unsigned int id)
{
    unsigned int tmp;

    tmp = __raw_readl(__CLKCON);
    tmp &= ~id;
    __raw_writel(tmp, __CLKCON);
}

void s3c2440_clock_init(void)
{
    /*
	 * FCLK = FCLK / 1 = 400MHz / 1 = 400MHz
     * HCLK = FCLK / 4 = 400MHz / 4 = 100MHz
     * PCLK = FCLK / 8 = 400MHz / 8 = 50MHz
	 */
	__raw_writel(5, __CLKDIVN);

    __asm__
    (
        "mrc    p15, 0, r1, c1, c0, 0\r\n"
        "orr    r1, r1, #0xC0000000\r\n"
        "mcr    p15, 0, r1, c1, c0, 0\r\n"
    );

	__raw_writel((0x5C << 12) | (0x01 << 4) | (0x01), __MPLLCON);
}
