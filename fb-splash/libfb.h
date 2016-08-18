#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include <time.h>

#define YELLOW	0xFFFF00
#define GRAY	0x888888
#define GREEN	0x00FF00
#define BLUE	0x0000FF
#define RED	0xFF0000
#define WHITE	0xFFFFFF
#define BLACK	0x000000

typedef unsigned char byte;

typedef struct{
	int x;
	int y;
} Point;

typedef int Color;

typedef struct{
	int width;
	int height;
	int bpp;
	char *data;
} Image;

struct{
	int width;
	int height;
	int bpp;
	void (*init)();
	void (*memset)(void *, unsigned int, size_t);
	void (*fillscr)(Color);
	void (*fillbox)(int, int, int, int, Color);
	void (*drawsquare)(Point, int, int, int, Color);
	void (*fillsquare)(Point, int, int, Color);
	Image* (*loadPNG)(int);
	int (*drawimage)(Image *, Point);
	void (*setpixel)(int offset, Color);
	void (*putpixel)(int, int, Color);
	void (*refresh)();
} lfb;

extern byte *src;

void lfb_init();
