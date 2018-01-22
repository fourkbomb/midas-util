/*
 * Copyright (C) 2018 Simon Shields <simon@lineageos.org>
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
#ifndef _KERNEL_H
#define _KERNEL_H
#include "config.h"

void *load_zimage(struct global_config *config, off_t *len);

void *load_ramdisk(struct global_config *config, off_t *len);

unsigned long long get_kernel_base_addr(void);

char *get_cmdline(struct global_config *cfg, char *root);
#endif
