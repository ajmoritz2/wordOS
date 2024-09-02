#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct __attribute__((packed)) GDTR {
	uint16_t limit;
	uint32_t base;
};

struct __attribute__((packed)) gdt_entry {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid; // Start of the fun parts
	uint8_t access;
	uint8_t gran; // Includes limit_high and flag bits. Limit high are bits 0-3 and flags are 4-7
	uint8_t base_high;
};

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install(void);

#endif
