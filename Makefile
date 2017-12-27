CROSS_COMPILE = arm-linux-
CC ?= $(CROSS_COMPILE)gcc

OBJDIR = bin

all: $(OBJDIR) $(OBJDIR)/bootloader $(OBJDIR)/getdtb $(OBJDIR)/reboot $(OBJDIR)/blank $(OBJDIR)/gpioutil

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/blank: blank.c
	$(CC) -o $(OBJDIR)/blank blank.c

$(OBJDIR)/reboot: reboot.c
	$(CC) -o $(OBJDIR)/reboot reboot.c

$(OBJDIR)/getdtb: getdtb.c
	$(CC) -o $(OBJDIR)/getdtb getdtb.c

$(OBJDIR)/bootloader:
	$(MAKE) -C boot bootloader
	mv boot/bootloader $(OBJDIR)/

$(OBJDIR)/gpioutil: gpioutil.c
	$(CC) -o $(OBJDIR)/gpioutil gpioutil.c -static

clean:
	rm -rf $(OBJDIR)
	$(MAKE) -C boot clean
