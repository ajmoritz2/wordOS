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
	uint8_t base_mid;
	uint8_t access;
	uint8_t gran;
	uint8_t base_high;
};

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install(void);

#endif
