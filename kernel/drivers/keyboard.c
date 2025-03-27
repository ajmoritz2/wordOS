#include "../kernel/kernel.h"
#include "keyboard.h"
#include "framebuffer.h"
#include <stdint.h>

#define MAX_KEY_BUFFER_SIZE	255

#define NORMAL_STATE	0
#define PREFIX_STATE	1

// TODO: make this more robust, as in actually fix it

kernel_scancode key_lookup[] = {
	0, KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, 			// 5
	KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, 					//10
	KEY_0, KEY_MINUS, KEY_EQUALS, KEY_BACKSPACE, KEY_TAB, //15
	KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, // 20
	KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, // 25
	KEY_LBRACKET, KEY_RBRACKET, KEY_RETURN, KEY_CTRL, KEY_A, // 30
	KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, // 35
	KEY_J, KEY_K, KEY_L, KEY_SEMI_COLON, KEY_SINGLE_QUOTE, //40
	KEY_BACKTICK, KEY_SHIFT, KEY_BSLASH, KEY_Z, KEY_X,	//45
	KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, //50
	KEY_COMMA, KEY_PERIOD, KEY_FSLASH, KEY_SHIFT, 0, // 55
	KEY_ALT, KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F2, // 60
	KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, // 65
	KEY_F8, KEY_F9, KEY_F10, 0, 0, //70
	0, 0, 0, 0, 0, // 75
	0, 0, 0, 0, 0, // 80
	0, 0, 0, 0, 0, // 85
	0, KEY_F11, KEY_F12
};

key_event key_buffer[MAX_KEY_BUFFER_SIZE];
uint8_t buffer_pos = 0;

uint8_t current_state = NORMAL_STATE;

uint8_t handle_masks(kernel_scancode key)
{
	// TODO: Make the masks global because shift should stay shift until released!
	switch (key) {
		case KEY_ALT:
			key_buffer[buffer_pos].masks |= ALT_MASK;
			return 1;
		case KEY_CTRL:
			key_buffer[buffer_pos].masks |= CTRL_MASK;
			return 1;
		case KEY_SHIFT:
			key_buffer[buffer_pos].masks |= SHIFT_MASK;
			return 1;
		default:
			return 0;
	}		
}

void recieve_scancode(uint8_t code)
{
	if (code != 0xE0 && (code > 85))
		return;
	if (current_state == NORMAL_STATE) {
		if (code == 0xE0) {
			current_state = PREFIX_STATE;
			return;
		}	
		if (handle_masks(key_lookup[code]))
			return;

		buffer_pos = (buffer_pos + 1) % MAX_KEY_BUFFER_SIZE;
		key_buffer[buffer_pos].code = key_lookup[code];
	}

	if (current_state == PREFIX_STATE) {
		current_state = NORMAL_STATE;
		kernel_scancode key = key_lookup[code];
		handle_masks(key);
	}
}

char get_printable_char(key_event key)
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

void init_keyboard()
{
	outportb(0x64, 0x20);
	uint8_t trans = inportb(0x60);
	io_wait();
	logf("Trans bit %x\n", trans);

	outportb(0x60, 0xF0); // Check the scancode set
	io_wait();
	outportb(0x60, 0);
	io_wait();
	uint8_t ack = inportb(0x60);
	io_wait();
	uint8_t set = inportb(0x60);
	logf("Recieved: %x | Scancode set:  %x\n", ack, set);

	if (!(trans >> 6) && set != 0x43)
		panic("Keyboard type not supported! Sowwy :3");
	else
		logf("Keyboard type supported! Continue!\n");
}
