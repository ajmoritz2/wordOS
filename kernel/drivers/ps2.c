/*	WORD OS
 *	PS2_C
 *	
 *	Driver for the I8042 ps/2 controller
 */

#include "../kernel/kernel.h"
#include "../programs/terminal.h"
#include "ps2.h"
#include <stdint.h>

#define CMD_PORT	0x64
#define DATA_PORT 0x60
#define STATUS_PORT		0x64

uint8_t check_status()
{
	uint8_t status = inportb(STATUS_PORT);	
	
	return status;
}

void ps2_send_byte_port1(uint8_t byte)
{
	uint32_t time = 0;
	uint32_t timeout = 3200;

	while ((check_status() & 2)) {
		if (time == timeout) {
			logf("Unable to send bit...\n");
			return;
		}
		time++;
	}
	
	outportb(0x60, byte);
	// with macro outportb(DATA_PORT, byte);
}

void ps2_send_byte_port2(uint8_t byte)
{
	uint32_t time = 0;
	uint32_t timeout = 3200;
	outportb(0x64, 0xD4);

	while ((check_status() & 2)) {
		if (time == timeout) {
			logf("Unable to send bit...\n");
			return;
		}
		time++;
	}
	
	outportb(0x60, byte);
	// with macro outportb(DATA_PORT, byte);
}

uint8_t ps2_get_responce()
{
	while (!(check_status() & 1));

	return inportb(DATA_PORT);
}

uint8_t ps2_get_responce_to(uint32_t timeout)
{
	uint32_t time = 0;
	while (!(check_status() & 1)) {
		if (time >= timeout)
			return 0x0;
		time++;
	}

	return inportb(DATA_PORT);
}

void ps2_send_dbyte(uint8_t byte1, uint8_t byte2)
{
	outportb(CMD_PORT, byte1);
	
	while ((check_status() & 2)) { logf("STATUS: %x\n", check_status());}

	outportb(CMD_PORT, byte2);
}

void ps2_send_command(uint8_t byte)
{
	while ((check_status() & 2));
	outportb(CMD_PORT, byte);

}
