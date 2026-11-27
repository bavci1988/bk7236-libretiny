#include <ArduinoPrivate.h>

#include <xr806_pm.h>

#include "driver/chip/hal_util.h"

void delayMicroseconds(unsigned int us) {
	HAL_UDelay(us);
}

unsigned long millis(void) {
	// the RTC free-running counter, like micros(): the FreeRTOS tick stops in standby
	return (unsigned long)(xr806_uptime_us() / 1000);
}

unsigned long micros(void) {
	// the RTC free-running counter (LFCLK, about 31 us resolution), which also counts in standby
	return (unsigned long)xr806_uptime_us();
}

void pinRemoveMode(PinInfo *pin, uint32_t mask) {
	if ((mask & PIN_PWM) && (pin->enabled & PIN_PWM)) {
		pwm_stop(pin);
		pinDisable(pin, PIN_PWM);
	}
	if ((mask & PIN_IRQ) && (pin->enabled & PIN_IRQ)) {
		HAL_GPIO_DisableIRQ(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio));
		pinData(pin)->irqHandler = NULL;
		pinDisable(pin, PIN_IRQ);
		// back from the EINT pin function to the GPIO mode
		GPIO_InitParam param;
		HAL_GPIO_GetConfig(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
		param.mode = pinData(pin)->gpioMode == OUTPUT ? GPIOx_Pn_F1_OUTPUT : GPIOx_Pn_F0_INPUT;
		HAL_GPIO_Init(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
	}
	if ((mask & PIN_GPIO) && (pin->enabled & PIN_GPIO)) {
		HAL_GPIO_DeInit(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio));
		pinDisable(pin, PIN_GPIO);
	}
}
