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
#include <errno.h>

#include <libfdt.h>
#include <ufdt_overlay.h>

#include "config.h"
#include "gpio.h"
#include "ufdt.h"
#include "util.h"

#define FDT_ALIGN(x, a)		(((x) + (a) - 1) & ~((a) - 1))
#define FDT_TAGALIGN(x)		(FDT_ALIGN((x), FDT_TAGSIZE))
/*
 * if add a new subnode:
 * see: fdt_add_subnode -> fdt_add_subnode_namelen
 */
static inline int fdt_node_len(const char* node_name)
{
	return sizeof(struct fdt_node_header) +
		FDT_TAGALIGN(strlen(node_name) + 1) + FDT_TAGSIZE;
}

/*
 * if add a new prop: (assume prop_name not exist in strtab)
 * see: fdt_setprop -> _fdt_add_property
 */
static inline int fdt_prop_len(const char* prop_name, int len)
{
	return (strlen(prop_name) + 1) +
		sizeof(struct fdt_property) +
		FDT_TAGALIGN(len);
}


struct fdt_header *apply_overlays(struct global_config *cfg, struct device_config *dev,
		void *dtb_buf, off_t *dtb_size) {
	struct fdt_header *hdr = ufdt_install_blob(dtb_buf, *dtb_size);
	struct fdt_header *new;

	if (!dev->overlays)
		return hdr;

	node_t *n = dev->overlays;
	struct overlay_cfg *o;

	while (n != NULL) {
		o = listGet(n);
		int applied = 0;
		switch (o->mode) {
			case MODE_FIXED:
				// always apply
				break;
			case MODE_GPIO:
				if (!gpio_should_apply(&o->u.gpio))
					goto skip_apply;
				// gpio indicates should apply, so apply
				break;
			case MODE_CMDLINE:
				if (!util_has_cmdline(o->u.cmdline.key, o->u.cmdline.value))
					goto skip_apply;
				break;
			default:
				fprintf(stderr, "overlay '%s' has an invalid mode\n", o->name);
				goto skip_apply;
		}
		printf("applying overlay '%s'\n", o->name);
		applied = 1;
		off_t sz;
		void *buf = load_overlay(cfg, o, &sz);
		if (buf == NULL) {
			printf("loading overlay failed\n");
			applied = 0;
			goto skip_apply;
		}
		new = ufdt_apply_overlay(hdr, *dtb_size,
				buf, sz);
		free(hdr);
		free(buf);
		hdr = new;
		*dtb_size = fdt_totalsize(hdr);

skip_apply:
		if (!applied)
			printf("not applying overlay '%s'\n", o->name);
		n = listNext(dev->overlays, n);
	}

	return hdr;
}

int setup_dtb_prop(struct fdt_header **dtb, off_t *size, const char *node_name,
		const char *prop_name, const void *val, size_t len) {
	void *buf = (void*)*dtb;
	off_t sz = *size;
	int off;
	int prop_len = 0;
	const struct fdt_property *prop;

	off = fdt_subnode_offset(buf, 0, node_name);
	if (off == -FDT_ERR_NOTFOUND) {
		sz += fdt_node_len(node_name);
		fdt_set_totalsize(buf, sz);
		buf = realloc(buf, sz);
		if (buf == NULL)
			return -ENOMEM;
		off = fdt_add_subnode(buf, 0, node_name);
	}

	if (off < 0) {
		return -EINVAL;
	}

	prop = fdt_get_property(buf, off, prop_name, &prop_len);
	if ((prop == NULL) && (prop_len != -FDT_ERR_NOTFOUND))
		return -EINVAL;
	else if (prop == NULL) {
		sz += fdt_prop_len(prop_name, len);
	} else {
		if (prop_len < len)
			sz += FDT_TAGALIGN(len - prop_len);
	}

	if (fdt_totalsize(buf) < sz) {
		fdt_set_totalsize(buf, sz);
		buf = realloc(buf, sz);
		if (buf == NULL)
			return -ENOMEM;
	}

	if (fdt_setprop(buf, off, prop_name, val, len) != 0) {
		fprintf(stderr, "failed to set prop '%s/%s'", node_name, prop_name);
		return -EINVAL;
	}

	*dtb = buf;
	*size = sz;

	return 0;
}

int setup_dtb_prop_int(struct fdt_header **dtb, off_t *size, const char *node_name,
		const char *prop_name, uint32_t val) {
	void *buf = (void*)*dtb;
	off_t sz = *size;
	int off;
	int prop_len = 0;
	const struct fdt_property *prop;

	off = fdt_subnode_offset(buf, 0, node_name);
	if (off == -FDT_ERR_NOTFOUND) {
		sz += fdt_node_len(node_name);
		fdt_set_totalsize(buf, sz);
		buf = realloc(buf, sz);
		if (buf == NULL)
			return -ENOMEM;
		off = fdt_add_subnode(buf, 0, node_name);
	}

	if (off < 0) {
		return -EINVAL;
	}

	prop = fdt_get_property(buf, off, prop_name, &prop_len);
	if ((prop == NULL) && (prop_len != -FDT_ERR_NOTFOUND))
		return -EINVAL;
	else if (prop == NULL) {
		sz += fdt_prop_len(prop_name, sizeof(val));
	} else {
		if (prop_len < sizeof(val))
			sz += FDT_TAGALIGN(sizeof(val) - prop_len);
	}

	if (fdt_totalsize(buf) < sz) {
		fdt_set_totalsize(buf, sz);
		buf = realloc(buf, sz);
		if (buf == NULL)
			return -ENOMEM;
	}

	if (fdt_setprop_cell(buf, off, prop_name, val) != 0) {
		fprintf(stderr, "failed to set prop '%s/%s'", node_name, prop_name);
		return -EINVAL;
	}

	*dtb = buf;
	*size = sz;

	return 0;
}

