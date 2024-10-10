#include <stdint.h>
#include "gdt.h"
#include "kernel.h"

struct gdt_entry desc[5];
struct GDTR gp;

extern void flush_gdt(uint32_t gp);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	desc[num].base_low = (base & 0xFFFF);
	desc[num].base_mid = (base >> 16) & 0xFF;
	desc[num].base_high = (base >> 24) & 0xFF;

	desc[num].limit_low = (limit & 0xFFFF);
	desc[num].gran = (limit >> 16) & 0x0F;
	desc[num].gran |= (gran & 0xF0); // Error reason here was granularity was still set to 0. So the jump didnt cover all memory
	desc[num].access = access;
}

void gdt_install(void)
{
	gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
	gp.base = (uint32_t)&desc;

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
	gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
	gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	flush_gdt((uint32_t)&gp);
}
