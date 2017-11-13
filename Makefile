
CROSS_COMPILE = arm-linux-
CC = $(CROSS_COMPILE)gcc

$(shell mkdir -p bin)

all: getdtb reboot

reboot: reboot.c
	$(CC) -o bin/reboot reboot.c

getdtb: getdtb.c
	$(CC) -o bin/getdtb getdtb.c
