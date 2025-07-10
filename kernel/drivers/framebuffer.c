#include <stdint.h>
#include <stdarg.h>
#include "framebuffer.h"
#include "keyboard.h"
#include "../kernel/kernel.h"
#include "../multiboot/mb_parse.h"
#include "../multiboot/multiboot2.h"

#define FONT_SIZE		2
#define MAX_CURSOR_X 	74
#define MAX_CURSOR_Y	37
extern struct multiboot_tag_framebuffer* fb;

uint32_t* fb_virt_addr;

struct psf1_header* font_head;

uint16_t cursor_xpaws = 0;
uint16_t cursor_ypaws = 0; // :3

uint16_t screen_width;
uint16_t screen_height;


void fb_set_width(uint16_t w)
{
	screen_width = w;
}

void fb_set_height(uint16_t h)
{
	screen_height = h;
}

void fb_put_glyph(char glyph, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, uint16_t size)
{
	uint8_t* gaddr = get_glyph(glyph);
	uint8_t fgred = fg >> 24;
	uint8_t fggreen = (fg >> 16) & 255;
	uint8_t fgblue = (fg >> 8) & 255;

	uint8_t bgred = bg >> 24;
	uint8_t bggreen = (bg >> 16) & 255;
	uint8_t bgblue = (bg >> 8) & 255;

	for (int i = 0; i < font_head->charsize; i++) {
		uint32_t* pixel_addr = get_pixel_addr(x, y);
		for(int j = 7; j >= 0; j--) {
			if (*gaddr & 1 << j) {
				fill_square(pixel_addr, size, size, fgred, fggreen, fgblue, 0x0);
			} else {
				fill_square(pixel_addr, size, size, bgred, bggreen, bgblue, 0x0);
			}
			pixel_addr+=size;
		}
		y += size;
		gaddr += 1;
	}
}

uint8_t* get_glyph(int num)
{
	uint8_t* glyph = (uint8_t*) &_binary_font_psf_start + sizeof(struct psf1_header)
		+ (num * font_head->charsize);
	return glyph;
}

void fill_square(uint32_t* addr, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t w) 
{
	uint8_t* pix_loc = (uint8_t*) addr;
	uint8_t pixel_width = 4;

	for  (int yh = 0; yh < height; yh++) {
		for (int xw = 0; xw < width; xw++) {
			pix_loc[pixel_width*xw] = b;
			pix_loc[pixel_width*xw + 1] = g;
			pix_loc[pixel_width*xw + 2] = r;
		}
		pix_loc += fb->common.framebuffer_pitch;
	}
}


void init_font()
{
	font_head = (struct psf1_header*) &_binary_font_psf_start;	

	// Check signature
	if (font_head->magic[0] != PSF_MAGIC0 
			|| font_head->magic[1] != PSF_MAGIC1)
		return;

	logf("font_mode: %x, char_size: %d\n", font_head->mode, font_head->charsize);
}

void put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
	uint32_t* pixel_addr = get_pixel_addr(x, y);

	*pixel_addr = color;	
}
