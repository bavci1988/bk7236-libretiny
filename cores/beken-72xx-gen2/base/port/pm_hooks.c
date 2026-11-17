#include <libretiny.h>
#include <sdk_private.h>

#include <driver/uart.h>

#include <FreeRTOS.h>
#include <task.h>

// ARMINO UART IDs used for TX, besides the console (set by SerialClass)
uint32_t lt_uart_tx_mask = 0;

// keep awake for this many ticks after the last UART TX FIFO data
// (the last character is still in the shift register: 1 ms at 9600 baud)
#define UART_TX_TAIL_TICKS 2

static TickType_t uartBusyTick = 0;

// Called by the FreeRTOS tickless idle code right before sleeping, see configPRE_SLEEP_PROCESSING.
// The low-voltage sleep stops the UART clock without waiting for TX, which cuts log output and
// UART frames - setting the idle time to 0 skips this sleep attempt.
void lt_pre_sleep_processing(void *idle_time) {
	TickType_t *idle = (TickType_t *)idle_time;
	uint32_t mask	 = lt_uart_tx_mask | (1 << (LT_UART_DEFAULT_PORT - 1));
	TickType_t now	 = xTaskGetTickCount();
	for (uart_id_t id = UART_ID_0; id < UART_ID_MAX; id++) {
		if ((mask & (1 << id)) && !bk_uart_is_tx_over(id)) {
			uartBusyTick = now;
			*idle		 = 0;
			return;
		}
	}
	if ((TickType_t)(now - uartBusyTick) < UART_TX_TAIL_TICKS)
		*idle = 0;
}
