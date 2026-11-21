#include <ArduinoPrivate.h>

#include <driver/adc.h>
#include <driver/pwm.h>

extern float bk_adc_data_calculate(uint16_t adc_val, uint8_t adc_chan);
extern void sys_drv_set_ana_pwd_gadc_buf(uint32_t value);

// PWM clock: XTAL (selected by the PWM driver)
#define PWM_TICKS_PER_US (CONFIG_XTAL_FREQ / 1000 / 1000)

// channel numbers as in ARMINO's GPIO_PWM_MAP_TABLE
static const gpio_id_t pwmToGpio[] = {
	GPIO_18, // PWM0
	GPIO_19, // PWM1
	GPIO_8,	 // PWM2
	GPIO_9,	 // PWM3
	GPIO_24, // PWM4
	GPIO_25, // PWM5
	GPIO_32, // PWM6
	GPIO_33, // PWM7
	GPIO_34, // PWM8
	GPIO_35, // PWM9
	GPIO_36, // PWM10
	GPIO_37, // PWM11
};

// channel numbers as in ARMINO's ADC_DEV_MAP
static const struct {
	gpio_id_t gpio;
	adc_chan_t chan;
} adcToGpio[] = {
	{GPIO_25, ADC_1 },
	{GPIO_24, ADC_2 },
	{GPIO_23, ADC_3 },
	{GPIO_28, ADC_4 },
	{GPIO_22, ADC_5 },
	{GPIO_21, ADC_6 },
	{GPIO_8,	 ADC_10},
	{GPIO_0,	 ADC_12},
	{GPIO_1,	 ADC_13},
	{GPIO_12, ADC_14},
	{GPIO_13, ADC_15},
};

static int gpioToPwm(gpio_id_t gpio) {
	for (unsigned int i = 0; i < sizeof(pwmToGpio) / sizeof(pwmToGpio[0]); i++) {
		if (pwmToGpio[i] == gpio)
			return i;
	}
	return -1;
}

static int gpioToAdc(gpio_id_t gpio) {
	for (unsigned int i = 0; i < sizeof(adcToGpio) / sizeof(adcToGpio[0]); i++) {
		if (adcToGpio[i].gpio == gpio)
			return adcToGpio[i].chan;
	}
	return -1;
}

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	pinCheckGetInfo(pinNumber, PIN_ADC, 0);
	int chan = gpioToAdc(pin->gpio);
	if (chan < 0)
		return 0;

	// GPIO can't be used together with ADC
	pinRemoveMode(pin, PIN_GPIO | PIN_IRQ | PIN_PWM);

	// same sequence as ARMINO's "adc" CLI command
	uint16_t value = 0;
	if (bk_adc_acquire() != BK_OK)
		return 0;
	sys_drv_set_ana_pwd_gadc_buf(1);
	bk_err_t ret = bk_adc_init(chan);
	if (ret == BK_OK) {
		adc_config_t config	 = {0};
		config.chan			 = chan;
		config.adc_mode		 = ADC_CONTINUOUS_MODE;
		config.src_clk		 = 1;
		config.clk			 = 0x30e035;
		config.saturate_mode = 4;
		config.steady_ctrl	 = 7;
		config.adc_filter	 = 0;
		config.sample_rate	 = 0;
		ret					 = bk_adc_set_config(&config);
		if (ret == BK_OK)
			ret = bk_adc_enable_bypass_clalibration();
		if (ret == BK_OK)
			ret = bk_adc_start();
		if (ret == BK_OK)
			ret = bk_adc_read(&value, ADC_READ_SEMAPHORE_WAIT_TIME);
		bk_adc_stop();
	}
	sys_drv_set_ana_pwd_gadc_buf(0);
	bk_adc_deinit(chan);
	bk_adc_release();
	if (ret != BK_OK)
		return 0;

	// calibrated value in volts (ARMINO's 1 V / 2 V calibration points)
	float voltage = bk_adc_data_calculate(value, chan);
	return (uint16_t)(voltage * 1000.0f + 0.5f);
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
	// documented as 0-2.4 V, but the calibrated reading of a 3.3 V input is correct (3322 mV);
	// GPIO inputs can't exceed VDDIO anyway
	return 3300;
}

void analogWrite(pin_size_t pinNumber, int value) {
	pinCheckGetData(pinNumber, PIN_PWM, );
	int chan = gpioToPwm(pin->gpio);
	if (chan < 0)
		return;

	if (!pinEnabled(pin, PIN_PWM)) {
		// two channels share one timer and one compare value (0/1, 2/3, ...):
		// the second pin of a pair would repeat or change the output of the first
		PinInfo *other = pinByGpio(pwmToGpio[chan ^ 1]);
		if (other && pinEnabled(other, PIN_PWM)) {
			LT_W("P%u: PWM timer is used by P%u", pin->gpio, other->gpio);
			return;
		}
	}

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_GPIO | PIN_IRQ);

	uint32_t period = PWM_TICKS_PER_US * _analogWritePeriod;
	uint32_t max	= (1 << _analogWriteResolution) - 1;
	if (value < 0)
		value = 0;
	if ((uint32_t)value > max)
		value = max;
	uint32_t duty = (uint64_t)period * value / max;

	if (!pinEnabled(pin, PIN_PWM)) {
		pinEnable(pin, PIN_PWM);
		data->pwmChannel = chan;
		data->pwmState	 = LT_PWM_STOPPED;
	}

	if (data->pwmState == LT_PWM_STOPPED) {
		pwm_init_config_t config = {0};
		config.period_cycle		 = period;
		config.duty_cycle		 = duty;
		if (bk_pwm_init(chan, &config) != BK_OK || bk_pwm_start(chan) != BK_OK) {
			pinRemoveMode(pin, PIN_PWM);
			return;
		}
		data->pwmState = LT_PWM_RUNNING;
	} else {
		pwm_period_duty_config_t config = {0};
		config.period_cycle				= period;
		config.duty_cycle				= duty;
		bk_pwm_set_period_duty(chan, &config);
	}
}
