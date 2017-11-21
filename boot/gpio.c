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

			printf("got gpiochip %s\n", info.label);
			if (strcmp(label, info.label) == 0) {
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
