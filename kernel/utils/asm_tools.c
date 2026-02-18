/* Tools for certain ASM calls
 *
 * 11/03/25
 */

<<<<<<< HEAD
inline int get_iflag()
{
	int eflags = 0;
	asm volatile ("pushf\nmov (%%esp), %0" : "=m"(eflags));
	return eflags & (1 << 8);
=======
#include "asm_tools.h"
#include "../kernel/idt.h"
#include "../kernel/kernel.h"


struct cpuid_status get_cpuid(int eax_val)
{
	uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

	asm volatile ("mov %[eax_val], %%eax \n \
					cpuid \n \
					mov %%eax, %[eax] \n \
					mov %%ebx, %[ebx] \n \
				   	mov %%ecx, %[ecx] \n \
					mov %%edx, %[edx]"
					: [eax] "=m"(eax),
						[ebx] "=m"(ebx),
						[ecx] "=m"(ecx),
						[edx] "=m"(edx)
					: [eax_val] "r"(eax_val));

	struct cpuid_status ret_val = {eax, ebx, ecx, edx};

	return ret_val;
}

uint64_t read_msr(uint32_t msr)
{
	// TODO: change this to a get_msr function instead...
	uint32_t eax = 0; 
	uint32_t edx = 0;
	asm volatile ("mov %[msr], %%ecx \n \
			rdmsr \n \
			mov %%eax, %[eax] \n \
			mov %%edx, %[edx]" 
			: [eax] "=m"(eax),
			  [edx] "=m"(edx) 
			: [msr] "r"(msr));
	return (uint64_t)(eax | ((uint64_t)edx << 32));
>>>>>>> 8fae1a042b331c7b5acb0b428159f7ae1710921f
}
