#ifndef __STDIO_H__
#define __SDTIO_H__

#include "tinyprintf.h"

#define stdout 0
#define stderr 1
#define stdin 2

int puts(const char *str);
int printf(const char *fmt, ...);

int vprintf(const char *format, va_list ap);

int fflush(void *stream);

#endif
