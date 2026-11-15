#include <ArduinoPrivate.h>

#include <driver/gpio.h>

void delayMicroseconds(unsigned int us) {
	// coarse busy-wait, TODO: use a hardware timer
	volatile uint32_t i, j;
	for (i = 0; i < us; i++) {
		for (j = 0; j < 12; j++) {}
	}
}

unsigned long millis() {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

unsigned long micros() {
	return millis() * 1000;
}

void pinRemoveMode(PinInfo *pin, uint32_t mask) {
	PinData *data = pinData(pin);
	if ((mask & PIN_GPIO) && (pin->enabled & PIN_GPIO)) {
		bk_gpio_disable_output(pin->gpio);
		bk_gpio_disable_input(pin->gpio);
		bk_gpio_disable_pull(pin->gpio);
		pinDisable(pin, PIN_GPIO);
	}
	if ((mask & PIN_IRQ) && (pin->enabled & PIN_IRQ)) {
		data->irqHandler = NULL;
		bk_gpio_disable_interrupt(pin->gpio);
		bk_gpio_unregister_isr(pin->gpio);
		pinDisable(pin, PIN_IRQ);
	}
}
