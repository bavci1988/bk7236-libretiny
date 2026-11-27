#include <libretiny.h>
#include <sdk_private.h>

#include <string.h>

#include "common/framework/sysinfo.h"
#include "driver/chip/hal_prcm.h"
#include "driver/chip/hal_wakeup.h"
#include "driver/chip/hal_wdg.h"
#include "driver/chip/system_chip.h"

void lt_get_device_mac(uint8_t *mac) {
	// sysinfo_init() (platform_init) derives it from the chip ID (PRJCONF_MAC_ADDR_SOURCE)
	struct sysinfo *sysinfo = sysinfo_get();
	if (sysinfo != NULL)
		memcpy(mac, sysinfo->mac_addr, 6);
	else
		memset(mac, 0, 6);
}

void lt_reboot() {
	HAL_WDG_Reboot();
	while (1) {}
}

static lt_reboot_reason_t boot_reason = REBOOT_REASON_UNKNOWN;
// the RTC keeps counting through a reset and a wake-up from hibernation
static uint64_t boot_rtc_us;

uint64_t xr806_uptime_us(void) {
	return HAL_RTC_GetFreeRunTime() - boot_rtc_us;
}

// Called by _start() (port/xr806_main.c). SystemInit() has already read and cleared the reset source;
// a standby changes the startup state and the wake-up events later.
void xr806_save_boot_reason(void) {
	boot_rtc_us = HAL_RTC_GetFreeRunTime();
	switch (SysGetStartupState()) {
		case SYS_POWERON:
			boot_reason = REBOOT_REASON_POWER;
			break;
		case SYS_WATCHDOG_CHIP_RST:
		case SYS_WATCHDOG_CPU_RST:
			boot_reason = REBOOT_REASON_WATCHDOG;
			break;
		case SYS_REBOOT:
		case SYS_CPU_RST:
		case SYS_NVIC_RST:
			boot_reason = REBOOT_REASON_SOFTWARE;
			break;
		case SYS_HIBERNATION:
			// pm_init() (in SystemInit()) saved the wake-up events, then cleared their registers
			if (HAL_Wakeup_GetEvent() & PM_WAKEUP_SRC_WKTIMER)
				boot_reason = REBOOT_REASON_SLEEP_RTC;
			else if (HAL_Wakeup_GetEvent() & ((1u << WAKEUP_IO_MAX) - 1))
				boot_reason = REBOOT_REASON_SLEEP_GPIO;
			break;
		default:
			break;
	}
}

lt_reboot_reason_t lt_get_reboot_reason() {
	return boot_reason;
}
