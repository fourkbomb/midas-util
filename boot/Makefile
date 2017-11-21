
CC := $(CROSS_COMPILE)gcc
CFLAGS := -g -Wall -Werror -Ilibufdt/include

inih/libinih.a:
	$(MAKE) -C inih

libufdt/libufdt.so:
	$(MAKE) -C libufdt

config: gpio.c llist.c config.c inih/libinih.a
	$(CC) $(CFLAGS) -DCONFIG_TEST -o $@ $^
