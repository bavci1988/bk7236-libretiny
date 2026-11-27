#include <ArduinoPrivate.h>

#include "driver/chip/hal_adc.h"
#include "driver/chip/hal_pwm.h"
#include "driver/hal_board.h"

// internal reference, input range 0..2.5 V (ADC_VREF_MODE_1)
#define ADC_MAX_VOLTAGE 2500

uint16_t analogReadVoltage(pin_size_t pinNumber) {
	pinCheckGetInfo(pinNumber, PIN_ADC, 0);
	// ADC_CH0..6 are PA10..PA16
	if (pin->gpio < 10 || pin->gpio > 16)
		return 0;
	ADC_Channel channel = (ADC_Channel)(pin->gpio - 10);

	ADC_InitParam param = {
		.work_clk  = ADC_WORKCLK_HFCLK,
		.freq	   = 1000000,
		.delay	   = 0,
		.vref_mode = ADC_VREF_MODE_1,
		.mode	   = ADC_CONTI_CONV,
	};
	uint32_t value = 0;
	if (HAL_ADC_Init(&param) != HAL_OK)
		return 0;
	HAL_Status status = HAL_ADC_Conv_Polling(channel, &value, 100);
	HAL_ADC_DeInit();
	if (status != HAL_OK)
		return 0;
	return (uint16_t)(ADC_MAX_VOLTAGE * value / 4095);
}

uint16_t analogReadMaxVoltage(pin_size_t pinNumber) {
	return ADC_MAX_VOLTAGE;
}

// PWM channels and their pins (pin function numbers of hal_gpio.h, chip architecture 3)
typedef struct {
	uint8_t gpio;
	uint8_t channel;
	uint8_t function;
} PwmPin;

static const PwmPin pwm_pins[] = {
	{.gpio = PA_19, .channel = 0, .function = 4},
	  {.gpio = PB_4,	 .channel = 0, .function = 3},
	{.gpio = PA_20, .channel = 1, .function = 4},
	  {.gpio = PB_5,	 .channel = 1, .function = 3},
	{.gpio = PA_21, .channel = 2, .function = 4},
	  {.gpio = PB_6,	 .channel = 2, .function = 3},
	{.gpio = PA_22, .channel = 3, .function = 4},
	  {.gpio = PB_7,	 .channel = 3, .function = 3},
	{.gpio = PA_12, .channel = 4, .function = 3},
	  {.gpio = PB_0,	 .channel = 4, .function = 4},
	{.gpio = PA_13, .channel = 5, .function = 3},
	  {.gpio = PB_1,	 .channel = 5, .function = 4},
	{.gpio = PA_0,  .channel = 5, .function = 9},
	  {.gpio = PB_14, .channel = 5, .function = 9},
	{.gpio = PA_14, .channel = 6, .function = 3},
	  {.gpio = PB_2,	 .channel = 6, .function = 4},
	{.gpio = PA_1,  .channel = 6, .function = 9},
	  {.gpio = PB_15, .channel = 6, .function = 9},
	{.gpio = PB_3,  .channel = 7, .function = 4},
};

// the pin of each channel; the SDK board configuration has one fixed pin per channel
static const PwmPin *pwm_channel_pin[PWM_CH_NUM];
static HAL_BoardIoctlCb sdk_board_ioctl;

