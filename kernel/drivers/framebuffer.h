#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

#define PSF_MAGIC0 0x36
#define PSF_MAGIC1 0x04

#define PSF1_MODE512    0x01
#define PSF1_MODEHASTAB 0x02
#define PSF1_MODEHASSEQ 0x04
#define PSF1_MAXMODE    0x05

#define PSF1_SEPARATOR  0xFFFF
#define PSF1_STARTSEQ   0xFFFE

struct psf1_header {
        unsigned char magic[2];     /* Magic number */
        unsigned char mode;         /* PSF font mode */
        unsigned char charsize;     /* Character size */
} __attribute__((packed));

extern char _binary_font_psf_start;
extern char _binary_font_psf_end;
extern char _binary_font_psf_size;

extern uint16_t screen_width;
extern uint16_t screen_height;


void fb_set_width(uint16_t w);
void fb_set_height(uint16_t h);
uint16_t fb_get_width();
uint16_t fb_get_height();
void fb_put_glyph(char glyph, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, uint16_t size);
void fill_square(uint32_t* addr, uint16_t width, uint16_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
void put_pixel(uint32_t x, uint32_t y, uint32_t color);
uint8_t* get_glyph(int num);

void init_font();

#endif
