#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <png.h>
#include "libfb.h"

const int bar_thick = 24;
const int bar_margin = 16;
const char* logo_file = "logo.png";
struct {
	int w;
	int h;
	int bytes_pp;
	png_bytep* rows;
} logo = {0, 0, 4, NULL};

void progress_bar(unsigned char percent)
{
	int bar_y, bar_width = lfb.width - bar_margin * 2;
	int cur_bar_width = round((bar_width / 100.0) * percent);
	if (cur_bar_width > bar_width)
		cur_bar_width = bar_width;
	bar_y = lfb.height - bar_thick - bar_margin;
	lfb.fillbox(bar_margin - 4, bar_y - 4, bar_width + 8, bar_thick + 8, 0xfb2108);
	lfb.fillbox(bar_margin, bar_y, cur_bar_width, bar_thick, 0x32eb0c);
	lfb.fillbox(bar_margin + cur_bar_width, bar_y, bar_width - cur_bar_width, bar_thick, 0xffffff);
}

int read_png_file(void)
{
	unsigned char header[8];	// 8 is the maximum size that can be checked
	png_structp png_ptr;
	png_infop info_ptr;
	int color;
	int y;

	FILE *fp = fopen(logo_file, "rb");
	if (!fp)	return 3;
	
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))	return 4;

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)	return 5;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)	return 6;
	if (setjmp(png_jmpbuf(png_ptr)))	return 7;

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	logo.w = png_get_image_width(png_ptr, info_ptr);
	logo.h = png_get_image_height(png_ptr, info_ptr);
	color = png_get_color_type(png_ptr, info_ptr);
	if(color == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))	return 8;

	logo.rows = (png_bytep*) malloc(sizeof(png_bytep) * logo.h);
	for (y=0; y < logo.h; y++)
		logo.rows[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, logo.rows);
	if (color == PNG_COLOR_TYPE_RGB)
		logo.bytes_pp = 3;
	else if (color != PNG_COLOR_TYPE_RGBA)
		{printf("Image should be in type RGB/RGBA.\n"); return 9;}

	fclose(fp);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	int i, x, y, retval = 0;
	int logo_x, logo_y;

#ifndef DEBUG
	int splash_id = fork();
	if (splash_id) {	/* Executes original init with pid=1 */
		argv[0] = "/sbin/init";
		execv("/sbin/init", "/sbin/init");
	}
#endif

	retval = read_png_file();
	if (retval != EXIT_SUCCESS)
		return retval;
	system("/bin/echo -e \"\\033[?25l\"");
	lfb_init();
	lfb.fillscr(0xffffff);

	logo_x = (lfb.width - logo.w)/2;
	logo_y = (lfb.height - logo.h)/2;
	for (y = 0; y < logo.h; y++) {
		png_byte* row = logo.rows[y];
		for (x = 0; x < logo.w; x++) {
			png_byte* ptr = &(row[x * logo.bytes_pp]);
			lfb.putpixel(logo_x + x, logo_y + y, ptr[0] << 16 | ptr[1] << 8 | ptr[2]);
		}
	}
	for (i = 0; i< 5; i++) {
		progress_bar(i);lfb.refresh();sleep(1);
	}
	for (i = 95; i< 101; i++) {
		progress_bar(i);lfb.refresh();sleep(1);
	}
	system("/bin/echo -e \"\\033[?25h\";clear");
	return EXIT_SUCCESS;
}

