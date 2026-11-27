#include <libretiny.h>
#include <sdk_private.h>

#include "driver/chip/flashchip/flash_chip.h"

lt_flash_id_t lt_flash_get_id() {
	lt_flash_id_t id	 = {0};
	uint32_t jedec		 = 0;
	struct FlashDev *dev = getFlashDev(PRJCONF_IMG_FLASH);
	if (dev != NULL && dev->chip != NULL && dev->chip->jedecID(dev->chip, &jedec) == 0) {
		id.manufacturer_id = (uint8_t)(jedec >> 0);
		id.chip_id		   = (uint8_t)(jedec >> 8);
		id.chip_size_id	   = (uint8_t)(jedec >> 16);
	}
	return id;
}
