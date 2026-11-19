#include <libretiny.h>
#include <sdk_private.h>

// The bootloader (Beken ARMINO "normal_bootloader", also used by Tuya) applies Tuya's
// diff2ya OTA format: a compressed full app image ("LZCP") in the 'download' partition,
// requested by the "manage info" block in the 'diff2ya' partition. Both are written by
// uf2ota; the bootloader verifies the image CRC32 before erasing the app.

#define DIFF2YA_MAGIC 0x004D4D4D // "MMM\0", little-endian

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	if (index != 0)
		return false;
	// check download image header
	uint32_t magic;
	lt_flash_read(FLASH_DOWNLOAD_OFFSET, (uint8_t *)&magic, 4);
	return magic == DIFF2YA_MAGIC;
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

bool lt_ota_switch(bool revert) {
	if (!lt_ota_is_valid(0))
		// no valid "download" image
		// - return false when trying to activate
		// - return true when trying to revert
		return revert;
	if (revert) {
		// remove the update request and the "download" image
#ifdef FLASH_DIFF2YA_OFFSET
		if (!lt_flash_erase_block(FLASH_DIFF2YA_OFFSET))
			return false;
#endif
		return lt_flash_erase_block(FLASH_DOWNLOAD_OFFSET);
	}
	return true;
}
