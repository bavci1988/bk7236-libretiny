#include <ArduinoPrivate.h>

#include <driver/gpio.h>
#include <driver/pwm.h>
#include <driver/timer.h>
#include <os/os.h>
#include <soc/soc.h>

#include <FreeRTOS.h>
#include <timers.h>

// TIMER0 is reserved by ARMINO as a free-running 26 MHz counter (CONFIG_TIMER_US)
#define TIMER0_TICKS_PER_US (CONFIG_XTAL_FREQ / 1000 / 1000)
// the counter overflows every 2^32 / 26 MHz = 165 s; extend it at least once per minute
#define TIMER0_EXTEND_MS	60000

extern void bk_timer_delay_us(uint32_t us);
extern bk_err_t gpio_dev_unmap(gpio_id_t gpio_id);

static uint64_t timer0Total	   = 0;
static uint32_t timer0Last	   = 0;
static TickType_t tickLast	   = 0;
static TimerHandle_t timer0Ext = NULL;

// latch and read the TIMER0 counter, like ARMINO's timer_hal_get_timer0_cnt()
// (bk_timer_get_cnt() adds PM checks to every call)
static inline uint32_t timer0Read() {
	volatile uint32_t *readCtl	 = (volatile uint32_t *)(SOC_TIMER0_REG_BASE + (8 << 2));
	volatile uint32_t *readValue = (volatile uint32_t *)(SOC_TIMER0_REG_BASE + (9 << 2));
	// bits 2-3: timer index (0), bit 0: start read
	*readCtl = (*readCtl & ~(0b11 << 2)) | 1;
	while (*readCtl & 1) {}
	return *readValue;
}

static uint64_t timer0Ticks() {
	// usable from ISRs as well
	uint32_t irq	= rtos_disable_int();
	uint32_t now	= timer0Read();
	TickType_t tick = xPortIsInsideInterrupt() ? xTaskGetTickCountFromISR() : xTaskGetTickCount();
	uint32_t delta	= now - timer0Last;
	// TIMER0 stops while the chip is in low-voltage sleep (26 MHz crystal halted); the FreeRTOS tick count
	// is corrected from the AON RTC after sleeping, so use it when it shows more elapsed time than TIMER0
	uint64_t tickUs = (uint64_t)(TickType_t)(tick - tickLast) * portTICK_PERIOD_MS * 1000;
	if (tickUs > (uint64_t)delta / TIMER0_TICKS_PER_US + 2 * portTICK_PERIOD_MS * 1000)
		timer0Total += tickUs * TIMER0_TICKS_PER_US;
	else
		timer0Total += delta;
	timer0Last	   = now;
	tickLast	   = tick;
	uint64_t ticks = timer0Total;
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
		pinKeepLevel(pin, data, PIN_KEEP_NONE);
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
		if (data->irqWake) {
			// this also disables the input
			bk_gpio_unregister_wakeup_source(pin->gpio);
			if ((pin->enabled & PIN_GPIO) && pinIsInput(pin, data))
				bk_gpio_enable_input(pin->gpio);
		}
		data->irqWake = false;
		pinDisable(pin, PIN_IRQ);
	}
	if ((mask & PIN_PWM) && (pin->enabled & PIN_PWM)) {
		pinPwmStop(data);
		gpio_dev_unmap(pin->gpio);
		pinDisable(pin, PIN_PWM);
	}
}
