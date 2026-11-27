#include <libretiny.h>
#include <sdk_private.h>

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M33 (ARMv8-M)";
}

uint32_t lt_cpu_get_freq() {
	return HAL_GetCPUClock();
}
