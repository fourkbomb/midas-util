
CC := $(CROSS_COMPILE)gcc
CFLAGS := -g -Wall -Werror -Ilibufdt/include -I.
LINKER_DIR ?= /usr/lib

inih/libinih.a:
	$(MAKE) -C inih

libufdt/libufdt.so:
	$(MAKE) -C libufdt libufdt.so

boot: config.o gpio.o main.c llist.o ufdt.o util.o inih/libinih.a libufdt/libufdt.so
	$(CC) $(CFLAGS) -o $@ main.c config.o gpio.o llist.o ufdt.o util.o inih/libinih.a -L$(LINKER_DIR) -l:libfdt.so -Llibufdt -l:libufdt.so

config.o: config.c config.h
	$(CC) $(CFLAGS) -c -o $@ config.c

gpio.o: gpio.c gpio.h
	$(CC) $(CFLAGS) -c -o $@ gpio.c

llist.o: llist.c llist.h
	$(CC) $(CFLAGS) -c -o $@ llist.c

ufdt.o: ufdt.c ufdt.h
	$(CC) $(CFLAGS) -c -o $@ ufdt.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c -o $@ util.c

clean:
	rm -f *.o boot
	$(MAKE) -C inih clean
	$(MAKE) -C libufdt clean
