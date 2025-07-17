/*
 *	Terminal Program
 *	07/07/25
 *
 *	This is the actual terminal of the OS. Yea
 *
 */
#include "../kernel/kernel.h"
#include "../drivers/framebuffer.h"
#include "terminal.h"
#include "../drivers/keyboard.h"
#include "../memory/heap.h"
#include "../memory/string.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

uint8_t key_buffer_tail = 0;

uint8_t font_size = 1;

char *text_buffer;
uint16_t textbuf_loc = 0;
uint16_t cur_lines = 0;

int max_char_x, max_char_y;

uint32_t term_bg = 0;
uint32_t term_fg = 0xf0f0f000;



void scroll_text_buffer_down()
{
	// Get the end of the first line
	int end_first_line = max_char_x;
	for (; end_first_line < max_char_x || text_buffer[end_first_line] != '\n'; end_first_line++);
	for (int i = 0; i < max_char_x; i++) {
		if (text_buffer[i] == '\n') {
			end_first_line = i + 1;
			break;
		}
	}
	logf("End first line %d\n", end_first_line);

	// Move all characters backwards by first line length
	int cur_letter = 0;
	while (cur_letter + end_first_line < max_char_x * max_char_y)
	{
		if (text_buffer[cur_letter + end_first_line] == 0)
			break;
		text_buffer[cur_letter] = text_buffer[cur_letter + end_first_line];

		cur_letter++;
	}

	// Reset the final line to all 0
	for(int i = 0; text_buffer[cur_letter + i]; i++) {
		text_buffer[cur_letter + i] = 0;
	}
	cur_lines--;

	textbuf_loc -= end_first_line;
}

void write_text_buffer(char *text)
{
	int max_characters = max_char_x * max_char_y;
	while (*text) {
		if (*text == '\n')
			cur_lines++;
		if (cur_lines > max_char_y)
			scroll_text_buffer_down();	
		text_buffer[textbuf_loc] = *text;
		text++;
		textbuf_loc++;
	}
}

void write_char_text_buffer(char ch)
{

	int max_characters = max_char_x * max_char_y;
	if (ch == '\n')
		cur_lines++;
	if (cur_lines > max_char_y)
		scroll_text_buffer_down();	
	text_buffer[textbuf_loc] = ch;
	textbuf_loc++;
}

void draw_text_buffer()
{
	uint16_t print_loc = 0;
	uint16_t line_x = 0;
	uint16_t line_num = 0;
	uint8_t font_width = font_size * 8;
	uint8_t font_height = font_size * 8;

	while (text_buffer[print_loc] != 0) {
		if (text_buffer[print_loc] == '\n') {
			for (int i = line_x; i < max_char_x; i++)
				fb_put_glyph(' ', i * font_width, line_num * font_height, term_fg, term_bg, font_size);
			line_num++;
			print_loc++;
			line_x = 0;
			continue;
		}
		if (line_x % max_char_x == 0) {
			line_num++;
			line_x = 0;
		}

		fb_put_glyph(text_buffer[print_loc], line_x * font_width, line_num * font_height, term_fg, term_bg, font_size);
		print_loc++;
		line_x++;
	}
}

uint32_t single_shtoi(char num)
{

	if (num >= '0' && num <= '9')
		return num - '0';
	if (num >= 'a' && num <= 'f')
		return 10 + (num - 'a');

	return -1;
}


void parse_color(char *string)
{
	// 8 bit color. first 4 bits are fg second 4 are bg
	// Black, White, Red, Green, Blue, Yellow, Magenta, Cyan
	uint32_t colors[16] = {0x0, 0xffffff00, 0xff000000, 0x00ff0000, 0x0000ff00,
						0xc4a00000, 0x75507b00, 0x06989a00};	
	string++;
	uint32_t hex_fg = single_shtoi(*string);
	string++;
	uint32_t hex_bg = single_shtoi(*string);

	term_fg = colors[hex_fg];
	term_bg = colors[hex_bg];
}


void term_render_hex(uint32_t number)
{	
	if (number == 0) { // Hack
		write_text_buffer("0x0");
		return;
	}
	char final[64];
	final[0] = '0';
	final[1] = 'x';
	uint32_t i = 2;
	for (uint32_t num = number; num != 0; num = num/16) {
	 	uint32_t value = num%16;
		if (value < 10) {
			final[i] = '0' + value;
		} else {
			final[i] = 'a' + (value-10);
		}

		i++;
	}
	
	for (uint32_t j = 2, k = i - 1; j <= k; j++, k--) {
		char c = final[k];
		final[k] = final[j];
		final[j] = c;
	}
	final[i] = 0;

	write_text_buffer(final);
}

void printf(char *string, ...)
{
	int size = 1;
	va_list params;
	va_start(params, string);
	logf("PRINTED %s\n", string);
	while (*string) {
		if (*string == '%') {
			string++;
			switch (*string) {
				case 'x':
					term_render_hex(va_arg(params, uint32_t));
					break;
				case '%':
					write_char_text_buffer('%');
					break;
				case 't':
					parse_color(string);
					string += 2;
					break;
				case 's':
					write_text_buffer(va_arg(params, char *));
					break;
				case 'c':
					char character = (char) va_arg(params, uint32_t);
					write_char_text_buffer(character);
					break;

			}

			string++;
			continue;
		}


		write_char_text_buffer(*string);
		if (*string == '\n') 
			draw_text_buffer();
		string++;
	}
}

void tflush()
{
	draw_text_buffer();
}

key_event next_keycode()
{
	if (key_buffer_tail == key_buffer_pos) {
		key_event nuller = {KEY_NULL, 0};
		return nuller;
	}

	key_buffer_tail = (key_buffer_tail + 1)	% MAX_KEY_BUFFER_SIZE;
	key_event to_ret = key_buffer[key_buffer_tail];

	return to_ret;
}

void terminal_loop() 
{

	key_event event = next_keycode();
	if (event.code != KEY_NULL) {
		if (event.masks & RELEASE_MASK) 
			printf("Key %x Released!\n", event.code);
		if (event.code == KEY_H)
			printf("Hello Worldb\n");
	}
}

void init_terminal()
{
	// Font characters are 8x16 px default
	max_char_x = (screen_width / (8 * font_size)) - 1;
	max_char_y = (screen_height / (16 * font_size)) - 1;

	text_buffer = (char *)kalloc(max_char_x * max_char_y);
	int past_buf_loc = textbuf_loc;


	write_text_buffer("Hello World!\n");
	write_text_buffer("Hello World\n");
	write_text_buffer("Hello Worl\n");
	tflush();
}
