#include <libretiny.h>
#include <sdk_private.h>

#include <fal.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)

static int init() {
	bk_flash_set_protect_type(FLASH_PROTECT_NONE);
	return 0;
}

static int read(long offset, uint8_t *buf, size_t size) {
	if (bk_flash_read_bytes(offset, buf, size) != BK_OK)
		return -1;
	return size;
}

static int write(long offset, const uint8_t *buf, size_t size) {
	if (bk_flash_write_bytes(offset, buf, size) != BK_OK)
		return -1;
	return size;
}

static int erase(long offset, size_t size) {
	offset &= ~(FLASH_ERASE_MIN_SIZE - 1);
	size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;
	for (uint16_t i = 0; i < size; i++) {
		if (bk_flash_erase_sector(offset + i * FLASH_ERASE_MIN_SIZE) != BK_OK)
			return -1;
	}
	return size * FLASH_ERASE_MIN_SIZE;
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {init, read, write, erase},
	.write_gran = 1,
};
