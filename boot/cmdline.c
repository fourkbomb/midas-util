/*
 * Copyright (C) 2017-2018 Simon Shields <simon@lineageos.org>
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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define BL_NAME_LEN (sizeof("I9300") - 1)
char cmdline_buf[BUF_SIZE] = {0};

#ifdef CMDLINE_TEST
char *CMDLINE_PATH = "/proc/cmdline";
#else
#define CMDLINE_PATH "/proc/cmdline"
#endif

static int populate_cmdline_buf(void){
	FILE *fp;
	char *s;
	if (cmdline_buf[0] != 0)
		return 0;

	fp = fopen(CMDLINE_PATH, "r");
	if (!fp) {
		perror("Open /proc/cmdline");
		return -1;
	}

	s = fgets(cmdline_buf, BUF_SIZE, fp);
	if (!s) {
		perror("fgets failed");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

static char *cmdline_find_key(char *key, bool need_value) {
	char *ret, *buf;
	int klen;
	if (populate_cmdline_buf() < 0)
		return NULL;

	ret = cmdline_buf;
	klen = strlen(key);
	// following '=', null terminator
	buf = calloc(klen + 2, sizeof(char));
	strcat(buf, key);
	if (need_value)
		strcat(buf, "=");

	do {
		ret = strstr(ret, buf);
		if (ret == NULL)
			break;
		else if (ret > cmdline_buf && ret[-1] != ' ')
			ret++; // don't get stuck on the same option forever
		else if (ret == cmdline_buf || ret[-1] == ' ') {
			if (!need_value) {
				if (isspace(ret[klen]) || ret[klen] == '=' || ret[klen] == 0)
					break;
			} else
				break;
			printf("%d\n", ret[klen]);
			ret++;
		}

		if (ret >= (cmdline_buf + BUF_SIZE))
			break;

	} while (1);

	free(buf);

	if (ret == NULL) {
		return NULL;
	}

	return ret;
}

char *cmdline_get_value(char *key) {
	char *ret = cmdline_find_key(key, true);
	char *space;
	int klen;

	if (!ret)
		return NULL;

	klen = strlen(key);
	// + 1 for the equals sign.
	ret = &ret[klen+1];
	space = strstr(ret, " ");
	if (space)
		space[0] = 0;
	ret = strdup(ret);
	if (space)
		space[0] = ' ';

	return ret;
}

bool cmdline_has_key(char *key) {
	char *ret = cmdline_find_key(key, false);
	if (ret) {
		return true;
	}
	return false;
}

bool cmdline_check_value(char *key, char *val) {
	char *ret = cmdline_get_value(key);
	if (!ret)
		return false;

	if (!strcmp(ret, val)) {
		free(ret);
		return true;
	}

	free(ret);
	return false;
}

char *get_bootloader(void) {
	char *bl_ver = cmdline_get_value("androidboot.bootloader");
	if (!bl_ver)
		return NULL;
	bl_ver[BL_NAME_LEN] = 0;
	return bl_ver;
}

#ifdef CMDLINE_TEST
int main(int argc, char *argv[]) {
	if (argc == 1)
		return 1;
	CMDLINE_PATH = argv[1];
	char *val = get_bootloader();
	if (!val)
		printf("NULL\n");
	else
		printf("%s\n", val);
	if (cmdline_has_key("androidboot.bootloader") && cmdline_has_key("blah") && cmdline_has_key("android"))
		printf("ok\n");
	return 0;
}
#endif
