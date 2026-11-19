#include <libretiny.h>
#include <sdk_private.h>

#include <components/system.h>
#include <modules/pm.h>
#include <printf/printf.h>

extern bk_err_t uart_write_byte(uart_id_t id, uint8_t data);

void putchar_(char c) {
	uart_write_byte(LT_UART_DEFAULT_PORT, c);
}

void putchar_p(char c, unsigned long port) {
	uart_write_byte(port & 0xFF, c);
}
WRAP_PRINTF(bk_printf);

// in this file, as it is always linked: lt_main() only has a weak reference to lt_init_family()
void lt_init_family() {
	// set default UART output port (ARMINO prints to UART_ID_0 by default);
	// this also moves ARMINO's own log output there
	bk_set_printf_port(LT_UART_DEFAULT_PORT);
#if LT_AUTO_LIGHT_SLEEP
	// the WiFi library decides with the APP1 vote; the application may veto with lt_light_sleep_allow(false)
	bk_pm_module_vote_sleep_ctrl(PM_SLEEP_MODULE_NAME_APP1, 0, 0);
	bk_pm_module_vote_sleep_ctrl(PM_SLEEP_MODULE_NAME_APP, 1, 0);
#endif
}
