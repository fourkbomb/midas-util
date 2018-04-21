
CC ?= $(CROSS_COMPILE)gcc
LOCAL_CFLAGS := -g -Wall -Werror -I./libufdt/include -I.

SRCS := $(filter-out main.c,$(wildcard *.c))
OBJS := $(patsubst %.c,%.o,$(SRCS))

inih/libinih.a:
	$(MAKE) -C inih

libufdt/libufdt.so:
	$(MAKE) -C libufdt libufdt.so

boot: $(OBJS) main.c inih/libinih.a libufdt/libufdt.so
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ inih/libinih.a -l:libfdt.so -Llibufdt -l:libufdt.so $^

%.o: %.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

clean:
	rm -f *.o boot
	$(MAKE) -C inih clean
	$(MAKE) -C libufdt clean
