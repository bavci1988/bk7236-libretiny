/* Copyright (c) Kuba Szczodrzyński 2022-07-31. */

#include <ArduinoPrivate.h>

#include <driver/gpio.h>

extern bk_err_t gpio_dev_unmap(gpio_id_t gpio_id);

static void irqHandler(gpio_id_t gpio) {
	PinInfo *pin = pinByGpio(gpio);
	if (pin == NULL)
		return;
	PinData *data = pinData(pin);
	if (!data->irqHandler)
		return;
	if (!data->irqParam)
		((voidFuncPtr)data->irqHandler)();
	else
		((voidFuncPtrParam)data->irqHandler)(data->irqParam);
	if (data->irqChange) {
		// the hardware has no "both edges" mode - flip the edge
		gpio_int_type_t type;
		if (data->irqMode == RISING) {
			data->irqMode = FALLING;
			type		  = GPIO_INT_TYPE_FALLING_EDGE;
		} else {
			data->irqMode = RISING;
			type		  = GPIO_INT_TYPE_RISING_EDGE;
		}
		bk_gpio_set_interrupt_type(gpio, type);
	}
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	pinCheckGetData(interruptNumber, PIN_IRQ, );

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_PWM);

	data->irqHandler = callback;
	data->irqParam	 = param;

	if (pinEnabled(pin, PIN_IRQ) && data->irqMode == mode)
		return;

	if (!(pin->enabled & (PIN_GPIO | PIN_IRQ))) {
		// configure as input, releasing the default peripheral function
		gpio_config_t cfg = {
			.io_mode   = GPIO_INPUT_ENABLE,
			.pull_mode = GPIO_PULL_DISABLE,
			.func_mode = GPIO_SECOND_FUNC_DISABLE,
		};
		gpio_dev_unmap(pin->gpio);
		bk_gpio_set_config(pin->gpio, &cfg);
	}

	gpio_int_type_t type;
	bool change = false;
	switch (mode) {
		case LOW:
			type = GPIO_INT_TYPE_LOW_LEVEL;
			break;
		case HIGH:
			type = GPIO_INT_TYPE_HIGH_LEVEL;
			break;
		case FALLING:
			type = GPIO_INT_TYPE_FALLING_EDGE;
			break;
		case RISING:
			type = GPIO_INT_TYPE_RISING_EDGE;
			break;
		case CHANGE:
			if (bk_gpio_get_input(pin->gpio)) {
				type = GPIO_INT_TYPE_FALLING_EDGE;
				mode = FALLING;
			} else {
				type = GPIO_INT_TYPE_RISING_EDGE;
				mode = RISING;
			}
			change = true;
			break;
		default:
			return;
	}
	pinEnable(pin, PIN_IRQ);
	data->irqMode	= mode;
	data->irqChange = change;

	bk_gpio_disable_interrupt(pin->gpio);
	bk_gpio_register_isr(pin->gpio, irqHandler);
	bk_gpio_set_interrupt_type(pin->gpio, type);
	bk_gpio_enable_interrupt(pin->gpio);
}

void detachInterrupt(pin_size_t interruptNumber) {
	pinModeRemove(interruptNumber, PIN_IRQ);
}
