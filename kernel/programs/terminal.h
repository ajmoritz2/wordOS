#ifndef TERMINAL_PROG_C
#define TERMINAL_PROG_C
#include <stdint.h>

uint8_t is_term_ready();
void init_terminal();
void start_terminal();
void printf(char *string, ...);
void tflush();
void terminal_loop();

#endif
