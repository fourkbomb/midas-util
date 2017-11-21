
CROSS_COMPILE = arm-linux-
CC = $(CROSS_COMPILE)gcc

$(shell mkdir -p bin)

all: boot bin/getdtb bin/reboot bin/blank

bin/blank: blank.c
	$(CC) -o bin/blank blank.c

bin/reboot: reboot.c
	$(CC) -o bin/reboot reboot.c

bin/getdtb: getdtb.c
	$(CC) -o bin/getdtb getdtb.c

boot:
	$(MAKE) -C boot boot

clean:
	rm -f bin/*
	$(MAKE) -C boot clean
