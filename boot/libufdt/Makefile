CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Werror -Iinclude -Isysdeps/include
FILES := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(FILES))

sysdeps/libufdt_sysdeps.a:
	$(MAKE) -C sysdeps

libufdt.so: sysdeps/libufdt_sysdeps.a $(FILES)
	$(CC) $(CFLAGS) -o $@ $< -shared

all: libufdt.so

clean_sysdeps:
	$(MAKE) -C sysdeps clean

clean: clean_sysdeps
	rm -f libufdt.so
