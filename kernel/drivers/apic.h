#ifndef APIC_H
#define APIC_H
#include <stdint.h>


struct LVTEntry {
	uint16_t Vector : 8;
	uint16_t DelMode : 2;
	uint16_t DesMode : 1;
	uint16_t DelStat : 1;
	uint16_t PinPole : 1;
	uint16_t RemoteIRR : 1;
	uint16_t TriggerMode : 1;
	uint16_t InterruptsMask : 1;
};

void set_initial_timer_count(void* lapic_addr, uint32_t count);
void send_EOI(void* apic_addr);
void disable_pic();
void init_apic();

#endif
