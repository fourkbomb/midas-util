/*
 * Copyright (C) 2018 Simon Shields <simon@lineageos.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bootimage.h"
#include "bootimg.h"
#include "config.h"
#include "util.h"

#define LVL_MASK ((1 << 11) - 1) /* bottom 11 bits */
#define MONTH_MASK ((1 << 4) - 1) /* bottom 4 bits */
#define VER_MASK ((1 << 7) - 1) /* bottom 7 bits */
static void show_android_info(struct boot_img_hdr *img) {
	uint32_t ver_a, ver_b, ver_c;
	uint32_t patch_y, patch_m;

	ver_a = img->os_version >> 11;
	patch_y = img->os_version & LVL_MASK;
	patch_m = patch_y & MONTH_MASK;

	patch_y = patch_y >> 4;
	patch_y += 2000;

	ver_c = ver_a & VER_MASK;
	ver_a = ver_a >> 7;
	ver_b = ver_a & VER_MASK;
	ver_a = ver_a >> 7;

	printf("Android %d.%d.%d, patch level %d-%02d.\n", ver_a, ver_b, ver_c,
			patch_y, patch_m);
}

static struct boot_img_hdr *load_and_validate_bootimg(struct global_config *cfg, off_t *len) {
	char *name;
	struct boot_img_hdr *header;
	uint8_t *buf;
	if (cfg->is_recovery) {
		if (cfg->recoveryImg == NULL)
			return NULL;
		name = cfg->recoveryImg;
	}

	if (!cfg->is_recovery) {
		if (cfg->bootImg == NULL)
			return NULL;
		name = cfg->bootImg;
	}

	buf = load_file(cfg, name, len);
	if (buf == NULL)
		return NULL;

	if (*len < sizeof(struct boot_img_hdr)) {
		fprintf(stderr, "Boot image file %s is too small! (Header size 0x%x, boot image length 0x%x)\n",
				name, (unsigned int)sizeof(struct boot_img_hdr), (unsigned int)*len);
		goto err;
	}

	header = (struct boot_img_hdr *)buf;

	if (memcmp(header->magic, BOOT_MAGIC, BOOT_MAGIC_SIZE) != 0) {
		fprintf(stderr, "Invalid boot image magic in %s, not loading.\n", name);
		goto err;
	}

	printf("Loaded Android boot image, size=0x%x, product_name=%s, cmdline=%s, ", (unsigned int)*len, header->name, header->cmdline);
	show_android_info(header);

	return header;
err:
	free(buf);
	return NULL;
}

void *load_android_zimage(struct global_config *cfg, off_t *len) {
	off_t bootimg_size;
	uint8_t *zimage;
	void *ret;
	struct boot_img_hdr *header = load_and_validate_bootimg(cfg, &bootimg_size);
	if (header == NULL)
		return NULL;

	/* zimage is stored directly after boot header, starting from second page. */
	zimage = (uint8_t *)header;
	zimage = &zimage[header->page_size];


	if ((header->kernel_size + header->page_size) > bootimg_size) {
		fprintf(stderr, "Kernel end address 0x%x is larger than boot image size 0x%lx!\n", header->kernel_size + header->page_size,
				bootimg_size);
		goto err;
	}

	ret = malloc(header->kernel_size);
	if (ret == NULL)
		goto err;
	memcpy(ret, zimage, header->kernel_size);
	*len = header->kernel_size;

	free(header);
	return ret;
err:
	free(header);
	return NULL;
}

void *load_android_initrd(struct global_config *cfg, off_t *len) {
	off_t bootimg_size;
	uint8_t *initrd;
	void *ret;
	uint32_t initrd_offset;
	struct boot_img_hdr *header = load_and_validate_bootimg(cfg, &bootimg_size);
	if (header == NULL)
		return NULL;

	/* initrd is stored directly after zimage, page-aligned. */
	initrd = (uint8_t *)header;
	initrd_offset = ALIGN(header->kernel_size, header->page_size) + header->page_size;
	initrd = &initrd[initrd_offset];


	if ((initrd_offset + header->ramdisk_size) > bootimg_size) {
		fprintf(stderr, "Ramdisk end address 0x%x is larger than boot image size 0x%lx!\n", initrd_offset + header->ramdisk_size,
				bootimg_size);
		goto err;
	}

	ret = malloc(header->ramdisk_size);
	if (ret == NULL)
		goto err;
	memcpy(ret, initrd, header->ramdisk_size);
	*len = header->ramdisk_size;

	free(header);
	return ret;
err:
	free(header);
	return NULL;
}

char *load_android_cmdline(struct global_config *cfg) {
	char *buf = calloc(sizeof(char), BOOT_ARGS_SIZE + BOOT_EXTRA_ARGS_SIZE);
	char *pos;
	off_t size;

	struct boot_img_hdr *header = load_and_validate_bootimg(cfg, &size);
	if (header == NULL)
		return NULL;

	memcpy(buf, header->cmdline, BOOT_ARGS_SIZE);
	pos = &buf[BOOT_ARGS_SIZE];
	memcpy(pos, header->extra_cmdline, BOOT_EXTRA_ARGS_SIZE);

	free(header);

	return buf;
}

