#include <libretiny.h>
#include <sdk_private.h>

#include <soc/soc.h>

// SYS register 0x1: chip ID in the upper half (0x7236 on BK7236), version in the lower half
#define SYS_VERSION_ID (SOC_SYS_REG_BASE + (0x1 << 2))

lt_cpu_model_t lt_cpu_get_model() {
	uint8_t chipId = (REG_READ(SYS_VERSION_ID) >> 16) & 0xFF;
	return CPU_MODEL_ENUM(FAMILY, chipId);
}

const char *lt_cpu_get_core_type() {
	return "ARM Cortex-M33 (ARMv8-M)";
}
