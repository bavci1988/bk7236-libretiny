#if LT_ARD_HAS_SERIAL || DOXYGEN

#include "SerialPrivate.h"

#include <xr806_pm.h>

#define DATA ((SerialData *)this->data)

// the error of the received byte; the FIFO error bit stays set while any bad byte is in the FIFO
#define LINE_ERROR_BITS (UART_FRAME_ERROR_BIT | UART_PARITY_ERROR_BIT | UART_OVERRUN_ERROR_BIT)

// called by the HAL UART interrupt handler; arg is the SerialData of the port
static void callback_rx(void *arg) {
	SerialData *data = (SerialData *)arg;
	uint32_t errors	 = 0;
	uint32_t bytes	 = 0;
	// reading the line status clears its error bits, so it is read once per byte
	for (;;) {
		uint32_t status = data->uart->LINE_STATUS;
		if (status & LINE_ERROR_BITS)
			errors++;
		if (!(status & UART_RX_READY_BIT))
			break;
		uint8_t c = HAL_UART_GetRxData(data->uart);
		bytes++;
		if (data->rxBuf->availableForStore() == 0)
			errors++; // dropped
		else
			data->rxBuf->store_char(c);
	}
	xr806_uart_note_activity(data->port, bytes, errors);
}

void SerialClass::beginPrivate(unsigned long baudrate, uint16_t config) {}

void SerialClass::configure(unsigned long baudrate, uint16_t config) {
	if (!this->data)
		return;

	UART_InitParam param = {
		.baudRate		  = (uint32_t)baudrate,
		.parity			  = UART_PARITY_NONE,
		.stopBits		  = UART_STOP_BITS_1,
		.dataBits		  = UART_DATA_BITS_8,
		.isAutoHwFlowCtrl = 0,
	};
	switch (config & SERIAL_PARITY_MASK) {
		case SERIAL_PARITY_EVEN:
			param.parity = UART_PARITY_EVEN;
			break;
		case SERIAL_PARITY_ODD:
			param.parity = UART_PARITY_ODD;
			break;
	}
	if ((config & SERIAL_STOP_BIT_MASK) == SERIAL_STOP_BIT_2)
		param.stopBits = UART_STOP_BITS_2;
	switch (config & SERIAL_DATA_MASK) {
		case SERIAL_DATA_5:
			param.dataBits = UART_DATA_BITS_5;
			break;
		case SERIAL_DATA_6:
			param.dataBits = UART_DATA_BITS_6;
			break;
		case SERIAL_DATA_7:
			param.dataBits = UART_DATA_BITS_7;
			break;
	}

	if (DATA->uart) {
		// a running port keeps its pins, so the other side sees no break (e.g. on a baud rate change)
		flush();
		if (HAL_UART_SetConfig((UART_ID)this->port, &param) != HAL_OK)
			return;
	} else {
		// the HAL sets the pin functions of the port through the board configuration;
		// the SDK opens the console port (UART2) for stdout before the application starts
		HAL_Status status = HAL_UART_Init((UART_ID)this->port, &param);
		if (status == HAL_BUSY) {
			status			 = HAL_UART_SetConfig((UART_ID)this->port, &param);
			DATA->sdkConsole = true;
		}
		if (status != HAL_OK)
			return;
		DATA->uart	= HAL_UART_GetInstance((UART_ID)this->port);
		DATA->rxBuf = this->rxBuf;
		DATA->port	= this->port;
	}
	HAL_UART_EnableRxCallback((UART_ID)this->port, callback_rx, this->data);

	this->baudrate = baudrate;
	this->config   = config;
}

void SerialClass::endPrivate() {
	if (!this->data)
		return;
	if (DATA->uart) {
		HAL_UART_DisableRxCallback((UART_ID)this->port);
		// the SDK console stays open for stdout (printf, ROM and SDK messages)
		if (!DATA->sdkConsole)
			HAL_UART_DeInit((UART_ID)this->port);
		DATA->uart = NULL;
	}
}

void SerialClass::flush() {
	if (!this->data || !DATA->uart)
		return;
	// the TX FIFO and the shift register
	xr806_uart_wait_tx_done(this->port, UINT32_MAX);
}

size_t SerialClass::write(uint8_t c) {
	if (!this->data || !DATA->uart)
		return 0;
	HAL_UART_Transmit_Poll((UART_ID)this->port, &c, 1);
	xr806_uart_note_activity(this->port, 1, 0);
	return 1;
}

#endif
