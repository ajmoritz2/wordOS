#include "../kernel/kernel.h"
#include "keyboard.h"
#include "apic.h"
#include "../programs/terminal.h"
#include "../memory/heap.h"
#include "../memory/string.h"
#include <stdint.h>


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

kernel_scancode extended_key_lookup[] = {

}; // To extend onto
  

uint8_t *key_codes;
key_event key_buffer[MAX_KEY_BUFFER_SIZE];
uint8_t key_buffer_pos = 0;

static uint8_t current_masks = 0;

uint8_t handle_masks(kernel_scancode key)
{
	// TODO: Make the masks global because shift should stay shift until released!
	switch (key) {
		case KEY_ALT:
			key_buffer[key_buffer_pos].masks |= ALT_MASK;
			return 1;
		case KEY_CTRL:
			key_buffer[key_buffer_pos].masks |= CTRL_MASK;
			return 1;
		case KEY_SHIFT:
			key_buffer[key_buffer_pos].masks |= SHIFT_MASK;
			return 1;
		default:
			return 0;
	}		
}

void handle_keychange()
{

	/* This handles SCANCODE SET 1 ONLY
	 *
	 * Key releases start at 0x80 and the correpsonding key is CODE - 0x80
	 * The extend bit really means nothing
	 * 	
	 */

	memset(key_codes, 0, 4); // Must change this too if we have more bytes (we wont.)
	int bytes_in = 0;
	for (; bytes_in < 4; bytes_in++) {
		uint8_t code = inportb(0x60);
		key_codes[bytes_in] = code;
		if (code != 0xE0)
			break;
	}

	key_buffer_pos = (key_buffer_pos + 1) % MAX_KEY_BUFFER_SIZE;
	key_buffer[key_buffer_pos].masks = 0;	
	key_buffer[key_buffer_pos].code = key_lookup[key_codes[bytes_in] % 0x80];
	
	if (key_codes[bytes_in] > 0x80) {
		key_buffer[key_buffer_pos].masks |= RELEASE_MASK;	
	}

	if (key_lookup[key_codes[bytes_in] % 0x80] == KEY_SHIFT) {
		if (key_buffer[key_buffer_pos].masks & RELEASE_MASK)
			current_masks &= ~SHIFT_MASK;
		else
			current_masks |= SHIFT_MASK;
	}

	key_buffer[key_buffer_pos].masks |= current_masks;	

	// We will have the program handle shifts and whatnot. Not my problem here.
}

void init_keyboard()
{
	outportb(0x64, 0x20);
	uint8_t trans = inportb(0x60);
	io_wait();
	logf("Trans bit %x\n", trans);

	outportb(0x60, 0xF0); // Check the scancode set
	io_wait();
	outportb(0x60, 2);
	io_wait();
	uint8_t ack = inportb(0x60);
	io_wait();
	uint8_t set = inportb(0x60);
	printf("Recieved: %x | Scancode set:  %x\n", ack, set); // Uhh this is wrong
															// The keyboard may use scancode set 2 but it gets translated into 1
	// for now Im going to leave it that way, but in the future I should really just remove the translation

	if (!(trans >> 6) && set != 0x43)
		panic("Keyboard type not supported! Sowwy :3");
	else
		logf("Keyboard type supported! Continue!\n");

	key_codes = kalloc(4); // 4 bytes of data ig
	uint8_t keyboard_reg = 0x12;	
	uint32_t ioapic_data = 50; // 50 is the vector
	write_ioapic_register(keyboard_reg, ioapic_data); // We set up the APIC keyboard reg before init keyboard IN KERNELC
	printf("Keyboard Function: %t30OK!%t10\n");
}
