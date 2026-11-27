#include <libretiny.h>
#include <sdk_private.h>

#include "image/image.h"
#include "xz/xz.h"

// The OTA image is one AWIH APP section holding the XZ-compressed APP section chain.
// The UF2 writes it to the download partition (image sequence 1). The bootloader unpacks it
// over the running image (sequence 0) on the next boot, once the boot selection sector
// (OTA config, FDCM) says sequence 1 is verified. Nothing can be rolled back after that.

#define AWIH_HEADER_SIZE   64
#define AWIH_MAGIC		   0x48495741
#define AWIH_VERSION	   5
#define AWIH_ID_APP		   0xA5FE5A01
#define AWIH_COMPRESS	   (1u << 4)
#define AWIH_NO_NEXT	   0xFFFFFFFF
#define AWIH_MAX_SECTIONS  32
// project/bootloader/main.c: BL_DEC_IMG_DICT_MAX
#define BOOTLOADER_XZ_DICT (8 * 1024)

#define STAGED_SEQ	 1
#define XZ_IN_CHUNK	 512
#define XZ_OUT_CHUNK 1024

static uint32_t load_le32(const uint8_t *p) {
	return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static uint16_t sum16(const uint8_t *p, size_t n) {
	uint16_t sum = 0;
	for (size_t i = 0; i < n; i += 2)
		sum += p[i] | (i + 1 < n ? p[i + 1] << 8 : 0);
	return sum;
}

// The layout in the BOOT header (read by the ROM) must be the layout of the board.
static bool xr806_ota_layout_ok(void) {
	const image_ota_param_t *iop = image_get_ota_param();
	if (iop == NULL) {
		LT_EM(OTA, "ROM OTA layout unavailable");
		return false;
	}
	bool ok = iop->running_seq == 0 && iop->flash[0] == iop->flash[1] && iop->ota_flash == iop->flash[0] &&
			  iop->ota_addr == FLASH_OTA_CFG_OFFSET && iop->ota_size == FLASH_OTA_CFG_LENGTH &&
			  iop->bl_size == FLASH_APP_OFFSET && iop->addr[0] == FLASH_APP_OFFSET &&
			  iop->img_max_size * 1024 == FLASH_APP_LENGTH && iop->addr[1] == FLASH_DOWNLOAD_OFFSET &&
			  iop->img_xz_max_size * 1024 == FLASH_DOWNLOAD_LENGTH;
	if (!ok) {
		LT_EM(
			OTA,
			"BOOT header layout differs from the board: seq %u, app 0x%X+%uK, download 0x%X+%uK, FDCM 0x%X",
			iop->running_seq,
			iop->addr[0],
			iop->img_max_size,
			iop->addr[1],
			iop->img_xz_max_size,
			iop->ota_addr
		);
		return false;
	}
	// a wired flash leaves the FDCM empty, which boots sequence 0
	image_cfg_t cfg;
	if (image_get_cfg(&cfg) == 0 && (cfg.seq != 0 || cfg.state != IMAGE_STATE_VERIFIED)) {
		LT_EM(OTA, "Boot selection is not a verified sequence 0 (seq %u, state %u)", cfg.seq, cfg.state);
		return false;
	}
	return true;
}

// Size of the staged AWIH/XZ wrapper including its header, or 0 if it is not valid.
static uint32_t xr806_ota_staged_size(void) {
	uint8_t header[AWIH_HEADER_SIZE + 6];
	if (flash_read(PRJCONF_IMG_FLASH, FLASH_DOWNLOAD_OFFSET, header, sizeof(header)) != sizeof(header))
		return 0;
	static const uint8_t xz_magic[6] = {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00};
	uint32_t body					 = load_le32(header + 12);
	uint32_t flags					 = load_le32(header + 28);
	if (load_le32(header) != AWIH_MAGIC || load_le32(header + 4) != AWIH_VERSION ||
		sum16(header, AWIH_HEADER_SIZE) != 0xFFFF || load_le32(header + 36) != AWIH_ID_APP ||
		(flags & AWIH_COMPRESS) == 0 || (flags & ((1u << 2) | (1u << 3))) != 0 ||
		load_le32(header + 32) != AWIH_NO_NEXT || body < 6 || load_le32(header + 24) != body ||
		body > FLASH_DOWNLOAD_LENGTH - AWIH_HEADER_SIZE || memcmp(header + AWIH_HEADER_SIZE, xz_magic, 6) != 0)
		return 0;
	return body + AWIH_HEADER_SIZE;
}

// Checks the bytes that the bootloader will write into the app partition, as they come out of
// the decompressor. The bootloader erases the app partition first and does not stop on a decode
// error, so this must pass before the update is committed. Mirrors the ROM section walk: the
// chain starts with an APP section and follows next_addr (relative to the app partition).
typedef struct {
	uint8_t header[AWIH_HEADER_SIZE];
	uint32_t total;
	uint32_t next_start;
	uint32_t body_left;
	uint32_t body_end;
	uint32_t next;
	uint32_t sections;
	uint16_t body_sum;
	uint8_t header_len;
	uint8_t phase; // 0: gap, 1: header, 2: body, 3: done
	bool odd;
	bool failed;
} staged_check_t;

static void staged_check_header(staged_check_t *c, uint32_t pos) {
	const uint8_t *h = c->header;
	if (load_le32(h) != AWIH_MAGIC || load_le32(h + 4) != AWIH_VERSION || sum16(h, AWIH_HEADER_SIZE) != 0xFFFF ||
		(c->sections == 0 && load_le32(h + 36) != AWIH_ID_APP) || c->sections >= AWIH_MAX_SECTIONS) {
		c->failed = true;
		return;
	}
	c->sections++;
	c->body_sum	 = h[10] | (h[11] << 8);
	c->body_left = load_le32(h + 12);
	c->next		 = load_le32(h + 32);
	c->odd		 = false;
	if (c->body_left == 0 || c->body_left > FLASH_APP_LENGTH - pos - 1) {
		c->failed = true;
		return;
	}
	c->body_end = pos + 1 + c->body_left;
	c->phase	= 2;
}

static void staged_check_body_done(staged_check_t *c) {
	if (c->body_sum != 0xFFFF) {
		c->failed = true;
		return;
	}
	if (c->next == AWIH_NO_NEXT) {
		c->phase = 3;
		return;
	}
	if (c->next < FLASH_APP_OFFSET || c->next - FLASH_APP_OFFSET < c->body_end ||
		c->next - FLASH_APP_OFFSET >= FLASH_APP_LENGTH) {
		c->failed = true;
		return;
	}
	c->next_start = c->next - FLASH_APP_OFFSET;
	c->phase	  = 0;
}

static bool staged_check_add(staged_check_t *c, const uint8_t *data, size_t len) {
	if (c->failed || len > FLASH_APP_LENGTH - c->total)
		return !(c->failed = true);
	for (size_t i = 0; i < len && !c->failed; i++) {
		uint8_t b	 = data[i];
		uint32_t pos = c->total + i;
		switch (c->phase) {
			case 0:
				if (pos != c->next_start)
					break;
				c->phase	  = 1;
				c->header_len = 0;
				// fall through
			case 1:
				c->header[c->header_len++] = b;
				if (c->header_len == AWIH_HEADER_SIZE)
					staged_check_header(c, pos);
				break;
			case 2:
				c->body_sum += c->odd ? (b << 8) : b;
				c->odd = !c->odd;
				if (--c->body_left == 0)
					staged_check_body_done(c);
				break;
		}
	}
	c->total += len;
	return !c->failed;
}

// Decode the staged image with the ROM decoder and the dictionary limit of the bootloader.
static bool xr806_ota_staged_image_ok(uint32_t size) {
	xz_crc32_init();
	struct xz_dec *dec	  = xz_dec_init(XZ_PREALLOC, BOOTLOADER_XZ_DICT);
	uint8_t *in			  = malloc(XZ_IN_CHUNK + XZ_OUT_CHUNK);
	staged_check_t *check = calloc(1, sizeof(staged_check_t));
	bool valid			  = dec != NULL && in != NULL && check != NULL;
	if (!valid)
		LT_EM(OTA, "Not enough memory to check the staged image");

	uint8_t *out	= in + XZ_IN_CHUNK;
	uint32_t start	= FLASH_DOWNLOAD_OFFSET + AWIH_HEADER_SIZE;
	uint32_t length = size - AWIH_HEADER_SIZE;
	uint32_t offset = 0;
	struct xz_buf b = {
		.out	  = out,
		.out_size = XZ_OUT_CHUNK,
	};
	enum xz_ret ret = XZ_OK;
	while (valid && ret == XZ_OK) {
		if (b.in_pos == b.in_size && offset < length) {
			uint32_t count = MIN(XZ_IN_CHUNK, length - offset);
			if (flash_read(PRJCONF_IMG_FLASH, start + offset, in, count) != count) {
				valid = false;
				break;
			}
			offset += count;
			b.in	  = in;
			b.in_pos  = 0;
			b.in_size = count;
		}
		// a truncated stream ends with XZ_BUF_ERROR on the second call without progress
		ret = xz_dec_run(dec, &b);
		if (b.out_pos != 0) {
			valid	  = staged_check_add(check, out, b.out_pos);
			b.out_pos = 0;
		}
		lt_wdt_feed();
	}
	valid = valid && ret == XZ_STREAM_END && offset == length && b.in_pos == b.in_size && check->phase == 3;
	if (valid)
		LT_IM(OTA, "Staged image decodes to %u bytes in %u sections", check->total, check->sections);
	else if (check != NULL)
		LT_EM(OTA, "Staged image check failed (xz=%d, in=%u/%u, out=%u)", ret, offset, length, check->total);

	if (dec != NULL)
		xz_dec_end(dec);
	free(in);
	free(check);
	return valid;
}

// The flash port (fal_flash_xr806_port.c) writes the download partition only while this is set.
// With another layout in the BOOT header, the download partition could overlap the boot
// selection sector; after a commit, the staged image must stay as it is until the reboot.
static bool download_writable = false;
static bool committed		  = false;

bool xr806_ota_download_writable(void) {
	return download_writable;
}

void lt_ota_set_write_protect(uf2_ota_t *uf2) {
	download_writable = !committed && xr806_ota_layout_ok();
}

lt_ota_type_t lt_ota_get_type() {
	return OTA_TYPE_SINGLE;
}

bool lt_ota_is_valid(uint8_t index) {
	uint32_t size = xr806_ota_staged_size();
	return size != 0 && xr806_ota_staged_image_ok(size);
}

uint8_t lt_ota_dual_get_current() {
	return 0;
}

uint8_t lt_ota_dual_get_stored() {
	return 0;
}

bool lt_ota_switch(bool revert) {
	if (revert)
		// The bootloader installs only a committed image, so before the commit a staged image is
		// already void. After it, the running image gets overwritten; there is nothing to go back to.
		return !committed;
	download_writable = false;
	if (committed || !xr806_ota_layout_ok())
		return false;
	uint32_t size = xr806_ota_staged_size();
	if (size == 0) {
		LT_EM(OTA, "Wrong OTA image: not a single AWIH/XZ APP section");
		return false;
	}
	// the same section check as ota_push_finish() of the SDK
	if (image_check_sections(STAGED_SEQ) == IMAGE_INVALID) {
		LT_EM(OTA, "Staged image sections are invalid");
		return false;
	}
	if (!xr806_ota_staged_image_ok(size))
		return false;

	// the commit: the next boot unpacks sequence 1 into sequence 0
	image_cfg_t cfg = {
		.seq   = STAGED_SEQ,
		.state = IMAGE_STATE_VERIFIED,
	};
	if (image_set_cfg(&cfg) != 0) {
		LT_EM(OTA, "Writing the boot selection failed");
		return false;
	}
	committed = true;
	image_cfg_t confirmed;
	if (image_get_cfg(&confirmed) != 0 || confirmed.seq != STAGED_SEQ || confirmed.state != IMAGE_STATE_VERIFIED) {
		LT_EM(OTA, "Boot selection reads back wrong; next boot state uncertain");
		return false;
	}
	LT_IM(OTA, "Update committed; the bootloader installs it on the next boot");
	return true;
}
