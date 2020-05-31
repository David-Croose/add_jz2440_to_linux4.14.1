#include <linux/kernel.h>

extern void printascii(const char *);

void qin_printf(const char *fmt, ...)
{
	va_list va;
	char buff[256];

	va_start(va, fmt);
	vsprintf(buff, fmt, va);
	va_end(va);

	printascii(buff);
}
