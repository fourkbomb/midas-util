
CROSS_COMPILE = arm-linux-
CC ?= $(CROSS_COMPILE)gcc

$(shell mkdir -p out/bin out/lib)

all: bootloader getdtb reboot blank gpioutil mixer

blank: blank.c
	$(CC) -o blank blank.c

reboot: reboot.c
	$(CC) -o reboot reboot.c

getdtb: getdtb.c
	$(CC) -o getdtb getdtb.c

bootloader:
	$(MAKE) -C boot boot

gpioutil: gpioutil.c
	$(CC) -o gpioutil gpioutil.c -static

mixer:
	$(MAKE) -C audio mixer

install: all
	cp blank out/bin
	cp reboot out/bin
	cp getdtb out/bin
	cp gpioutil out/bin
	cp boot/boot out/bin/bootloader
	cp boot/libufdt/libufdt.so out/lib/
	cp audio/mixer out/bin/mixer

clean:
	rm -rf out/* getdtb reboot blank gpioutil
	$(MAKE) -C boot clean
