/* Copyright (c) Kuba Szczodrzyński 2022-06-23. */

#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

extern "C" {
#include <driver/uart.h>

extern int uart_read_byte(uart_id_t id);
extern bk_err_t uart_write_byte(uart_id_t id, uint8_t data);
}

// LibreTiny port N is SoC UART N (datasheet numbering), which is ARMINO UART_ID_N
#define UART_ID(port) ((uart_id_t)(port))

static void callback(uart_id_t id, void *param) {
	SerialRingBuffer *buf = (SerialRingBuffer *)param;
	int ch;
	while ((ch = uart_read_byte(id)) != -1) {
		buf->store_char(ch);
	}
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {
	// RX is set up in configure(), as bk_uart_init() resets the interrupt enables
}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	uint8_t dataWidth = ((config & SERIAL_DATA_MASK) >> 8) - 1;				  // 0x100..0x400 -> 0..3
	uint8_t parity	  = 3 - (config & SERIAL_PARITY_MASK);					  // 0x3..0x1 -> 0..2
	uint8_t stopBits  = (config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2; // 0x10..0x30 -> 0..1

	uart_config_t cfg = {
		.baud_rate = (uint32_t)baudrate,
		.data_bits = (uart_data_bits_t)dataWidth,
		.parity	   = (uart_parity_t)parity,
		.stop_bits = (uart_stop_bits_t)stopBits,
		.flow_ctrl = UART_FLOWCTRL_DISABLE,
		.src_clk   = UART_SCLK_XTAL_26M,
		.rx_dma_en = UART_DMA_DISABLE,
		.tx_dma_en = UART_DMA_DISABLE,
	};
	bk_uart_init(UART_ID(this->port), &cfg);

	if (this->rx != PIN_INVALID) {
		// read the RX FIFO in our own ISR instead of the SDK's software FIFO
		bk_uart_disable_sw_fifo(UART_ID(this->port));
		bk_uart_register_rx_isr(UART_ID(this->port), callback, this->rxBuf);
		bk_uart_enable_rx_interrupt(UART_ID(this->port));
	}

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data)
		return;

	bk_uart_disable_rx_interrupt(UART_ID(this->port));
	bk_uart_register_rx_isr(UART_ID(this->port), NULL, NULL);
	bk_uart_enable_sw_fifo(UART_ID(this->port));
}

void SerialClass::flush() {
	if (!this->data)
		return;
	bk_uart_wait_tx_over(UART_ID(this->port));
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data)
		return 0;
	uart_write_byte(UART_ID(this->port), c);
	return 1;
}

#endif
