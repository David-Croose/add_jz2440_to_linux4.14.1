/**
 * enable the clock source
 * @param clknum: could be 'CLKSRC_*', see include/mach/clock.h
 */
void clock_enable(unsigned int clknum)
{
	unsigned int tmp;

	tmp = *(volatile unsigned int *)__CLKCON;
	tmp |= clknum;
	*(volatile unsigned int *)__CLKCON = tmp;
}

/**
 * disable the clock source
 * @param clknum: could be 'CLKSRC_*', see include/mach/clock.h
 */
void clock_disable(unsigned int clknum)
{
	unsigned int tmp;

	tmp = *(volatile unsigned int *)__CLKCON;
	tmp &= ~clknum;
	*(volatile unsigned int *)__CLKCON = tmp;
}
