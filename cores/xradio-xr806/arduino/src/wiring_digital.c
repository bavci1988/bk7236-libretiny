#include <ArduinoPrivate.h>

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	pinCheckGetData(pinNumber, PIN_GPIO, );

	if (pinEnabled(pin, PIN_GPIO) && data->gpioMode == pinMode)
		return;

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);

	GPIO_InitParam param = {
		.mode	 = GPIOx_Pn_F0_INPUT,
		.driving = GPIO_DRIVING_LEVEL_1,
		.pull	 = GPIO_PULL_NONE,
	};
	switch (pinMode) {
		case INPUT:
			break;
		case INPUT_PULLDOWN:
			param.pull = GPIO_PULL_DOWN;
			break;
		case INPUT_PULLUP:
			param.pull = GPIO_PULL_UP;
			break;
		case OUTPUT:
			param.mode = GPIOx_Pn_F1_OUTPUT;
			break;
		case OUTPUT_OPENDRAIN:
			// no open-drain output; emulated by switching between input and output low
			break;
		default:
			return;
	}
	// an input with an interrupt keeps the EINT function (attachInterrupt())
	if (param.mode == GPIOx_Pn_F0_INPUT && pinEnabled(pin, PIN_IRQ))
		param.mode = GPIOx_Pn_F6_EINT;
	pinEnable(pin, PIN_GPIO);
	data->gpioMode = pinMode;
	HAL_GPIO_Init(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	pinCheckGetData(pinNumber, PIN_GPIO, );
	pinSetOutputPull(pin, data, pinNumber, status);
	if (data->gpioMode == OUTPUT_OPENDRAIN) {
		GPIO_InitParam param = {
			.mode	 = status ? GPIOx_Pn_F0_INPUT : GPIOx_Pn_F1_OUTPUT,
			.driving = GPIO_DRIVING_LEVEL_1,
			.pull	 = GPIO_PULL_NONE,
		};
		HAL_GPIO_Init(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
		if (status)
			return;
	}
	HAL_GPIO_WritePin(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), status ? GPIO_PIN_HIGH : GPIO_PIN_LOW);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
	pinSetInputMode(pin, data, pinNumber);
	return HAL_GPIO_ReadPin(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio)) == GPIO_PIN_HIGH;
}
