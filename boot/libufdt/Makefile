CC ?= $(CROSS_COMPILE)gcc
LOCAL_CFLAGS = -Wall -Werror -Iinclude -Isysdeps/include -fPIC
FILES := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(FILES))

sysdeps/libufdt_sysdeps.a:
	$(MAKE) -C sysdeps

libufdt.so: sysdeps/libufdt_sysdeps.a $(FILES)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ $(FILES) -shared -l:libfdt.so -L./sysdeps -l:libufdt_sysdeps.a

all: libufdt.so

clean_sysdeps:
	$(MAKE) -C sysdeps clean

clean: clean_sysdeps
	rm -f libufdt.so
