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
#include <libfdt.h>
struct fdt_header *apply_overlays(struct global_config *cfg, struct device_config *dev,
		void *dtb_buf, off_t *dtb_size);
int setup_dtb_prop(struct fdt_header **dtb, off_t *size, const char *node_name,
		const char *prop_name, const void *val, size_t len);
int setup_dtb_prop_int(struct fdt_header **dtb, off_t *size, const char *node_name,
		const char *prop_name, uint32_t val);

