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
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/kexec.h>
#include <linux/reboot.h>

#include <libfdt.h>

#include "cmdline.h"
#include "config.h"
#include "kernel.h"
#include "ufdt.h"
#include "util.h"

struct zimage_header {
	uint32_t instr[9];
	uint32_t magic;
#define ZIMAGE_MAGIC 0x016f2818
	uint32_t start;
	uint32_t end;
};

static off_t check_zimage(void *zimage, off_t sz) {
	if (sz < 0x34)
		return -1;

	const struct zimage_header *hdr = (const struct zimage_header *)zimage;
	if (hdr->magic != ZIMAGE_MAGIC)
		return -1;

	uint32_t actual_sz = hdr->end - hdr->start;
	if (actual_sz > sz) {
		fprintf(stderr, "zImage is truncated: header says length=0x%x bytes, file is 0x%lx bytes.\n", actual_sz, sz);
		return -1;
	} else if (sz > actual_sz)
		return actual_sz;

	return sz;
}

static void dump_kexec_segs(struct kexec_segment *s, int nr_segs) {
	int i;
	for (i = 0; i < nr_segs; i++) {
		printf("Kexec segment %d:\n", i);
		printf("\tUserspace buffer: 0x%x @ %p\n", (unsigned int)s[i].bufsz, s[i].buf);
		printf("\tDestination buffer: 0x%x @ %p\n", (unsigned int)s[i].memsz, s[i].mem);
	}
}

#ifdef DEBUG
static void dump_buf_to_disk(char *path, void *dtb, off_t size) {
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0) {
		fprintf(stderr, "failed to open dtb to dump: %s\n", strerror(errno));
		return;
	}
	printf("dumping dtb\n");
	write(fd, dtb, size);
	close(fd);
	sync();
}
#endif

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

	/* S-BOOT passes bootmode=2 in recovery mode */
	cfg->is_recovery = cmdline_check_value("bootmode", "2");
	/* KK S-BOOT (N7100/N7105, I9305) */
	cfg->is_lpm = cmdline_check_value("androidboot.mode", "charger");
	/* <= JB S-BOOT (I9300) */
	if (!cfg->is_lpm)
		cfg->is_lpm = cmdline_check_value("lpcharge", "1");

	struct device_config *dev = get_cur_device(cfg);
	if (dev == NULL) {
		fprintf(stderr, "couldn't find matching device for current board!\n");
		return 2;
	}

	printf("Preparing to boot on %s/%s...\n", dev->name, dev->codename);

	// load zImage, ramdisk
	off_t zimagesz, aligned_zsz;
	void *zimage = load_zimage(cfg, &zimagesz);
	aligned_zsz = ALIGN(zimagesz, getpagesize());
#ifdef DEBUG
	dump_buf_to_disk("/mnt/root/zimage", zimage, zimagesz);
#endif

	off_t rdsz, aligned_rdsz;
	void *ramdisk = load_ramdisk(cfg, &rdsz);
#ifdef DEBUG
	dump_buf_to_disk("/mnt/root/ramdisk", ramdisk, rdsz);
#endif

	aligned_rdsz = ALIGN(rdsz, getpagesize());

	// load DTB
	off_t dtbsz;
	void *buf = load_dtb(cfg, dev, &dtbsz);
	printf("dtb at %p\n", buf);
	if (buf == NULL) {
		fprintf(stderr, "couldn't load dtb\n");
		return 3;
	}

	// apply configured overlays
	struct fdt_header *dtb = apply_overlays(cfg, dev, buf, &dtbsz);

	// TODO: apply serial number, board rev

	// apply cmdline
	char *cmdline = get_cmdline(cfg, argv[2]);
	printf("applying cmdline %s\n", cmdline);
	setup_dtb_prop(&dtb, &dtbsz, "chosen", "bootargs", cmdline, strlen(cmdline)+1);
	free(cmdline);

	unsigned long addr = get_kernel_base_addr();
	if (addr == (unsigned long)-1) {
		printf("failed to get valid paddr\n");
		return 4;
	}

	printf("Loading kernel at 0x%lx\n", addr);

	// figure out how much space the zImage might need
	off_t zimage_len = check_zimage(zimage, zimagesz);
	if (zimage_len == (off_t)-1) {
		fprintf(stderr, "invalid zimage\n");
		return 4;
	}

	// assume maximum kernel compression ratio is 4
	off_t decompress_buf = ALIGN(zimage_len * 4, getpagesize());
	uint32_t rd_start = addr + aligned_zsz + decompress_buf;
	uint32_t rd_end = addr + aligned_zsz + decompress_buf + aligned_rdsz;
	printf("setting up initrd fdt args\n");
	setup_dtb_prop_int(&dtb, &dtbsz, "chosen", "linux,initrd-start", rd_start);
	setup_dtb_prop_int(&dtb, &dtbsz, "chosen", "linux,initrd-end", rd_end);
	off_t aligned_dtbsz = ALIGN(dtbsz, getpagesize());


#ifdef DEBUG
	dump_buf_to_disk("/mnt/root/dtb", dtb, dtbsz);
#endif
	struct kexec_segment segs[3] = {
		{
			.buf = zimage,
			.bufsz = zimagesz,
			.mem = (void*)addr,
			.memsz = aligned_zsz,
		}, {
			.buf = ramdisk,
			.bufsz = rdsz,
			.mem = (void*)addr + aligned_zsz + decompress_buf,
			.memsz = aligned_rdsz,
		}, {
			.buf = (void*)dtb,
			.bufsz = dtbsz,
			.mem = (void*)addr + aligned_zsz + decompress_buf + aligned_rdsz,
			.memsz = aligned_dtbsz,
		},
	};
	dump_kexec_segs(segs, 3);

	// kexec!
	int ret = syscall(SYS_kexec_load, addr, 3, segs, KEXEC_ARCH_DEFAULT);
	if (ret < 0) {
		fprintf(stderr, "kexec_load failed: %s\n", strerror(errno));
		return 4;
	}

	printf("Booting the zImage...\n");
	sync();
	syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_KEXEC, NULL);

	// we should never get here
	return 5;
}
