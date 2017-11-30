#include <stdio.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct bank {
	const char *name;
	uintptr_t base;
	int npins;
};

#define NR_BANKS 44

#define BANK1(x) 0x11400000 + (x)
#define BANK2(x) 0x11000000 + (x)
#define BANK4(x) 0x106e0000 + (x)
#define B1(_name, _base, _npins) { .name = _name, .base = BANK1(_base), .npins = _npins }
#define B2(_name, _base, _npins) { .name = _name, .base = BANK2(_base), .npins = _npins }
#define B4(_name, _base, _npins) { .name = _name, .base = BANK4(_base), .npins = _npins }

struct bank banks[NR_BANKS] = {
	{
		.name = "gpa0",
		.base = BANK1(0),
		.npins = 8,
	}, {
		.name = "gpa1",
		.base = BANK1(0x20),
		.npins = 6,
	}, {
		.name = "gpb",
		.base = BANK1(0x40),
		.npins = 8,
	}, {
		.name = "gpc0",
		.base = BANK1(0x60),
		.npins = 5,
	}, {
		.name = "gpc1",
		.base = BANK1(0x80),
		.npins = 5,
	}, {
		.name = "gpd0",
		.base = BANK1(0xa0),
		.npins = 4,
	}, {
		.name = "gpd1",
		.base = BANK1(0xc0),
		.npins = 4,
	}, {
		.name = "gpf0",
		.base = BANK1(0x180),
		.npins = 8,
	},
	B1("gpf1", 0x1a0, 8),
	B1("gpf2", 0x1c0, 8),
	B1("gpf3", 0x1e0, 6),
	B1("gpj0", 0x240, 8),
	B1("gpj1", 0x260, 5),

	B2("gpk0", 0x40, 7),
	B2("gpk1", 0x60, 7),
	B2("gpk2", 0x80, 7),
	B2("gpk3", 0xa0, 7),
	B2("gpl0", 0xc0, 7),
	B2("gpl1", 0xe0, 2),
	B2("gpl2", 0x100, 8),
	B2("gpy0", 0x120, 6),
	B2("gpy1", 0x140, 4),
	B2("gpy2", 0x160, 6),
	B2("gpy3", 0x180, 8),
	B2("gpy4", 0x1a0, 8),
	B2("gpy5", 0x1c0, 8),
	B2("gpy6", 0x1e0, 8),
	B2("gpm0", 0x260, 8),
	B2("gpm1", 0x280, 7),
	B2("gpm2", 0x2a0, 5),
	B2("gpm3", 0x2c0, 8),
	B2("gpm4", 0x2e0, 8),

	// ALIVE
	B2("gpx0", 0xc00, 8),
	B2("gpx1", 0xc20, 8),
	B2("gpx2", 0xc40, 8),
	B2("gpx3", 0xc60, 8),

	// bank 3
	{ .name = "gpz", .base = 0x3860000, .npins = 7 },

	B4("gpv0", 0x0, 8),
	B4("gpv1", 0x20, 8),
	B4("gpv2", 0x60, 8),
	B4("gpv3", 0x80, 8),
	B4("gpv4", 0xc0, 2),
	{.name = NULL},

};

struct gpio {
	uint32_t con;
	uint32_t dat;
	uint32_t pud;
	uint32_t drv;
};

struct gpio *read_port(int fd, uintptr_t addr) {
	int sz = 4 * 4; // 4 4-byte registers
	unsigned mapped_size,page_size;
	mapped_size = page_size = getpagesize();
	unsigned offset_in_page = (unsigned)addr & (page_size - 1);

	if (offset_in_page + sz > page_size) {
		mapped_size *= 2;
	}

	void *map_base = mmap(NULL, mapped_size, PROT_READ, MAP_SHARED, fd, addr & ~(off_t)(page_size - 1));
	if (map_base == MAP_FAILED) {
		printf("mmap failed\n");
		return NULL;
	}

	struct gpio *ret = calloc(1, sizeof(struct gpio));

	volatile uint32_t *base = (volatile uint32_t *)((char*)map_base + offset_in_page);

	ret->con = base[0];
	ret->dat = base[1];
	ret->pud = base[2];
	ret->drv = base[3];

	munmap(map_base, mapped_size);

	return ret;
}

int main(int argc, char *argv[]) {
	if (argc == 1) {
		printf("%s <outfile>\n", argv[0]);
		return 1;
	}
	int fd = open("/dev/mem", O_RDONLY | O_SYNC);
	if (fd < 0) {
		printf("/dev/mem\n");
		return 2;
	}
	FILE *outfd = fopen(argv[1], "w");
	if (outfd == NULL) {
		printf("%s\n", argv[1]);
		return 3;
	}
	fprintf(outfd, "  pin  | con | dat | pud | drv\n");

	struct bank *b;
	struct gpio *gpio;
	#define CON(x) ((gpio->con & (0xF << (x*4))) >> (x*4))
	#define DAT(x) ((gpio->dat & (1 << x)) >> x)
	#define PUD(x) ((gpio->pud & (0x3 << (x*2))) >> (x*2))
	#define DRV(x) ((gpio->drv & (0x3 << (x*2))) >> (x*2))
	for (int i = 0; i < NR_BANKS && banks[i].name != NULL; i++) {
		b = &banks[i];
		gpio = read_port(fd, b->base);
		//fprintf(outfd, "%s: 0x%x, con=0x%x, dat=0x%x\n", b->name, b->base, gpio->con, gpio->dat);
		for (int j = 0; j < b->npins; j++) {
			fprintf(outfd, "% 4s-%d |  %d  |  %d  |  %d  |  %d\n", b->name, j, CON(j), DAT(j),
					PUD(j), DRV(j));
		}
		fprintf(outfd, "------------------------------\n");
		free(gpio);
	}
	fclose(outfd);
	close(fd);
	return 0;
}

