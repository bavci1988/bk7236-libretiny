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
	bool irqWake;	  // registered as light sleep wake-up source
	int8_t keepLevel; // level held during light sleep: 0/1, PIN_KEEP_NONE or PIN_KEEP_FULL
};

#define PIN_KEEP_NONE (-1)
#define PIN_KEEP_FULL (-2)

void pinKeepLevel(PinInfo *pin, PinData *data, int8_t level);

#ifdef __cplusplus
} // extern "C"
#endif
