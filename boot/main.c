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
#include <stdio.h>

#include <libfdt.h>

#include "config.h"
#include "ufdt.h"
#include "util.h"

int main(int argc, char * argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <config.ini>\n", argv[0]);
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
	struct fdt_header *dtb = apply_overlays(cfg, dev, buf, dtbsz);
	(void)dtb;
	// load zImage, ramdisk

	// kexec!

	return 0;
}
