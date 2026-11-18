#include <ArduinoPrivate.h>

#include <driver/gpio.h>

// release the pin from its default peripheral function (gpio_map.h)
extern bk_err_t gpio_dev_unmap(gpio_id_t gpio_id);

// In light sleep ARMINO lets every pin float, except wake-up sources and at most
// CONFIG_GPIO_DYNAMIC_KEEP_STATUS_MAX_CNT (4) "keep status" pins. Register driven outputs there,
// so that they hold their level while the CPU sleeps.
void pinKeepLevel(PinInfo *pin, PinData *data, int8_t level) {
	if (data->keepLevel == level || (data->keepLevel == PIN_KEEP_FULL && level >= 0))
		return;
	if (level < 0) {
		if (data->keepLevel >= 0)
			bk_gpio_unregister_lowpower_keep_status(pin->gpio);
		data->keepLevel = PIN_KEEP_NONE;
		return;
	}
	// ARMINO encodes the output level as the pull direction
	gpio_config_t cfg = {
		.io_mode   = GPIO_OUTPUT_ENABLE,
		.pull_mode = level ? GPIO_PULL_UP_EN : GPIO_PULL_DOWN_EN,
		.func_mode = GPIO_SECOND_FUNC_DISABLE,
	};
	if (bk_gpio_register_lowpower_keep_status(pin->gpio, &cfg) != BK_OK) {
		LT_W("P%u: no free slot to hold the output level during light sleep", pin->gpio);
		data->keepLevel = PIN_KEEP_FULL;
		return;
	}
	data->keepLevel = level;
}

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
	if (!(pin->enabled & (PIN_GPIO | PIN_IRQ)))
		gpio_dev_unmap(pin->gpio);
	if (bk_gpio_set_config(pin->gpio, &cfg) != BK_OK)
		return;
	if (!pinEnabled(pin, PIN_GPIO))
		data->keepLevel = PIN_KEEP_NONE;
	pinEnable(pin, PIN_GPIO);
	data->gpioMode = pinMode;
	// a new output starts low
	pinKeepLevel(pin, data, pinMode == OUTPUT ? 0 : PIN_KEEP_NONE);
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
		pinKeepLevel(pin, data, status ? PIN_KEEP_NONE : 0);
		return;
	}
	if (status)
		bk_gpio_set_output_high(pin->gpio);
	else
		bk_gpio_set_output_low(pin->gpio);
	pinKeepLevel(pin, data, status ? 1 : 0);
}

PinStatus digitalRead(pin_size_t pinNumber) {
	pinCheckGetData(pinNumber, PIN_GPIO, LOW);
	pinSetInputMode(pin, data, pinNumber);
	return bk_gpio_get_input(pin->gpio) ? HIGH : LOW;
}
