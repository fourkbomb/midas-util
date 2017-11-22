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
#include <ufdt_overlay.h>

#include "config.h"
#include "gpio.h"
#include "ufdt.h"
#include "util.h"

struct fdt_header *apply_overlays(struct global_config *cfg, struct device_config *dev,
		void *dtb_buf, int *dtb_size) {
	struct fdt_header *hdr = ufdt_install_blob(dtb_buf, *dtb_size);

	if (!dev->overlays)
		return hdr;

	node_t *n = dev->overlays;
	struct overlay_cfg *o;

	while (n != NULL) {
		o = listGet(n);
		switch (o->mode) {
			case MODE_FIXED:
				// always apply
				break;
			case MODE_GPIO:
				if (!gpio_should_apply(&o->u.gpio))
					goto skip_apply;
				// gpio indicates should apply, so apply
				break;
			default:
				fprintf(stderr, "overlay '%s' has an invalid mode\n", o->name);
				goto skip_apply;
		}

		int sz;
		void *buf = load_overlay(cfg, o, &sz);
		hdr = ufdt_apply_overlay(hdr, *dtb_size,
				buf, sz);
		*dtb_size = fdt_totalsize(hdr);

skip_apply:
		n = listNext(dev->overlays, n);
	}

	return hdr;
}

