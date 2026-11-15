#include <ArduinoPrivate.h>

#include <driver/gpio.h>

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
	pinCheckGetData(pinNumber, PIN_GPIO, );

	if (pinEnabled(pin, PIN_GPIO) && data->gpioMode == pinMode)
		return;

	gpio_config_t cfg = {
		.io_mode   = GPIO_INPUT_ENABLE,
		.pull_mode = GPIO_PULL_DISABLE,
		.func_mode = GPIO_SECOND_FUNC_DISABLE,
	};
	switch (pinMode) {
		case INPUT:
			break;
		case INPUT_PULLUP:
			cfg.pull_mode = GPIO_PULL_UP_EN;
			break;
		case INPUT_PULLDOWN:
			cfg.pull_mode = GPIO_PULL_DOWN_EN;
			break;
		case OUTPUT:
			cfg.io_mode = GPIO_OUTPUT_ENABLE;
			break;
		case OUTPUT_OPENDRAIN:
			// no open-drain output; start released (high impedance)
			cfg.io_mode = GPIO_IO_DISABLE;
			break;
		default:
			return;
	}
	if (bk_gpio_set_config(pin->gpio, &cfg) != BK_OK)
		return;
	pinEnable(pin, PIN_GPIO);
	data->gpioMode = pinMode;
}

void digitalWrite(pin_size_t pinNumber, PinStatus status) {
	pinCheckGetData(pinNumber, PIN_GPIO, );
	pinSetOutputPull(pin, data, pinNumber, status);
	if (data->gpioMode == OUTPUT_OPENDRAIN) {
		// emulate open-drain: drive low, release on high
		if (status) {
			bk_gpio_disable_output(pin->gpio);
		} else {
			bk_gpio_enable_output(pin->gpio);
			bk_gpio_set_output_low(pin->gpio);
		}
		return;
	}
	if (status)
		bk_gpio_set_output_high(pin->gpio);
	else
		bk_gpio_set_output_low(pin->gpio);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
	pinSetInputMode(pin, data, pinNumber);
	return bk_gpio_get_input(pin->gpio) ? HIGH : LOW;
}
