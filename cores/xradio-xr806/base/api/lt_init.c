#include <libretiny.h>
#include <sdk_private.h>

#include "pm/pm.h"
#include "sys/xr_debug.h"

void lt_init_family() {
	// clocks, GPIO, stdout and the SDK services are started by the SDK (xr806_main.c)
	// PM prints several lines on every standby; keep its warnings and errors
	pm_set_debug_mask(ROM_WRN_MASK | ROM_ERR_MASK | ROM_ABORT_MASK);
}

void lt_init_arduino() {
#if LT_AUTO_DOWNLOAD_REBOOT && defined(LT_UART_ADR_PATTERN) && LT_ARD_HAS_SERIAL && LT_HW_UART0
	// initialize auto-download-reboot parser
	Serial0.begin(115200);
#endif
}
