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

#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mixer.h"

struct parser_data {
	struct mixer_config *cfg;
	char *cur_device; // <device name="blah">
	int cur_state; // <path name="on">
	node_t **cur_widget_list;
};

node_t **find_list(struct parser_data *d) {
	char *device = d->cur_device;
	int state = d->cur_state;
	struct mixer_config *cfg = d->cfg;

	if (device == NULL)
		return &cfg->default_widgets;

	if (cfg->settings == NULL)
		return NULL;

	node_t *n = NULL;
	struct mixer_setting *set;
	while ((n = listNext(cfg->settings, n)) != NULL) {
		set = listGet(n);
		if (!strcmp(set->name, device))
			break;
	}

	if (n == NULL)
		return NULL;

	if (state)
		return &set->en_widgets;
	return &set->dis_widgets;
}

static void config_start(void *d, const XML_Char *elem,
		const XML_Char **attr)
{
	struct parser_data *data = d;
	const char *name = NULL;
	const char *val = NULL;
	node_t **wlist;
	struct mixer_setting *set;

	for (int i = 0; attr[i]; i += 2) {
		if (!strcmp(attr[i], "name"))
			name = attr[i+1];
		else if (!strcmp(attr[i], "val"))
			val = attr[i+1];
	}

	if (!strcmp(elem, "path")) {
		if (name) {
			if (!strcmp(name, "on"))
				data->cur_state = 1;
			else
				data->cur_state = 0;
			printf("PATH STATE=%s\n", name);
		} else if (data->cur_device)
			fprintf(stderr, "Device '%s' has sub-path with no name!\n", data->cur_device);

		/* find widget list for this path */
		wlist = find_list(d);
		if (wlist == NULL) {
			/* make the struct mixer_setting for this device if necessary */
			set = calloc(1, sizeof(*set));
			set->name = strdup(data->cur_device);

			if (!data->cfg->settings)
				data->cfg->settings = listCreate(set);
			else
				listAppend(data->cfg->settings, set);

			if (data->cur_state)
				wlist = &set->en_widgets;
			else
				wlist = &set->dis_widgets;
		}

		data->cur_widget_list = wlist;
	} else if (!strcmp(elem, "device")) {
		if (!name)
			fprintf(stderr, "device has no name!\n");
		else {
			printf("**%s**\n", name);
			data->cur_device = strdup(name);
		}
	} else if (!strcmp(elem, "ctl")) {
		if (!name || !val) {
			fprintf(stderr, "ctl has no name or no value!\n");
			return;
		}
		if (!data->cur_widget_list) {
			fprintf(stderr, "ctl in invalid context (must be child of a <path> element!)\n");
			return;
		}

		node_t **list = data->cur_widget_list;
		struct widget *w = calloc(1, sizeof(*w));
		if (!w) {
			fprintf(stderr, "out of memory allocating ctl for '%s'\n", name);
			return;
		}

		w->name = strdup(name);
		w->intval = atoi(val);
		printf("\t%s => %s\n", w->name, val);
		if (!w->intval && strcmp(val, "0")) {
			w->strval = strdup(val);
		}

		if (!*list) {
			*list = listCreate(w);
		} else {
			listAppend(*list, w);
		}
	}
}

static void config_end(void *d, const XML_Char *elem)
{
	struct parser_data *data = d;

	if (!strcmp(elem, "path")) {
		data->cur_state = -1;
		data->cur_widget_list = NULL;
	} else if (!strcmp(elem, "device")) {
		data->cur_device = NULL;
	}
}

struct mixer_config *parse_config(char *path) {
	XML_Parser p;
	FILE *f;
	struct parser_data data;
	char buf[80];
	int eof, len;

	f = fopen(path, "r");
	if (!f) {
		perror("Failed to open mixer paths configuration!");
		return NULL;
	}

	p = XML_ParserCreate(NULL);
	if (!p) {
		return NULL;
	}

	memset(&data, 0, sizeof(data));
	data.cfg = calloc(1, sizeof(*data.cfg));

	XML_SetUserData(p, &data);
	XML_SetElementHandler(p, config_start, config_end);
	eof = 0;

	while (!eof) {
		len = fread(buf, 1, sizeof(buf), f);
		if (ferror(f)) {
			perror("Failed to read mixer paths");
			goto put_parser;
		}
		eof = feof(f);

		if (XML_Parse(p, buf, len, eof) == XML_STATUS_ERROR) {
			fprintf(stderr, "Parse error at line %u:\n%s\n",
					XML_GetCurrentLineNumber(p),
					XML_ErrorString(XML_GetErrorCode(p)));
			goto put_parser;
		}
	}

put_parser:
	XML_ParserFree(p);
put_file:
	fclose(f);
	return data.cfg;
}

#ifdef DEBUG

static void dump_widgets(node_t *list) {
	node_t *n = NULL;
	struct widget *wid = NULL;
	while ((n = listNext(list, n)) != NULL) {
		wid = listGet(n);
		printf("\t%s => ", wid->name);
		if (wid->strval)
			printf("%s\n", wid->strval);
		else
			printf("%d\n", wid->intval);
	}
}

static void config_dump(struct mixer_config *cfg) {
	node_t *n = NULL;
	struct mixer_setting *set;

	printf("Default mixer paths:\n");
	dump_widgets(cfg->default_widgets);

	while ((n = listNext(cfg->settings, n)) != NULL) {
		set = listGet(n);
		printf("Path '%s' ENABLE:\n", set->name);
		dump_widgets(set->en_widgets);
		printf("Path '%s' DISABLE:\n", set->name);
		dump_widgets(set->dis_widgets);
	}
}

int main(int argc, char **argv) {
	if (argc == 1) return 1;

	config_dump(parse_config(argv[1]));
	return 0;
}
#endif
