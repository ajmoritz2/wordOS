/* ASM TOOLS HEADER FILE
 *
 * 11/03/25
 */

#include <stdint.h>

static inline int get_iflag()
{
	uint32_t eflags = 0;
	asm volatile ("pushf\nmovl (%%esp), %%eax\n\movl %%eax, %0" : "=m"(eflags));
	return eflags & (1 << 9);
}
