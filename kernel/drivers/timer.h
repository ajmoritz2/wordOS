#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void polled_sleep(uint32_t time_millis);
uint32_t poll_timer();
void arm_interrupt_timer(uint32_t ticks);

#endif
