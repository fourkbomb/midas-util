CC ?= $(CROSS_COMPILE)gcc
LOCAL_CFLAGS := -g -Wall -Werror -I./libufdt/include -I./libufdt/sysdeps/include -I.

bootloader: config.o gpio.o main.o llist.o ufdt.o util.o ini.o libufdt_sysdeps_posix.o ufdt_convert.o ufdt_node.o ufdt_node_pool.o ufdt_overlay.o ufdt_prop_dict.o
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ main.o config.o gpio.o llist.o ufdt.o util.o ini.o libufdt_sysdeps_posix.o ufdt_convert.o ufdt_node.o ufdt_node_pool.o ufdt_overlay.o ufdt_prop_dict.o -lfdt

main.o: main.c config.h ufdt.h util.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ main.c

config.o: config.c config.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ config.c

gpio.o: gpio.c gpio.h config.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ gpio.c

llist.o: llist.c llist.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ llist.c

ufdt.o: ufdt.c ufdt.h gpio.h util.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ ufdt.c

util.o: util.c util.h config.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ util.c

ini.o: inih/ini.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ inih/ini.c

libufdt_sysdeps_posix.o: libufdt/sysdeps/libufdt_sysdeps_posix.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/sysdeps/libufdt_sysdeps_posix.c

ufdt_convert.o: libufdt/ufdt_convert.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/ufdt_convert.c

ufdt_node.o: libufdt/ufdt_node.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/ufdt_node.c

ufdt_node_pool.o: libufdt/ufdt_node_pool.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/ufdt_node_pool.c

ufdt_overlay.o: libufdt/ufdt_overlay.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/ufdt_overlay.c

ufdt_prop_dict.o: libufdt/ufdt_prop_dict.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ libufdt/ufdt_prop_dict.c

clean:
	rm -f *.o bootloader
