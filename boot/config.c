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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "gpio.h"
#include "llist.h"
#include <ini.h>

static char **split(const char *str, char *delim, int *sz) {
	char *token, *tofree, *string;
	char **ret;
	int i;
	int nents = 2; // always at least one token + sentinel at end of array
	int len = strlen(str);
	int dlen = strlen(delim);

	for (i = 0; i < len; i++) {
		if (strncmp(&str[i], delim, dlen) == 0)
			nents++;
	}

	if (sz)
		*sz = nents-1;

	ret = calloc(nents, sizeof(*ret));

	tofree = string = strdup(str);
	if (string == NULL)
		return NULL;

	i = 0;
	while ((token = strsep(&string, ",")) != NULL) {
		ret[i] = strdup(token);
		i++;
	}

	free(tofree);
	return ret;
}

static int handle_global(struct global_config *cfg, const char *name,
		const char *value) {
	int i;

	#define MATCH(key) (strcmp(name, key) == 0)
	if (MATCH("devices")) {
		int count;
		char **devices = split(value, ",", &count);
		cfg->devices = calloc(count + 1, sizeof(struct device_config *));
		cfg->devices[count] = NULL;
		for (i = 0; i < count; i++) {
			cfg->devices[i] = malloc(sizeof(struct device_config));
			cfg->devices[i]->codename = devices[i];
			cfg->devices[i]->overlays = NULL;
		}
	} else if (MATCH("rootdir")) {
		cfg->rootdir = strdup(value);
	} else if (MATCH("zImage")) {
		cfg->zImageName = strdup(value);
	} else if (MATCH("initramfs")) {
		cfg->initramfsName = strdup(value);
	} else if (MATCH("dtbs")) {
		cfg->dtbFolder = strdup(value);
	} else if (MATCH("cmdline")) {
		cfg->cmdline = strdup(value);
	} else {
		fprintf(stderr, "unknown global config option %s\n", name);
		return 0;
	}
	#undef MATCH

	return 1;
}

static int handle_device(struct device_config *dev, const char *name,
		const char *value) {

	#define MATCH(key) (strcmp(name, key) == 0)
	if (MATCH("name")) {
		dev->name = strdup(value);
	} else if (MATCH("model")) {
		dev->models = split(value, ",", NULL);
	} else if (MATCH("dtbs")) {
		dev->dtbs = split(value, ",", NULL);
	}
	#undef MATCH

	return 1;
}

static struct overlay_cfg *find_or_create_overlay(struct device_config *dev, const char *name) {
	node_t *n = dev->overlays;
	struct overlay_cfg *overlay;

	while (n != NULL) {
		overlay = (struct overlay_cfg *)listGet(n);
		if (strcmp(name, overlay->name) == 0)
			return overlay;
		n = listNext(dev->overlays, n);
	}

	overlay = calloc(1, sizeof(*overlay));
	overlay->name = strdup(name);
	if (dev->overlays != NULL)
		listAppend(dev->overlays, overlay);
	else
		dev->overlays = listCreate(overlay);
	return overlay;
}

static int handle_gpio_overlay(struct overlay_cfg *overlay, const char *key,
		const char *value) {

	#define MATCH(k) (strcmp(key, k) == 0)
	if (MATCH("bank")) {
		char *name = get_gpio_name(value);
		if (!name) {
			fprintf(stderr, "invalid gpio bank: %s\n", name);
			return 0;
		}
		overlay->u.gpio.bank = strdup(name);
	} else if (MATCH("pin")) {
		overlay->u.gpio.pin = atoi(value);
	} else if (MATCH("value")) {
		overlay->u.gpio.value = atoi(value);
	} else {
		return 0;
	}
	return 1;
}

static int handle_device_overlay(struct device_config *dev, const char *oname,
		const char *key, const char *value) {
	struct overlay_cfg *overlay = find_or_create_overlay(dev, oname);

	#define MATCH(k) (strcmp(key, k) == 0)
	if (MATCH("path")) {
		overlay->path = strdup(value);
	} else if (MATCH("mode")) {
		#define MODE(m) (strcmp(value, m) == 0)
		if (MODE("fixed"))
			overlay->mode = MODE_FIXED;
		else if (MODE("gpio"))
			overlay->mode = MODE_GPIO;
		else
			overlay->mode = MODE_INVALID;
		#undef MODE
	} else {
		if (overlay->mode == MODE_GPIO)
			return handle_gpio_overlay(overlay, key, value);
		return 0;
	}



	#undef MATCH

	return 1;
}

static int handler(void *user, const char *section, const char *name,
		const char *value) {
	struct global_config *cfg = (struct global_config *)user;

	#define MATCH(sect) (strcmp(section, sect) == 0)
	if (MATCH("global"))
		return handle_global(cfg, name, value);
	else {
		if (!cfg->devices) {
			fprintf(stderr, "global section should be first!\n");
			return 0;
		}
		int i, len;
		for (i = 0; cfg->devices[i] != NULL; i++) {
			len = strlen(cfg->devices[i]->codename);
			if (strcmp(cfg->devices[i]->codename, section) == 0) {
				printf("%s == %s\n", cfg->devices[i]->codename, section);
				return handle_device(cfg->devices[i], name, value);
			} else if (strncmp(cfg->devices[i]->codename, section, len) == 0 && strstr(section, ".overlay") != NULL) {
				return handle_device_overlay(cfg->devices[i], strstr(section, ".overlay")+1, name, value);
			}
		}
	}
	#undef MATCH

	return 0;
}

struct global_config *load_config(char *file) {
	struct global_config *cfg = calloc(1, sizeof(*cfg));
	int ret;

	if ((ret = ini_parse(file, handler, cfg)) < 0) {
		fprintf(stderr, "Failed to open config file!\n");
		return NULL;
	} else if (ret > 0) {
		fprintf(stderr, "Parse error occured on line %d\n", ret);
	}


	return cfg;
};

#ifdef CONFIG_TEST
int main(int argc, char *argv[]) {
	if (argc < 2)
		return 1;

	struct global_config *cfg = load_config(argv[1]);
	int i;

	for (i = 0; cfg->devices[i] != NULL; i++) {
		printf("%s\n", cfg->devices[i]->codename);
		if (cfg->devices[i]->overlays) {
			node_t *n = cfg->devices[i]->overlays;
			struct overlay_cfg *o;

			while (n != NULL) {
				o = listGet(n);
				printf("\t %s, type=%d\n", o->name,o->mode);
				n = listNext(cfg->devices[i]->overlays, n);
			}
		}

	}
}
#endif
