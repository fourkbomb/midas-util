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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tinyalsa/mixer.h>

#include "llist.h"
#include "xml.h"

int apply_paths(node_t *widget_list, struct mixer *mixer) {
	node_t *n = NULL;
	struct widget *wid = NULL;

	while ((n = listNext(widget_list, n)) != NULL) {
		wid = listGet(n);
		printf("\t%s => ", wid->name);
		struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, wid->name);
		if (!ctl) {
			fprintf(stderr, "Didn't find CTL '%s'\n", wid->name);
			continue;
		}
		if (wid->strval) {
			printf("%s\n", wid->strval);
			mixer_ctl_set_enum_by_string(ctl, wid->strval);
		}
		else {
			printf("%d\n", wid->intval);
			for (int j = 0; j < mixer_ctl_get_num_values(ctl); j++) {
				if (mixer_ctl_set_value(ctl, j, wid->intval) != 0)
					fprintf(stderr, "Couldn't set '%s'\n", wid->name);
			}
		}
	}
}

node_t *find_path(struct mixer_config *cfg, char *name, int en) {
	node_t *n = NULL;
	struct mixer_setting *set;

	while ((n = listNext(cfg->settings, n)) != NULL) {
		set = listGet(n);
		if (!strcmp(set->name, name)) {
			if (en)
				return set->en_widgets;
			return set->dis_widgets;
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "usage: %s <mixer_paths.xml> <path_name> <1|0>\n", argv[0]);
		return 1;
	}

	struct mixer_config *cfg = parse_config(argv[1]);

	if (argc < 4) {
		fprintf(stderr, "usage: %s <mixer_paths.xml> <path_name> <1|0>\n", argv[0]);
		printf("Available paths:\n");
		struct mixer_setting *set;
		node_t *n = NULL;

		while ((n = listNext(cfg->settings, n)) != NULL) {
			set = listGet(n);
			printf("%s\n", set->name);
		}
		return 2;
	}

	struct mixer *mixer = mixer_open(0);
	apply_paths(cfg->default_widgets, mixer);

	node_t *widg = find_path(cfg, argv[2], atoi(argv[3]));
	if (!widg) {
		fprintf(stderr, "No such path!\n");
		return 2;
	}
	apply_paths(widg, mixer);
}
