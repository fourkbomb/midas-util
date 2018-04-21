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
#include "cmdline.h"
#include "config.h"
#include "kernel.h"
#include "util.h"

#define CMDLINE_LEN 1024 /* arch/arm/include/uapi/asm/setup.h */
#define LINE_SIZE 160
#define RAM_STR "System RAM\n"
#define TEXT_OFFSET 0x8000

void *load_zimage(struct global_config *cfg, off_t *len) {
	void *ret = load_android_zimage(cfg, len);
	if (ret != NULL)
		return ret;

	return load_file(cfg, cfg->zImageName, len);
}

void *load_ramdisk(struct global_config *cfg, off_t *len) {
	void *ret = load_android_initrd(cfg, len);
	if (ret != NULL)
		return ret;

	return load_file(cfg, cfg->initramfsName, len);
}

unsigned long long get_kernel_base_addr(void) {
	FILE *fp = fopen("/proc/iomem", "r");
	if (!fp) {
		fprintf(stderr, "couldn't open iomem: %s\n", strerror(errno));
		return (unsigned long)-1;
	}
	char buf[LINE_SIZE];
	unsigned long long start, end;
	int ok = 0;

	/* FIXME: is it possible that the RAM area selected won't have enough space? */
	while (fgets(buf, sizeof(buf), fp) != 0) {
		int pos;
		char *name;
		int count = sscanf(buf, "%llx-%llx : %n", &start, &end, &pos);
		if (count != 2) continue;
		name = buf + pos;

		if (memcmp(name, RAM_STR, strlen(RAM_STR)) == 0) {
			ok = 1;
			break;
		}
	}

	if (!ok)
		return (unsigned long)-1;

	int page_size = getpagesize();

	return ALIGN(start + TEXT_OFFSET, page_size);
}

static int strlcat(char *dest, char *src, int destlen) {
	int i, j;
	for (i = 0; i < destlen; i++) {
		if (dest[i] == 0)
			break;
	}

	if (i >= destlen)
		return -1;

	if (i + strlen(src) + 1 >= destlen)
		return -1;

	for (j = 0; j < strlen(src) && i < destlen; i++, j++) {
		dest[i] = src[j];
	}

	dest[i] = '\0';

	return 0;
}

char *get_cmdline(struct global_config *cfg, char *root) {
	char buf[CMDLINE_LEN];
	char *res = calloc(CMDLINE_LEN, sizeof(char));
	char *cmdline = load_android_cmdline(cfg);
	if (cmdline == NULL)
		cmdline = cfg->cmdline;
	if (cmdline == NULL)
		cmdline = "";

	snprintf(res, CMDLINE_LEN, "root=%s %s", root, cmdline);
	free(cmdline);

	/* and copy over any other cmdline options */
	for (int i = 0; i < cfg->keep_cmdline_len; i++) {
		char *key = cfg->keep_cmdline[i];
		char *val = cmdline_get_value(key);
		if (val) {
			snprintf(buf, CMDLINE_LEN, " %s=%s", key, val);
		} else if (cmdline_has_key(key)) {
			snprintf(buf, CMDLINE_LEN, " %s", key);
		}
		strlcat(res, buf, CMDLINE_LEN);
	}

	return res;
}
