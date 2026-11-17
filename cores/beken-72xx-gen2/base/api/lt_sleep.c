#include <libretiny.h>
#include <sdk_private.h>

#include <modules/pm.h>

bool lt_light_sleep_allow(bool allow) {
	// ARMINO enters its low-voltage sleep from the FreeRTOS idle task only while every module votes for it;
	// the APP vote is the one held back after boot
	return bk_pm_module_vote_sleep_ctrl(PM_SLEEP_MODULE_NAME_APP, allow ? 1 : 0, 0) == BK_OK;
}
