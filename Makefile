CROSS_COMPILE = arm-linux-
CC ?= $(CROSS_COMPILE)gcc

OBJDIR = bin

all: bootloader getdtb reboot blank gpioutil

install: $(OBJDIR) all
	mv blank $(OBJDIR)/
	mv reboot $(OBJDIR)/
	mv getdtb $(OBJDIR)/
	mv gpioutil $(OBJDIR)/
	mv boot/bootloader $(OBJDIR)/

$(OBJDIR):
	mkdir -p $(OBJDIR)

blank: blank.c
	$(CC) -o blank blank.c

reboot: reboot.c
	$(CC) -o reboot reboot.c

getdtb: getdtb.c
	$(CC) -o getdtb getdtb.c

gpioutil: gpioutil.c
	$(CC) -o gpioutil gpioutil.c -static

bootloader:
	$(MAKE) -C boot bootloader

clean:
	rm -rf $(OBJDIR) blank reboot getdtb gpioutil
	$(MAKE) -C boot clean
