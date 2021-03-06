/*
 * Copyright (C) 2017 Simon Shields <simon@lineageos.org>
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"

#define BUF_SIZE 1024
#define BL_NAME_LEN (sizeof("I9300") - 1)

static char *get_bootloader(void) {
	int ret;
	char buf[BUF_SIZE];
	char *bl_name;
	int fd = open("/proc/cmdline", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to read cmdline: %s\n", strerror(errno));
		return NULL;
	}

	while ((ret = read(fd, buf, BUF_SIZE)) > 0) {
		if ((bl_name = strstr(buf, "androidboot.bootloader=")) != NULL) {
			bl_name = strstr(bl_name, "=") + 1;
			bl_name[BL_NAME_LEN] = 0; // hack
			break;
		}
	}

	close(fd);
	if (ret <= 0) {
		fprintf(stderr, "Failed to find androidboot.bootloader parameter: %s\n", (ret == 0 ? "EOF" : strerror(errno)));
		return NULL;
	}

	return strdup(bl_name);
}

int util_has_cmdline(char *key, char *val) {
	char buf[BUF_SIZE];
	char *key_loc;
	char *space_loc;
	int ret;
	int fd = open("/proc/cmdline", O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "failed to read cmdline: %s\n", strerror(errno));
		return 0;
	}

	while ((ret = read(fd, buf, BUF_SIZE)) > 0) {
		if ((key_loc = strstr(buf, key)) != NULL) {
			key_loc = strstr(key_loc, "=") + 1;
			space_loc = strstr(buf, " ");
			space_loc[0] = 0;

			/* is it a match? */
			if (strncmp(key_loc, val, strlen(val)) == 0)
				break;

			space_loc[0] = ' ';
		}
	}

	close(fd);
	if (ret <= 0) {
		fprintf(stderr, "Failed to find read parameter: %s\n", (ret == 0 ? "EOF" : strerror(errno)));
		return 0;
	}

	return 1;
}

struct device_config *get_cur_device(struct global_config *cfg) {
	char *blname = get_bootloader();
	if (blname == NULL)
		return NULL;

	struct device_config *ret;
	int i, j, done = 0;
	for (i = 0; cfg->devices[i]; i++) {
		ret = cfg->devices[i];

		for (j = 0; ret->models[j]; j++) {
			if (strncmp(ret->models[j], blname, BL_NAME_LEN) == 0) {
				done = 1;
				break;
			}
		}
		if (done) break;
	}

	if (!done)
		return NULL;
	return ret;
}

// this will close the fd
static void *load_blob(int fd, off_t *dtbsz) {
	off_t length = lseek(fd, 0, SEEK_END);
	if (length == (off_t)-1) {
		fprintf(stderr, "lseek to end failed: %s\n", strerror(errno));
		goto err_open;
	}

	*dtbsz = length;
	if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
		fprintf(stderr, "lseek to start failed: %s\n", strerror(errno));
		goto err_open;
	}

	void *buf = malloc(length);
	if (buf == NULL) {
		fprintf(stderr, "Failed to allocate %ld byte buffer for dtb\n", length);
		goto err_open;
	}

	void *cur = buf;
	int ret;
	while ((ret = read(fd, cur, 4096)) > 0) {
		cur += ret;
	}

	if (ret < 0) {
		fprintf(stderr, "Failed to read file: %s\n", strerror(errno));
		goto err_malloc;
	}
	printf("done\n");
	close(fd);
	return buf;
err_malloc:
	free(buf);
err_open:
	close(fd);
	return NULL;
}

void *load_overlay(struct global_config *cfg, struct overlay_cfg *overlay, off_t *sz) {
	char path[256];
	snprintf(path, 255, "%s/%s/%s", cfg->rootdir, cfg->dtbFolder, overlay->path);
	int dtbfd = open(path, O_RDONLY);
	if (dtbfd < 0) {
		fprintf(stderr, "failed to open overlay %s: %s\n", path, strerror(errno));
		return NULL;
	}

	return load_blob(dtbfd, sz);
}

void *load_file(struct global_config *cfg, char *file, off_t *sz) {
	char path[256];
	snprintf(path, 255, "%s/%s", cfg->rootdir, file);
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "failed to open zImage %s: %s\n", path, strerror(errno));
		return NULL;
	}
	printf("loading blob %s\n", path);

	return load_blob(fd, sz);
}

void *load_dtb(struct global_config *cfg, struct device_config *dev, off_t *dtbsz) {
	int i, dtbfd;
	char path[256];

	for (i = 0; dev->dtbs[i]; i++) {
		snprintf(path, 255, "%s/%s/%s",  cfg->rootdir, cfg->dtbFolder, dev->dtbs[i]);
		dtbfd = open(path, O_RDONLY);
		if (dtbfd < 0) {
			fprintf(stderr, "failed to open %s: %s\n", path, strerror(errno));
			continue;
		}
		// load_blob closes the fd for us.
		printf("loading dtb %s\n", path);
		return load_blob(dtbfd, dtbsz);
	}

	fprintf(stderr, "couldn't find any matching dtbs!\n");
	return NULL;
}
