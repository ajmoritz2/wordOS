#include "../kernel/kernel.h"
#include "apic.h"
#include <stdint.h>

extern uint32_t ticks_per_ms;

void polled_sleep(uint32_t time)
{
	uint32_t first_count = get_lapic_timer_count();
	uint32_t time_left = 0;
	uint32_t tick_time = time*ticks_per_ms;

	uint32_t passes = 0;
	uint32_t wanted_passes = 0;
	
	if (tick_time > first_count) {
		wanted_passes = 1;
		time_left = 0xFFFFFFFF - (tick_time - first_count);
	} else {
		time_left = first_count - tick_time;
	}


	uint32_t current_time = first_count;
	uint32_t last_time = first_count;
	while (passes <= wanted_passes) {
		current_time = get_lapic_timer_count();
		if (passes == wanted_passes) {
			if (current_time < time_left) {
				break;
			}
		}

		if (current_time > last_time){
			passes++;
		}

		last_time = current_time;
	}
}

uint32_t poll_timer()
{
	return get_lapic_timer_count();
}

void arm_interrupt_timer(uint32_t ticks)
{
	lapic_write_reg(0x320, lapic_read_reg(0x320) & 0xFF);
	set_initial_lapic_timer_count(ticks);
}
