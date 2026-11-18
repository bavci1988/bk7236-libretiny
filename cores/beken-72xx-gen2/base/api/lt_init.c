#include <libretiny.h>
#include <sdk_private.h>

#include <components/system.h>

void lt_init_family() {
	// set default UART output port (ARMINO prints to UART_ID_0 by default);
	// this also moves ARMINO's own log output there
	bk_set_printf_port(LT_UART_DEFAULT_PORT - 1);
}
