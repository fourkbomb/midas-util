#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
	int fd = open("/dev/fb0", O_RDWR);
	if (fd < 0) {
		printf("Failed to open fb0: %s\n", strerror(errno));
		return 1;
	}

	int ret = ioctl(fd, FBIOBLANK, FB_BLANK_NORMAL);
	if (ret < 0) {
		printf("blank ioctl failed: %s\n", strerror(errno));
		return 2;
	}

	ret = ioctl(fd, FBIOBLANK, FB_BLANK_UNBLANK);
	if (ret < 0) {
		printf("unblank ioctl failed: %s\n", strerror(errno));
		return 3;
	}

	close(fd);

	return 0;
}
