
CROSS_COMPILE = arm-linux-
CC = $(CROSS_COMPILE)gcc

$(shell mkdir -p bin)

all: getdtb

getdtb: getdtb.c
	$(CC) -o bin/getdtb getdtb.c