static HAL_Status pwm_board_ioctl(HAL_BoardIoctlReq req, uint32_t param0, uint32_t param1) {
	uint32_t channel = HAL_DEV_MINOR((HAL_Dev_t)param0);
	if (HAL_DEV_MAJOR((HAL_Dev_t)param0) != HAL_DEV_MAJOR_PWM || channel >= PWM_CH_NUM || !pwm_channel_pin[channel])
		return sdk_board_ioctl(req, param0, param1);
	const PwmPin *pwm	 = pwm_channel_pin[channel];
	GPIO_InitParam param = {
		.mode	 = (GPIO_WorkMode)pwm->function,
		.driving = GPIO_DRIVING_LEVEL_1,
		.pull	 = GPIO_PULL_NONE,
	};
	switch (req) {
		case HAL_BIR_PINMUX_INIT:
		case HAL_BIR_PINMUX_RESUME:
			HAL_GPIO_Init(GPIO_GET_PORT(pwm->gpio), GPIO_GET_PIN(pwm->gpio), &param);
			return HAL_OK;
		case HAL_BIR_PINMUX_DEINIT:
		case HAL_BIR_PINMUX_SUSPEND:
			HAL_GPIO_DeInit(GPIO_GET_PORT(pwm->gpio), GPIO_GET_PIN(pwm->gpio));
			return HAL_OK;
		default:
			return sdk_board_ioctl(req, param0, param1);
	}
}

static const PwmPin *pwm_find(uint8_t gpio) {
	for (size_t i = 0; i < sizeof(pwm_pins) / sizeof(pwm_pins[0]); i++) {
		if (pwm_pins[i].gpio == gpio)
			return &pwm_pins[i];
	}
	return NULL;
}

void pwm_stop(PinInfo *pin) {
	const PwmPin *pwm = pwm_find(pin->gpio);
	if (pwm == NULL || pwm_channel_pin[pwm->channel] != pwm)
		return;
	HAL_PWM_EnableCh((PWM_CH_ID)pwm->channel, PWM_CYCLE_MODE, 0);
	HAL_PWM_ChDeinit((PWM_CH_ID)pwm->channel);
	pwm_channel_pin[pwm->channel] = NULL;
	// disabling a channel gates the clock of its group, which the other channel shares
	uint8_t sibling = pwm->channel ^ 1;
	if (pwm_channel_pin[sibling])
		HAL_PWM_EnableCh((PWM_CH_ID)sibling, PWM_CYCLE_MODE, 1);
}

void analogWrite(pin_size_t pinNumber, int value) {
	pinCheckGetData(pinNumber, PIN_PWM, );
	const PwmPin *pwm = pwm_find(pin->gpio);
	// one pin per channel
	if (pwm == NULL || (pwm_channel_pin[pwm->channel] && pwm_channel_pin[pwm->channel] != pwm))
		return;
	PWM_CH_ID channel = (PWM_CH_ID)pwm->channel;

	// GPIO can't be used together with PWM
	pinRemoveMode(pin, PIN_GPIO | PIN_IRQ);

	if (!pinEnabled(pin, PIN_PWM) || data->pwmPeriod != _analogWritePeriod) {
		if (sdk_board_ioctl == NULL)
			sdk_board_ioctl = HAL_BoardIoctlCbRegister(pwm_board_ioctl);
		pwm_channel_pin[channel] = pwm;
		PWM_ClkParam clock		 = {
				  .clk = PWM_CLK_HOSC,
				  .div = PWM_SRC_CLK_DIV_1,
		  };
		PWM_ChInitParam param = {
			.mode	  = PWM_CYCLE_MODE,
			.polarity = PWM_HIGHLEVE,
			.hz		  = 1000000 / MAX(_analogWritePeriod, 1),
		};
		int max = -1;
		if (HAL_PWM_GroupClkCfg((PWM_GROUP_ID)(channel / 2), &clock) == HAL_OK)
			max = HAL_PWM_ChInit(channel, &param);
		if (max <= 0) {
			pwm_stop(pin);
			pinDisable(pin, PIN_PWM);
			return;
		}
		data->pwmMax	= max;
		data->pwmPeriod = _analogWritePeriod;
		pinEnable(pin, PIN_PWM);
	}

	uint32_t duty = (uint64_t)value * data->pwmMax / ((1 << _analogWriteResolution) - 1);
	HAL_PWM_ChSetDutyRatio(channel, MIN(duty, data->pwmMax));
	HAL_PWM_EnableCh(channel, PWM_CYCLE_MODE, 1);
}
