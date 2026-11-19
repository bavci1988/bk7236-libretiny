#include <libretiny.h>
#include <sdk_private.h>

lt_flash_id_t lt_flash_get_id() {
	uint32_t data	 = bk_flash_get_id();
	lt_flash_id_t id = {
		.manufacturer_id = (data >> 16) & 0xFF,
		.chip_id		 = (data >> 8) & 0xFF,
		.chip_size_id	 = (data >> 0) & 0xFF,
	};
	return id;
}
