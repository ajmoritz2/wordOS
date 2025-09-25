/*
 *	Terminal Program
 *	07/07/25
 *
 *	This is the actual terminal of the OS. Yea
 *
 */
#include "../kernel/kernel.h"
#include "../kernel/scheduler.h"
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
char *user_chars;
uint16_t user_char_index = 2;
uint16_t textbuf_loc = 0;
uint16_t cur_lines = 0;
uint8_t term_ready = 0;

int max_char_x, max_char_y;

static uint32_t term_bg = 0;
static uint32_t term_fg = 0xf0f0f000;

char keycode_to_keysym(key_event key)
{
	uint8_t shifted = key.masks & SHIFT_MASK;
	switch(key.code) {
	case KEY_A:
		return shifted ? 'A' : 'a';
	case KEY_B:
		return shifted ? 'B': 'b';
	case KEY_C:
		return shifted ? 'C' : 'c';
	case KEY_D:
		return shifted ? 'D' : 'd';
	case KEY_E:
		return shifted ? 'E' : 'e';
	case KEY_F:
		return shifted ? 'F' : 'f';
	case KEY_G:
		return shifted ? 'G' : 'g';
	case KEY_H:
		return shifted ? 'H' : 'h';
	case KEY_I:
		return shifted ? 'I' : 'i';
	case KEY_J:
		return shifted ? 'J' : 'j';
	case KEY_K:
		return shifted ? 'K' : 'k';
	case KEY_L:
		return shifted ? 'L' : 'l';
	case KEY_M:
		return shifted ? 'M' : 'm';
	case KEY_N:
		return shifted ? 'N' : 'n';
	case KEY_O:
		return shifted ? 'O' : 'o';
	case KEY_P:
		return shifted ? 'P' : 'p';
	case KEY_Q:
		return shifted ? 'Q' : 'q';
	case KEY_R:
		return shifted ? 'R' : 'r';
	case KEY_S:
		return shifted ? 'S' : 's';
	case KEY_T:
		return shifted ? 'T' : 't';
	case KEY_U:
		return shifted ? 'U' : 'u';
	case KEY_V:
		return shifted ? 'V' : 'v';
	case KEY_W:
		return shifted ? 'W' : 'w';
	case KEY_X:
		return shifted ? 'X' : 'x';
	case KEY_Y:
		return shifted ? 'Y' : 'y';
	case KEY_Z:
		return shifted ? 'Z' : 'z';
	case KEY_1:
		return shifted ? '!' : '1';
	case KEY_2:
		return shifted ? '@' : '2';
	case KEY_3:
		return shifted ? '#' : '3';
	case KEY_4:
		return shifted ? '$' : '4';
	case KEY_5:
		return shifted ? '%' : '5';
	case KEY_6:
		return shifted ? '^' : '6';
	case KEY_7:
		return shifted ? '&' : '7';
	case KEY_8:
		return shifted ? '*' : '8';
	case KEY_9:
		return shifted ? '(' : '9';
	case KEY_0:
		return shifted ? ')' : '0';
	case KEY_LBRACKET:
		return shifted ? '{' : '[';
	case KEY_RBRACKET:
		return shifted ? '}' : ']';
	case KEY_FSLASH:
		return shifted ? '?' : '/';
	case KEY_BSLASH:
		return shifted ? '|' : '\\';
	case KEY_SINGLE_QUOTE:
		return shifted ? '"' : '\'';
	case KEY_SEMI_COLON:
		return shifted ? ':' : ';';
	case KEY_PERIOD:
		return shifted ? '>' : '.';
	case KEY_COMMA:
		return shifted ? '<' : ',';
	case KEY_EQUALS:
		return shifted ? '+' : '=';
	case KEY_MINUS:
		return shifted ? '_' : '-';
	case KEY_BACKTICK:
		return shifted ? '~' : '`';
	case KEY_SPACE:
		return ' ';
	case KEY_TAB:
		return '\t';
	case KEY_BACKSPACE:
		return 0x8;
	case KEY_RETURN:
		return '\n';
	default:
		return 0;
	}
}

void scroll_text_buffer_down()
{
	// Get the end of the first line
	int end_first_line = 0;
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

	logf("cur_letter: %x\n", cur_letter);
	// Reset the final line to all 0
	for(int i = 0; text_buffer[cur_letter + i]; i++) {
		text_buffer[cur_letter + i] = 0;
	}
	cur_lines--;

	textbuf_loc -= end_first_line;
}

uint8_t is_term_ready()
{
	return term_ready;
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
	if (cur_lines >= max_char_y)
		scroll_text_buffer_down();	
	text_buffer[textbuf_loc] = ch;
	textbuf_loc++;
}

uint32_t single_shtoi(char num)
{

	if (num >= '0' && num <= '9')
		return num - '0';
	if (num >= 'a' && num <= 'f')
		return 10 + (num - 'a');

	return -1;
}


