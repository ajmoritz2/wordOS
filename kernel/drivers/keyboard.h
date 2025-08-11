#ifndef KEYBOARD_H
#define KEYBOARD_H

#define CTRL_MASK		1
#define ALT_MASK		2
#define SHIFT_MASK		4
#define RELEASE_MASK	8

#define MAX_KEY_BUFFER_SIZE	20

extern uint8_t key_buffer_pos;

typedef struct {
	uint8_t code;
	uint8_t masks;
} key_event;

extern key_event key_buffer[];

void handle_keychange();
void init_keyboard();

typedef enum {
	KEY_NULL, KEY_SPACE, KEY_BACKSPACE, KEY_TAB,
	KEY_ESC, KEY_RETURN, KEY_SHIFT, KEY_CTRL, KEY_ALT, KEY_CAPSLOCK,

	// Numbers
	KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,

	// Letters
	KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
	KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
	KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,

	// Function Keys
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8,
	KEY_F9, KEY_F10, KEY_F11, KEY_F12,

	// Symbols
	KEY_EQUALS, KEY_MINUS, KEY_PLUS, KEY_FSLASH, KEY_BSLASH,
   	KEY_RBRACKET,KEY_LBRACKET, KEY_PERIOD, KEY_COMMA, 
	KEY_BACKTICK, KEY_SINGLE_QUOTE, KEY_SEMI_COLON
} kernel_scancode;
#endif
