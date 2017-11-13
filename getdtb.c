#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>

#define BUF_SIZE 1024
#define BL_NAME_LEN (sizeof("I9300") - 1)
#define NUM_DTBS 2

struct device {
	// first 5 chars of androidboot.bootlodaer
	const char *bl_name;
	// list of DTBs to try, in order
	const char *dtbs[NUM_DTBS];
};

struct device devices[] = {
	{
		.bl_name = "I9300",
		.dtbs = {"m0", "galaxy-s3-common"},
	}, {
		.bl_name = "I9305",
		.dtbs = {"m3", "galaxy-s3-common"},
	}, {
		.bl_name = "N7100",
		.dtbs = {"t0-3g", "t0-common"},
	}, {
		.bl_name = "N7105",
		.dtbs = {"t0-lte", "t0-common"},
	}, {
		// sentinel
		.bl_name = NULL,
	},
};

int main(int argc, char* argv[]) {

	char buf[BUF_SIZE];
	char *bl_name;
	char *chosen_dtb;
	int dtbfd;
	struct device *device;
	ssize_t ret;

	if (argc < 2) {
		printf("usage: %s <dtb-base>\n", argv[0]);
		return 1;
	}

	int fd = open("/proc/cmdline", O_RDONLY);
	if (fd < 0) {
		printf("Failed to to read cmdline: %s\n", strerror(errno));
		return 1;
	}

	while ((ret = read(fd, buf, BUF_SIZE)) > 0) {
		if (bl_name = strstr(buf, "androidboot.bootloader=")) {
			bl_name = strstr(bl_name, "=") + 1;
			bl_name[BL_NAME_LEN] = 0; // hack
			break;
		}
	}

	if (ret <= 0) {
		printf("Failed to find androidboot.bootloader parameter: %s\n", (ret == 0 ? "EOF" : strerror(errno)));
		goto err_find;
	}

	for (int i = 0; devices[i].bl_name != NULL; i++) {
		device = &devices[i];
		printf("strcmp(%s, %s) = ", bl_name, device->bl_name);
		if (strncmp(bl_name, device->bl_name, BL_NAME_LEN) != 0) {
			device = NULL;
			printf("no match\n");
		} else {
			printf("match\n");
			break;
		}
	}

	if (device == NULL) {
		printf("Couldn't find DTB matching device '%s'\n", bl_name);
		goto err_find;
	}

	for (int i = 0; i < NUM_DTBS; i++) {
		if (device->dtbs[i] == NULL)
			break;
		memset(buf, 0, BUF_SIZE);
		strcpy(buf, argv[1]);
		strcat(buf, "exynos4412-");
		strcat(buf, device->dtbs[i]);
		strcat(buf, ".dtb");

		dtbfd = open(buf, O_RDONLY);
		if (dtbfd < 0) {
			printf("Failed to open dtb '%s': %s\n", buf, strerror(errno));
			continue;
		}

		close(dtbfd);
		dtbfd = 0;
		break;
	}

	if (dtbfd < 0) {
		printf("no dtb found\n");
		return 1;
	}
	printf("%s\n", buf);

	return 0;

err_find:
	close(fd);
	return 1;
}
