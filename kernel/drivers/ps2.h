/*	PS2_H
 *	Header file for PS2_C
 *
 *	wordOS
 */

#ifndef PS2_H
#define PS2_H
#include <stdint.h>


uint8_t check_status();

void ps2_send_command(uint8_t byte);
void ps2_send_dbyte(uint8_t byte1, uint8_t byte2);
void ps2_send_byte_port1(uint8_t byte);
void ps2_send_byte_port2(uint8_t byte);

uint8_t ps2_get_responce();
uint8_t ps2_get_responce_to(uint32_t timeout);

#endif
