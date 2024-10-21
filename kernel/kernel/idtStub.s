.section .text

.macro isr_err_stub num
.global isr_stub_\num
.type isr_stub_\num, @function
isr_stub_\num:
	cli
	pushl $\num
	jmp isr_frame_as
.endm

.macro isr_no_err_stub num
.global isr_stub_\num
.type isr_stub\num, @function
isr_stub_\num:
	cli
	pushl $0
	pushl $\num
	jmp isr_frame_as
.endm

.macro irq_stub num 
.global irq_stub_\num
.type irq_stub_\num, @function
irq_stub_\num:
	cli
	pushl $\num
	jmp irq_frame
.endm

isr_frame_as:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi

	movl %cr0, %eax
	pushl %eax
	movl %cr2, %eax
	pushl %eax
	movl %cr3, %eax
	pushl %eax 

	call isr_handler
	
	popl %eax
	movl %eax, %cr3
	popl %eax
	movl %eax, %cr2
	popl %eax
	movl %eax, %cr0

	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	addl $8, %esp
	sti
	iret

irq_frame:
	call irq_handler
	addl $4, %esp 
	sti
	iret

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

irq_stub 48
irq_stub 49
irq_stub 50
