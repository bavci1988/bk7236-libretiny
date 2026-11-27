#include <ArduinoPrivate.h>

// called by the HAL GPIO interrupt handler (in RAM, so it also runs during flash writes)
static void irq_handler(void *arg) {
	PinData *data = (PinData *)arg;
	if (!data->irqHandler)
		return;
	if (!data->irqParam)
		((voidFuncPtr)data->irqHandler)();
	else
		((voidFuncPtrParam)data->irqHandler)(data->irqParam);
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void *param) {
	pinCheckGetData(interruptNumber, PIN_IRQ, );

	data->irqHandler = callback;
	data->irqParam	 = param;

	if (pinEnabled(pin, PIN_IRQ) && data->irqMode == mode)
		return;

	GPIO_IrqEvent event;
	switch (mode) {
		case LOW:
			event = GPIO_IRQ_EVT_LOW_LEVEL;
			break;
		case HIGH:
			event = GPIO_IRQ_EVT_HIGH_LEVEL;
			break;
		case FALLING:
			event = GPIO_IRQ_EVT_FALLING_EDGE;
			break;
		case RISING:
			event = GPIO_IRQ_EVT_RISING_EDGE;
			break;
		case CHANGE:
			event = GPIO_IRQ_EVT_BOTH_EDGE;
			break;
		default:
			return;
	}
	GPIO_IrqParam irq = {
		.event	  = event,
		.callback = irq_handler,
		.arg	  = data,
	};
	if (pinEnabled(pin, PIN_IRQ)) {
		HAL_GPIO_DisableIRQ(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio));
	} else {
		// interrupts need the EINT pin function; the pull resistor and the input level stay
		GPIO_InitParam param;
		HAL_GPIO_GetConfig(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
		param.mode = GPIOx_Pn_F6_EINT;
		HAL_GPIO_Init(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &param);
	}
	if (HAL_GPIO_EnableIRQ(GPIO_GET_PORT(pin->gpio), GPIO_GET_PIN(pin->gpio), &irq) != HAL_OK)
		return;
	pinEnable(pin, PIN_IRQ);
	data->irqMode = mode;
}

void detachInterrupt(pin_size_t interruptNumber) {
	pinCheckGetData(interruptNumber, PIN_IRQ, );
	pinModeRemove(interruptNumber, PIN_IRQ);
}