void parse_color(char *string, int loc)
{
	// 8 bit color. first 4 bits are fg second 4 are bg
	// Black, White, Red, Green, Blue, Yellow, Magenta, Cyan
	uint32_t colors[16] = {0x0, 0xffffff00, 0xff000000, 0x00ff0000, 0x0000ff00,
						0xc4a00000, 0x75507b00, 0x06989a00};	
	loc++;

	uint32_t hex_fg = single_shtoi(string[loc]);
	loc++;
	uint32_t hex_bg = single_shtoi(string[loc]);


	term_fg = colors[hex_fg];
	term_bg = colors[hex_bg];
}

void draw_text_buffer()
{
	uint16_t print_loc = 0;
	uint16_t line_x = 0;
	uint16_t line_num = 0;
	uint8_t font_width = font_size * 8;
	uint8_t font_height = font_size * 16;

	while (text_buffer[print_loc] != 0) {
		if (text_buffer[print_loc] == 0xffffffff) { // The fucking compiler thinks this is what I mean when I put 0xff
			parse_color(text_buffer, print_loc);
			print_loc += 3;
			continue;
		}
		if (text_buffer[print_loc] == '\n') {
			for (int i = line_x; i < max_char_x; i++)
				fb_put_glyph(' ', i * font_width, line_num * font_height, term_fg, term_bg, font_size);
			line_num++;
			print_loc++;
			line_x = 0;
			continue;
		}
		if (line_x % max_char_x == 0 && line_x > 0) {
			line_num++;
			line_x = 0;
		}

		fb_put_glyph(text_buffer[print_loc], line_x * font_width, line_num * font_height, term_fg, term_bg, font_size);
		print_loc++;
		line_x++;
	}

	for (int i = 0; i < max_char_x; i++) {
		fb_put_glyph(' ', i * font_width, line_num * font_height, term_fg, term_bg, font_size);
		if (i > 1)
			user_chars[i] = 0;	
	}
}

void draw_user_chars()
{
	uint8_t font_width = font_size * 8;
	uint8_t font_height = font_size * 16;
	int index = 0;
	while (user_chars[index]) {
		fb_put_glyph(user_chars[index], index * font_width, cur_lines * font_height, term_fg, term_bg, font_size);
		index++;
	}
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
					write_char_text_buffer((char)0xff);
					string++;
					continue;
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
	draw_user_chars();
}

void write_user_char(char ch)
{
	if (ch == 0x8) {
		if (user_char_index <= 2)
			return;
		user_char_index--;
		user_chars[user_char_index] = ' ';	
		return;
	}

	if (user_char_index == max_char_x)
	   return;
	user_chars[user_char_index] = ch;	
	text_buffer[textbuf_loc + user_char_index] = ch;
	user_char_index++;
}

void tflush()
{
	draw_text_buffer();
}

void bark_process()
{
	for (int i = 0; i < 10; i++)
		printf("Bark\n");

	kill_current_process();
}

void parse_user_chars()
{
	char parse_index = 2;
	char *command = (char *) kalloc(max_char_x);
	logf("Aquired command at %x\n", command);

	while (user_chars[parse_index]) {
		if (user_chars[parse_index] == ' ')
			break;
		if (user_chars[parse_index] == '\n')
			break;

		command[parse_index-2] = user_chars[parse_index];
		parse_index++;
	}

	// TODO: Make these run the script with said name
	if (strcmp("version", command, strlen("version"))){
		printf("WordOS kernel version: %t30%s%t10\n", KERNEL_VERSION);
	} else if (strcmp("woof", command, strlen("woof"))){
		printf("%t54Bark! :3%t10\n");
		create_process("Bark", &bark_process, 0, 0);
	} else {
		printf("Command %t20%s%t10 unknown.\n", command);
	}

	kfree(command);
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

	while (1) {
		key_event event = next_keycode();
		if (event.code == KEY_NULL) {
			asm("hlt");

			continue;
		}


		char keysym = keycode_to_keysym(event);
		if (event.masks & RELEASE_MASK) keysym = 0;

		char *test = (char *)&event;
		//logf("Event masks %x\n", *(test));
		if (keysym) {
			write_user_char(keysym);
			if (keysym == '\n') {
				write_text_buffer(user_chars);
				parse_user_chars();
				user_char_index = 2;
				tflush();	
			}
			draw_user_chars();
		}
	}
}

void start_terminal()
{
	process_t *term_process = create_process("Terminal", &terminal_loop, 0, 0);
	logf("Created terminal process\n");
	draw_user_chars();
}

void init_terminal()
{
	// Font characters are 8x16 px default
	max_char_x = (screen_width / (8 * font_size)) - 1;
	max_char_y = (screen_height / (16 * font_size)) - 2;

	text_buffer = (char *)kalloc((max_char_x * max_char_y) + 1);
	memset(text_buffer, 0, max_char_x * max_char_y + 1);
	user_chars = (char *) kalloc(max_char_x + 1);
	user_chars[0] = '>';
	user_chars[1] = ' ';

	int past_buf_loc = textbuf_loc;

	printf("Terminal function: %t70OK!%t10\n");
	tflush();
	term_ready = 1;
}
