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

typedef struct node node_t;

struct global_config *load_config(char *file);

struct global_config {
	// list of devices
	struct device_config **devices;
	// top dir, containing kernel/initramfs/dtbs
	char *rootdir;
	// next three members are relative to rootdir
	// kernel name
	char *zImageName;
	// initramfs name
	char *initramfsName;
	// folder with dtbs
	char *dtbFolder;
	// kernel cmdline. root=<blah> will automatically be prepended to this string
	char *cmdline;
};

// core configuration structure, per-device
struct device_config {
	// device codename (t0, m0, etc)
	char *codename;
	// human-readable name
	char *name;
	// models - first five chars of androidboot.bootloader
	char **models;
	// dtbs to try, relative to dtbFolder
	char **dtbs;
	// overlays - linked list
	node_t *overlays;
};

// overlay mode
enum overlay_mode {
	MODE_INVALID = -1,
	MODE_UNKNOWN = 0,
	// always apply this overlay
	MODE_FIXED,
	// apply this overlay based on a GPIO pin's value
	MODE_GPIO,
};

struct gpio_overlay_cfg {
	// linux chip name of pin bank. converted from "gpa1" form by config parser
	char *bank;
	// pin number, e.g. 4
	int pin;
	// 1 (pin is HIGH) or 0 (pin is LOW).
	// if pin matches value here, overlay will be applied
	int value;
};

// dtb overlay config
struct overlay_cfg {
	// path of overlay, relative to dtbFolder
	char *path;
	// section name in ini
	char *name;
	// overlay mode
	enum overlay_mode mode;
	// mode-specific configuration
	union {
		struct gpio_overlay_cfg gpio;
	} u;
};


