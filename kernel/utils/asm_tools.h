/* ASM TOOLS HEADER FILE
 *
 * 11/03/25
 */
<<<<<<< HEAD

#include <stdint.h>

static inline int get_iflag()
{
	uint32_t eflags = 0;
	asm volatile ("pushf\nmovl (%%esp), %%eax\n\movl %%eax, %0" : "=m"(eflags));
	return eflags & (1 << 9);
}
=======
#ifndef ASM_TOOLS_H
#define ASM_TOOLS_H
#include <stdint.h>

struct cpuid_status {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
};

struct cpuid_status get_cpuid(int eax_val);

uint64_t read_msr(uint32_t msr);

static inline int get_iflag()
{
	uint32_t eflags = 0;
	asm volatile ("pushf\nmovl (%%esp), %%eax\nmovl %%eax, %0" : "=m"(eflags));
	return eflags & (1 << 9);
}

#endif
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
