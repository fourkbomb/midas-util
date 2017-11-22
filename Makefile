
CROSS_COMPILE = arm-linux-
CC = $(CROSS_COMPILE)gcc

$(shell mkdir -p out/bin out/lib)

all: bootloader getdtb reboot blank

blank: blank.c
	$(CC) -o blank blank.c

reboot: reboot.c
	$(CC) -o reboot reboot.c

getdtb: getdtb.c
	$(CC) -o getdtb getdtb.c

bootloader:
	$(MAKE) -C boot boot

install: all
	cp blank out/bin
	cp reboot out/bin
	cp getdtb out/bin
	cp boot/boot out/bin/bootloader
	cp boot/libufdt/libufdt.so out/lib/

clean:
	rm -f bin/*
	$(MAKE) -C boot clean
