#include <stdbool.h>

#ifndef CMDLINE_H
#define CMDLINE_H

char *cmdline_get_value(char *key);
bool cmdline_has_key(char *key);
bool cmdline_check_value(char *key, char *val);
char *get_bootloader(void);
#endif
