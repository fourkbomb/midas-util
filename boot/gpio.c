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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/gpio.h>

#include "config.h"
#include "gpio.h"

char *get_gpio_name(const char *label) {
	struct dirent *ent;
	struct gpiochip_info info;
	int fd, ret;

	DIR *dir = opendir("/dev");
	if (!dir) {
		printf("opendir failed\n");
		return NULL;
	}

	while ((ent = readdir(dir)) != NULL) {
		if (strstr(ent->d_name, "gpiochip") == ent->d_name) {
			fd = openat(dirfd(dir), ent->d_name, 0);
			if (fd < 0) {
				printf("open %s failed: %s\n", ent->d_name, strerror(errno));
				goto err_open;
			}

			ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &info);
			if (ret < 0) {
				printf("ioctl failed: %s\n", strerror(errno));
				goto err_ioctl;
			}
			close(fd);

			if (strcmp(label, info.label) == 0) {
				closedir(dir);
				return strdup(info.name);
			}
		}
	}
	closedir(dir);

	return NULL;
err_ioctl:
	close(fd);
err_open:
	closedir(dir);
	return NULL;
}

int gpio_should_apply(struct gpio_overlay_cfg *cfg) {
	if (cfg->bank == NULL)
		return -EINVAL;

	char buf[36];
	snprintf(buf, sizeof(buf), "/dev/%s", cfg->bank);

	int fd = open(buf, 0);
	if (fd < 0) {
		printf("failed to open %s: %s\n", buf, strerror(errno));
		return -errno;
	}

	struct gpiohandle_request req = {
		.lineoffsets = { cfg->pin, },
		.flags = GPIOHANDLE_REQUEST_INPUT,
		.consumer_label = "midas-boot",
		.lines = 1,
	};

	int ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
	if (ret < 0) {
		printf("failed to get linehandle: %s\n", strerror(errno));
		goto err_ioctl;
	}

	if (req.fd <= 0) {
		printf("invalid fd from linehandle ioctl: %d\n", req.fd);
		goto err_ioctl;
	}

	struct gpiohandle_data data;
	ret = ioctl(req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	close(req.fd);
	close(fd);
	if (ret < 0) {
		printf("failed to get line values: %s\n", strerror(errno));
		return -errno;
	}
	printf("%s pin %d == %d\n", cfg->bank, cfg->pin, data.values[0]);
	return data.values[0] == cfg->value;

err_ioctl:
	close(fd);
	return -errno;
}
