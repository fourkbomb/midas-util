CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Werror -Iinclude -Isysdeps/include -fPIC
FILES := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(FILES))
LINKER_DIR ?= /usr/lib
sysdeps/libufdt_sysdeps.a:
	$(MAKE) -C sysdeps

libufdt.so: sysdeps/libufdt_sysdeps.a $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES) -L$(LINKER_DIR) -shared -l:libfdt.so -Lsysdeps -l:libufdt_sysdeps.a

all: libufdt.so

clean_sysdeps:
	$(MAKE) -C sysdeps clean

clean: clean_sysdeps
	rm -f libufdt.so
