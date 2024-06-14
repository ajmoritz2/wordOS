# Declare constants here
.set MAGIC_NUMBER, 0xE85250D6
.set ARCH, 0
.set PAGESIZE, 4096 # 3 MiB
.set BOOTLOC, 0xC0000000 # Loc of kernel

.section .mb2_hdr, "aw"
# Start of header
mb2_hdr_begin:
.align 4
.long MAGIC_NUMBER
.long ARCH
.long (mb2_hdr_end - mb2_hdr_begin) 
.long -(MAGIC_NUMBER + (mb2_hdr_end - mb2_hdr_begin))

mb2_frame_buffer_req:
	.short 5 # Tag identifier
	.short 1 # Flag 
	.long (mb2_framebuffer_end - mb2_frame_buffer_req) # Should be 20 if I read the docs correct
	.long 0
	.long 0
	.long 0
mb2_framebuffer_end: # Puts in an address space
.align 8
# the end tag: type = 0, size = 8
.long 0
.long 8

mb2_hdr_end:

# END OF THE HEADER FILE

# STARTING SHIM PORTION

# init all the stack
.section .bootstrap_stack, "aw", @nobits
boot_stack_base:
	.skip  16384 # 16 KiB
stack_top:


.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:


	# Transfer control to the main kernel
	call kernel_main

	# If there is an unexpected return, hang on
	cli
1:	hlt
	jmp 1b

.global flush_gdt
.type flush_gdt, @function
flush_gdt:
        cli
        lgdt (gp)
        movw $0x10, %ax
        movw %ax, %ds
        movw %ax, %es
        movw %ax, %fs
        movw %ax, %gs
        movw %ax, %ss
        jmp $0x08, $.flush
.flush:
        ret
