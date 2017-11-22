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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <byteswap.h>

#include <linux/kexec.h>
#include <linux/reboot.h>

#include <libfdt.h>

#include "config.h"
#include "ufdt.h"
#include "util.h"

// we only support ARM little-endian
#define cpu_to_be32(x) bswap_32(x)

#define LINE_SIZE 256
#define RAM_STR "System RAM\n"
#define ALIGN(addr, size) (((addr) + (size)-1) & ((size) - 1))
#define ALIGN_DOWN(addr, size) ((addr) & ~(size))
static unsigned long get_phys_addr(unsigned long size) {
	FILE *fp = fopen("/proc/iomem", "r");
	if (!fp) {
		fprintf(stderr, "couldn't open iomem: %s\n", strerror(errno));
		return (unsigned long)-1;
	}
	char buf[LINE_SIZE];
	unsigned long start, end;

	while (fgets(buf, LINE_SIZE, fp) != 0) {
		int pos;
		char *name;
		scanf("%lu-%lu : %n", &start, &end, &pos);
		name = buf + pos;

		if (strncmp(name, RAM_STR, sizeof(RAM_STR)) == 0) {
			break;
		}
	}

	int page_size = getpagesize();

	return ALIGN_DOWN(end - size, page_size);
}

#define CMDLINE_SZ 1024
char *mkcmdline(char *root) {
	char *res = calloc(1024, sizeof(char));
	// TODO make this configurable
	snprintf(res, CMDLINE_SZ, "root=%s console=ttySAC2,115200", root);
	return res;
}


int main(int argc, char * argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage: %s </path/to/config.ini> </dev/root device>\n", argv[0]);
		return 1;
	}

	if (getuid() != 0) {
		fprintf(stderr, "%s: must be root\n", argv[0]);
		return 1;
	}

	struct global_config *cfg = load_config(argv[1]);
	if (cfg == NULL)
		return 1;

	struct device_config *dev = get_cur_device(cfg);
	if (dev == NULL) {
		fprintf(stderr, "couldn't find matching device for current board!\n");
		return 2;
	}

	// load DTB
	int dtbsz;
	void *buf = load_dtb(cfg, dev, &dtbsz);
	if (buf == NULL) {
		fprintf(stderr, "couldn't load dtb\n");
		return 3;
	}
	// apply overlays
	struct fdt_header *dtb = apply_overlays(cfg, dev, buf, &dtbsz);
	// TODO: apply serial number, board rev
	char *cmdline = mkcmdline(argv[2]);
	setup_dtb_prop(&dtb, &dtbsz, "chosen", "bootargs", cmdline, strlen(cmdline)+1);
	free(cmdline);

	// load zImage, ramdisk
	int zimagesz, aligned_zsz;
	void *zimage = load_file(cfg, cfg->zImageName, &zimagesz);
	aligned_zsz = ALIGN(zimagesz, getpagesize());

	int rdsz, aligned_rdsz;
	void *ramdisk = load_file(cfg, cfg->initramfsName, &rdsz);

	aligned_rdsz = ALIGN(rdsz, getpagesize());

	int	aligned_dtbsz = ALIGN(dtbsz, getpagesize());

	unsigned long addr = get_phys_addr(aligned_rdsz + aligned_zsz + aligned_dtbsz);

	unsigned long rd_start = cpu_to_be32(addr + aligned_zsz);
	unsigned long rd_end = cpu_to_be32(addr + aligned_zsz + aligned_rdsz);
	setup_dtb_prop(&dtb, &dtbsz, "chosen", "linux,initrd-start", &rd_start, sizeof(rd_start));
	setup_dtb_prop(&dtb, &dtbsz, "chosen", "linux,initrd-end", &rd_end, sizeof(rd_end));

	struct kexec_segment segs[3] = {
		{
			.buf = zimage,
			.bufsz = zimagesz,
			.mem = (void*)addr,
			.memsz = aligned_zsz,
		}, {
			.buf = ramdisk,
			.bufsz = rdsz,
			.mem = (void*)addr + aligned_zsz,
			.memsz = aligned_rdsz,
		}, {
			.buf = dtb,
			.bufsz = dtbsz,
			.mem = (void*)addr + aligned_zsz + aligned_rdsz,
			.memsz = aligned_dtbsz,
		},
	};

	// kexec!
	int ret = syscall(SYS_kexec_load, addr, 3, segs, KEXEC_ARCH_DEFAULT);
	if (ret < 0) {
		fprintf(stderr, "kexec_load failed: %s\n", strerror(errno));
		return 4;
	}

	printf("rebooting...\n");
	syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_KEXEC, NULL);

	// we should never get here
	return 5;
}
