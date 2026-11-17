#include <ArduinoPrivate.h>

#include <driver/gpio.h>
#include <driver/pwm.h>
#include <driver/timer.h>
#include <os/os.h>

#include <FreeRTOS.h>
#include <timers.h>

// TIMER0 is reserved by ARMINO as a free-running 26 MHz counter (CONFIG_TIMER_US)
#define TIMER0_TICKS_PER_US (CONFIG_XTAL_FREQ / 1000 / 1000)
// the counter overflows every 2^32 / 26 MHz = 165 s; extend it at least once per minute
#define TIMER0_EXTEND_MS	60000

extern void bk_timer_delay_us(uint32_t us);
extern bk_err_t gpio_dev_unmap(gpio_id_t gpio_id);

static uint32_t timer0Last	   = 0;
static uint64_t timer0High	   = 0;
static TimerHandle_t timer0Ext = NULL;

static uint64_t timer0Ticks() {
	// usable from ISRs as well
	uint32_t irq = rtos_disable_int();
	uint32_t now = bk_timer_get_cnt(TIMER_ID0);
	if (now < timer0Last)
		timer0High += 1ULL << 32;
	timer0Last	   = now;
	uint64_t ticks = timer0High | now;
	rtos_enable_int(irq);
	return ticks;
}

static void timer0Extend(TimerHandle_t timer) {
	(void)timer;
	timer0Ticks();
}

void lt_init_arduino() {
	timer0Ticks();
	timer0Ext = xTimerCreate("micros", pdMS_TO_TICKS(TIMER0_EXTEND_MS), pdTRUE, NULL, timer0Extend);
	if (timer0Ext)
		xTimerStart(timer0Ext, 0);
}

void delayMicroseconds(unsigned int us) {
	bk_timer_delay_us(us);
}

unsigned long millis() {
	return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

unsigned long micros() {
	return (unsigned long)(timer0Ticks() / TIMER0_TICKS_PER_US);
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
		// bk_gpio_unregister_isr() is declared, but not implemented in ARMINO
		bk_gpio_register_isr(pin->gpio, NULL);
		pinDisable(pin, PIN_IRQ);
	}
	if ((mask & PIN_PWM) && (pin->enabled & PIN_PWM)) {
		if (data->pwmState == LT_PWM_RUNNING)
			bk_pwm_stop(data->pwmChannel);
		bk_pwm_deinit(data->pwmChannel);
		data->pwmState = LT_PWM_STOPPED;
		gpio_dev_unmap(pin->gpio);
		pinDisable(pin, PIN_PWM);
	}
}
