#ifndef MULTIBOOT_PARSE
#define MULTIBOOT_PARSE
#include <stdint.h>
#include "vmm.h"
void init_multiboot(vmm* current_vmm, uint32_t addr);

#endif
