#ifndef __UNCOMPRESS_H_
#define __UNCOMPRESS_H_

static inline void putc(char ch)
{
	while(!((*(volatile unsigned int *)0x50000010) & (1 << 2)));
	*(volatile char *)0x50000020 = (char)ch;
	while(!((*(volatile unsigned int *)0x50000010) & (1 << 2)));
}

static inline void flush(void)
{
	
}

#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif
