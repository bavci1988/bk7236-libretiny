#include <libretiny.h>
#include <sdk_private.h>

#include <stdarg.h>
#include <stdio.h>

// The ROM calls these through its RAM table (src/rom/rom_core.c); printf(), vprintf(),
// puts() and putchar() come from the LibreTiny printf library.

extern int __wrap_vprintf(const char *format, va_list ap);
extern int __wrap_puts(const char *s);
extern int __wrap_putchar(int c);

int __wrap_vfprintf(FILE *stream, const char *format, va_list ap) {
	return __wrap_vprintf(format, ap);
}

int __wrap_fputs(const char *s, FILE *stream) {
	return __wrap_puts(s);
}

int __wrap_putc(int c, FILE *stream) {
	return __wrap_putchar(c);
}

int __wrap_fputc(int c, FILE *stream) {
	return __wrap_putchar(c);
}

int __wrap_fflush(FILE *stream) {
	return 0;
}
