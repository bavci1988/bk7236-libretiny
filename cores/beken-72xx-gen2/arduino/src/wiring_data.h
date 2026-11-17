#pragma once

#include <ArduinoPrivate.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum lt_pwm_state_tag { LT_PWM_STOPPED, LT_PWM_RUNNING } lt_pwm_state_t;

struct PinData_s {
	uint8_t pwmChannel;
	lt_pwm_state_t pwmState;
	PinMode gpioMode;
	PinStatus irqMode;
	void *irqHandler;
	void *irqParam;
	bool irqChange;
};

#ifdef __cplusplus
} // extern "C"
#endif
