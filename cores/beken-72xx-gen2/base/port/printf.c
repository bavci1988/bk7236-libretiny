#include <libretiny.h>
#include <sdk_private.h>

#include <printf/printf.h>

extern bk_err_t uart_write_byte(uart_id_t id, uint8_t data);

void putchar_(char c) {
	uart_write_byte(LT_UART_DEFAULT_PORT - 1, c);
}

void putchar_p(char c, unsigned long port) {
	uart_write_byte((port & 0xFF) - 1, c);
}
WRAP_PRINTF(bk_printf);
