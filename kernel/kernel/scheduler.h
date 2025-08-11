#ifndef WORD_SCHEDULER
#define WORD_SCHEDULER

#include <stdint.h>
#include <stddef.h>
#include "idt.h"
#include "../memory/vmm.h"

#define P_NAME_MAX_LEN	64

typedef enum {
	READY,
	RUNNING,
	DEAD
} process_status_t;

typedef struct process_t {
	size_t pid;
	process_status_t status;
	cpu_status_t *context;
	vmm *vmm_obj;
	char name[P_NAME_MAX_LEN];
	struct process_t* next;
} process_t;

process_t *create_process(char *name, void(*function)(void), void *arg, int privileged);
void add_process(process_t *process);
void delete_process(process_t *process);
process_t *get_next_process();

#endif
