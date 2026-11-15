#include <libretiny.h>
#include <sdk_private.h>

lt_cpu_model_t lt_cpu_get_model() {
	// TODO: the chip ID register value of BK7236 is not verified yet
	return CPU_MODEL_ENUM(FAMILY, 0x00);
}

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M33 (ARMv8-M)";
}
