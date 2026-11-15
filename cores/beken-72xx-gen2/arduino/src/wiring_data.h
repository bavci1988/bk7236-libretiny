#pragma once

#include <ArduinoPrivate.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PinData_s {
	PinMode gpioMode;
	PinStatus irqMode;
	void *irqHandler;
	void *irqParam;
	bool irqChange;
};

#ifdef __cplusplus
} // extern "C"
#endif
