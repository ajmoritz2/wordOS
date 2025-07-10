#ifndef APIC_H
#define APIC_H
#include <stdint.h>

#define LAPIC_TIMER_COUNT_REG	0x390
#define LAPIC_TIMER_SET_REG		0x380

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

void write_ioapic_register(uint32_t reg, uint32_t data);
void set_initial_lapic_timer_count(uint32_t count);
void set_pit_one_shot(uint16_t count);
void calibrate_timer();
uint32_t lapic_read_reg(uint32_t reg);
void lapic_write_reg(uint32_t reg, uint32_t data);
uint32_t get_lapic_timer_count();
uint32_t end_calibration();
void calibrate_lapic();
void send_EOI(void* apic_addr);
void disable_pic();
void init_apic();

#endif
