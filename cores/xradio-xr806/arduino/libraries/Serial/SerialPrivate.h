#pragma once

#include <ArduinoPrivate.h>
#include <Serial.h>

struct SerialData {
	UART_T *uart;
	SerialRingBuffer *rxBuf;
	uint8_t port;
	bool sdkConsole; //!< opened by the SDK for stdout, which keeps using it
};
