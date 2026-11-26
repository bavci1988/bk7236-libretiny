/* Copyright (c) Kuba Szczodrzyński 2023-02-27. */

#include <libretiny.h>
#include <sdk_private.h>

#include <driver/wdt.h>

bool lt_wdt_enable(uint32_t timeout) {
	return bk_wdt_start(timeout) == BK_OK;
}

void lt_wdt_disable() {
	bk_wdt_stop();
}

void lt_wdt_feed() {
	bk_wdt_feed();
#if CONFIG_TASK_WDT
	// ARMINO's task watchdog asserts when the idle task has not run for CONFIG_TASK_WDT_PERIOD_MS;
	// a long flash write (e.g. an OTA update) keeps it from running, so feed it like ARMINO's own OTA driver does
	bk_task_wdt_feed();
#endif
}
