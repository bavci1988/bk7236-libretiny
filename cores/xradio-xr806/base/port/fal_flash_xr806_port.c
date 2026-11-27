#include <libretiny.h>
#include <sdk_private.h>

#include <fal.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)

// api/lt_ota.c
extern bool xr806_ota_download_writable(void);

static bool is_allowed(long offset, size_t size) {
	bool in_download = offset < FLASH_DOWNLOAD_OFFSET + FLASH_DOWNLOAD_LENGTH && offset + size > FLASH_DOWNLOAD_OFFSET;
	return !in_download || xr806_ota_download_writable();
}

static int read(long offset, uint8_t *buf, size_t size) {
	return flash_read(PRJCONF_IMG_FLASH, offset, buf, size);
}

static int write(long offset, const uint8_t *buf, size_t size) {
	if (!is_allowed(offset, size))
		return -1;
	return flash_write(PRJCONF_IMG_FLASH, offset, (void *)buf, size);
}

static int erase(long offset, size_t size) {
	offset &= ~(FLASH_ERASE_MIN_SIZE - 1);
	size = ((size - 1) / FLASH_ERASE_MIN_SIZE + 1) * FLASH_ERASE_MIN_SIZE;
	if (!is_allowed(offset, size))
		return -1;
	if (flash_erase(PRJCONF_IMG_FLASH, offset, size) != 0)
		return -1;
	return size;
}

const struct fal_flash_dev flash0 = {
	.name		= FAL_FLASH_DEV_NAME,
	.addr		= 0x0,
	.len		= FLASH_LENGTH,
	.blk_size	= FLASH_ERASE_MIN_SIZE,
	.ops		= {NULL, read, write, erase},
	.write_gran = 1,
};
