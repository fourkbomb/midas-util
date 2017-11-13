#include <unistd.h>
#include <sys/syscall.h>
#include <linux/reboot.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char *argv[]) {
	sync();


	if (argc < 2) {
		printf("Usage: %s [<mode>]\n", argv[0]);
		return 1;
	}

	syscall(__NR_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, argv[1]);
	// reboot should never return
	return 0;
}
