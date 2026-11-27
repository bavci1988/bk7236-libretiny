#include <libretiny.h>
#include <sdk_private.h>

#include <printf/printf.h>

uint8_t uart_print_port = LT_UART_DEFAULT_LOGGER;

void putchar_(char c) {
	putchar_p(c, uart_print_port);
}

// The SDK's writer for its console UART (retarget_stdout.c), set by stdout_init()
static int (*console_write)(const char *buf, int len);

void stdio_set_write(int (*fn)(const char *buf, int len)) {
	console_write = fn;
}

void putchar_p(char c, unsigned long port) {
	// PM switches the console UART off for standby and prints before it is on again (also the
	// SDK and the ROM do). The SDK writer drops that output; a write to the UART itself would
	// wait forever for the transmitter.
	if (port == BOARD_MAIN_UART_ID && console_write != NULL) {
		console_write(&c, 1);
		return;
	}
	HAL_UART_Transmit_Poll((UART_ID)port, (uint8_t *)&c, 1);
}
