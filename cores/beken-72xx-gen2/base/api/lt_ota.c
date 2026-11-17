#include <libretiny.h>
#include <sdk_private.h>

// OTA is not supported yet: the update format expected by the bootloader
// (OTA partition at FLASH_OTA_OFFSET) still has to be implemented.
// The UF2 image only contains a flasher target, so no device partition is written.

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	return false;
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

bool lt_ota_switch(bool revert) {
	// nothing to activate; reverting is always possible
	return revert;
}
