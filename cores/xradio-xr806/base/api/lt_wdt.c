#include <libretiny.h>
#include <sdk_private.h>

#include "driver/chip/hal_wdg.h"

// HAL_WDG_Init() registers the watchdog with PM, which stops it in standby; once is enough
static bool initialized = false;

bool lt_wdt_enable(uint32_t timeout) {
	// the hardware supports 0.5, 1..6, 8, 10, 12, 14 and 16 seconds
	static const uint16_t timeouts_ms[] = {500, 1000, 2000, 3000, 4000, 5000, 6000, 8000, 10000, 12000, 14000, 16000};
	uint32_t index						= 0;
	while (index < sizeof(timeouts_ms) / sizeof(timeouts_ms[0]) - 1 && timeouts_ms[index] < timeout)
		index++;
	WDG_InitParam param = {0};
	param.hw.event		= WDG_EVT_RESET;
	param.hw.timeout	= (WDG_Timeout)(index << WDG_TIMEOUT_SHIFT);
	param.hw.resetCycle = WDG_DEFAULT_RESET_CYCLE;
	if (initialized) {
		HAL_WDG_Stop();
		HAL_WDG_DeInit();
	}
	if (HAL_WDG_Init(&param) != HAL_OK)
		return false;
	initialized = true;
	HAL_WDG_Start();
	return true;
}

void lt_wdt_disable() {
	if (!initialized)
		return;
	HAL_WDG_Stop();
	HAL_WDG_DeInit();
	initialized = false;
}

void lt_wdt_feed() {
	HAL_WDG_Feed();
}
