PWD := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
CC ?= $(CROSS_COMPILE)gcc
LOCAL_CFLAGS = -Wall -Werror -I./include -fPIC
SYSDEP_FILES = $(PWD)/libufdt_sysdeps_posix.c

libufdt_sysdeps.a: libufdt_sysdeps_posix.o
	ar rcs $@ $^

libufdt_sysdeps_posix.o: $(SYSDEP_FILES)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

clean:
	rm -f libufdt_sysdeps.a libufdt_sysdeps_posix.o
