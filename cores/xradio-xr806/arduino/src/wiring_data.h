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
	uint32_t pwmPeriod;
	uint16_t pwmMax;
};

void pwm_stop(PinInfo *pin);

// GPIO numbers: PAx is x, PBx is 32 + x
#define GPIO_GET_PORT(pin) ((pin) >= 32 ? GPIO_PORT_B : GPIO_PORT_A)
#define GPIO_GET_PIN(pin)  ((GPIO_Pin)((pin) & 0x1F))

#ifdef __cplusplus
} // extern "C"
#endif
