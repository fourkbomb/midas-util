
CC ?= $(CROSS_COMPILE)gcc
LOCAL_CFLAGS := -g -Wall -Werror -I./libufdt/include -I.

inih/libinih.a:
	$(MAKE) -C inih

libufdt/libufdt.so:
	$(MAKE) -C libufdt libufdt.so

boot: config.o bootimage.o gpio.o kernel.o main.c llist.o ufdt.o util.o inih/libinih.a libufdt/libufdt.so
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -o $@ main.c bootimage.o config.o gpio.o kernel.o llist.o ufdt.o util.o inih/libinih.a -l:libfdt.so -Llibufdt -l:libufdt.so

bootimage.o: bootimage.c bootimage.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ bootimage.c

config.o: config.c config.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ config.c

gpio.o: gpio.c gpio.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ gpio.c

kernel.o: kernel.c kernel.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ kernel.c

llist.o: llist.c llist.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ llist.c

ufdt.o: ufdt.c ufdt.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ ufdt.c

util.o: util.c util.h
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ util.c

clean:
	rm -f *.o boot
	$(MAKE) -C inih clean
	$(MAKE) -C libufdt clean
