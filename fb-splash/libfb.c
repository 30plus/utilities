#include "libfb.h"
//#define DELTA(x,y) ((x)>(y)?(x-y):(y-x))
#define DELTA(x,y) ((x) - (y))

int fb;
byte *scr;

void lfb_memset8(void *, unsigned int, size_t);
void lfb_memset32(void *, unsigned int, size_t);
void lfb_set_pixel8(int, Color);
void lfb_set_pixel32(int, Color);
void lfb_put_pixel8(int, int, Color);
void lfb_put_pixel32(int, int, Color);
	
void lfb_fill_scr(Color);
void lfb_exit_error(char *);
void lfb_fill_box(int x, int y, int w, int h, Color);
void lfb_refresh();

struct fb_var_screeninfo fb_var_info;
struct fb_fix_screeninfo fb_fix_info;

void lfb_init()
{	
	fb = open("/dev/fb0", O_RDWR);
	if(fb < 0)
		lfb_exit_error("Can`t open /dev/fb0\n");
	
	ioctl(fb, FBIOGET_VSCREENINFO, &fb_var_info);
	ioctl(fb, FBIOGET_FSCREENINFO, &fb_fix_info);
	
	// set some constants
	lfb.width = fb_var_info.xres;
	lfb.height = fb_var_info.yres;
	lfb.bpp = fb_var_info.bits_per_pixel;
	
	lfb.fillbox = &lfb_fill_box;
	lfb.fillscr = &lfb_fill_scr;
	lfb.refresh = &lfb_refresh;
	
	switch(lfb.bpp){
		case 8:
			lfb.memset = &lfb_memset8;
			lfb.setpixel = &lfb_set_pixel8;
            lfb.putpixel = &lfb_put_pixel8;
			break;
		case 16:
		case 24:
		case 32:
			lfb.memset = &lfb_memset32;
			lfb.setpixel = &lfb_set_pixel32;
            lfb.putpixel = &lfb_put_pixel32;
			break;
	}
	
	scr = (unsigned char *) mmap(0, fb_fix_info.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
	scr+= (fb_var_info.xoffset + fb_var_info.yoffset * fb_var_info.xres_virtual) * (fb_var_info.bits_per_pixel >> 3);
}

void lfb_fill_scr(Color c)
{
	lfb.memset(scr, c, lfb.width * lfb.height );
}

void lfb_fill_box(int x, int y, int w, int h, Color color)
{
	int cy;

	if(x+w > lfb.width) w = lfb.width - x;
	if(y+h > lfb.height) h = lfb.height - y;

	for(cy = y; cy < y + h; cy++)
		lfb.memset(((unsigned int *)scr) + (lfb.width * cy + x), color, w);
}

void lfb_exit_error(char *str)
{
	write(1, str, strlen(str));
	exit(1);
}

void lfb_memset8(void *dst, unsigned int b, size_t len){
	memset(dst, b, len);
}

void lfb_memset32(void *dst, unsigned int b, size_t len){
	wmemset(dst, b, len);
}

void lfb_set_pixel8(int offset, Color c){
	if(offset < 0) offset = 0;
	if(offset > lfb.width * lfb.height) offset = lfb.width * lfb.height;
	*(scr + offset) = (char) c;
}

void lfb_put_pixel8(int x, int y, Color c){
    lfb_set_pixel8(x + lfb.width * y, c);
}

void lfb_set_pixel32(int offset, Color c){
	if(offset < 0) return;
	if(offset > lfb.width * lfb.height) return;
	*(((unsigned int *) scr) + offset) = c;
}

void lfb_put_pixel32(int x, int y, Color c){
    lfb_set_pixel32(x + lfb.width * y, c);
}

void lfb_refresh()
{
	fb_var_info.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
	fb_var_info.yres_virtual = fb_var_info.yres * 2;
	fb_var_info.yoffset = fb_var_info.yres;
	ioctl(fb, FBIOGET_VSCREENINFO, &fb_var_info);	
}
