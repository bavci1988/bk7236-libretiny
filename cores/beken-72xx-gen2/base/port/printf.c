#include <libretiny.h>
#include <sdk_private.h>

#include <components/system.h>
#include <printf/printf.h>

extern bk_err_t uart_write_byte(uart_id_t id, uint8_t data);

void putchar_(char c) {
	uart_write_byte(LT_UART_DEFAULT_PORT - 1, c);
}

void putchar_p(char c, unsigned long port) {
	uart_write_byte((port & 0xFF) - 1, c);
}
WRAP_PRINTF(bk_printf);

// in this file, as it is always linked: lt_main() only has a weak reference to lt_init_family()
void lt_init_family() {
	// set default UART output port (ARMINO prints to UART_ID_0 by default);
	// this also moves ARMINO's own log output there
	bk_set_printf_port(LT_UART_DEFAULT_PORT - 1);
}
