/* Tools for certain ASM calls
 *
 * 11/03/25
 */

inline int get_iflag()
{
	int eflags = 0;
	asm volatile ("pushf\nmov (%%esp), %0" : "=m"(eflags));
	return eflags & (1 << 8);
}
