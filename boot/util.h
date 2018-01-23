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
#ifndef _UTIL_H
#define _UTIL_H
#define _ALIGN(addr, size) (((addr) + (size)-1) & ~((size) - 1))
#define ALIGN(addr, size) _ALIGN(addr, (typeof(addr))(size))
#define ALIGN_DOWN(addr, size) ((addr) & ~(size))

struct device_config *get_cur_device(struct global_config *cfg);
void *load_dtb(struct global_config *cfg, struct device_config *dev, off_t *dtbsz);
void *load_file(struct global_config *cfg, char *file, off_t *sz);
void *load_overlay(struct global_config *cfg, struct overlay_cfg *overlay, off_t *sz);

int util_has_cmdline(struct cmdline_overlay_cfg *cfg);
#endif
