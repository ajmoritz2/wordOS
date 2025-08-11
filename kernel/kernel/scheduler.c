#include "kernel.h"
#include "../memory/vmm.h"
#include "../memory/pmm.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "../memory/string.h"
#include <stdint.h>
#include "scheduler.h"

process_t *process_list_head;
process_t *current_process;

size_t next_free_pid = 0;

void add_process(process_t *process)
{
	process_t *current = process_list_head;
	while (current->next) {
		current = current->next; 
	}
	

	current->next = process;
}

void delete_process(process_t *process)
{
	if (process_list_head == process) {
		process_t *next = process_list_head->next;
		kfree(process_list_head);
		process_list_head = next;
		// Edge case (probably)
	}
	process_t *prev_process = process_list_head;

	// Searches for the previous process so we can set next
	while (prev_process->next) {
		if (prev_process->next == process)
			break;
		prev_process = prev_process->next;
	}

	prev_process->next = process->next; // Should be 0 if NULL
	
	kfree(process);
}

process_t *get_next_process()
{
	// ?
	return current_process->next;
}

void run_idle()
{
	while (1)
		asm("hlt");
}

void configure_page_directory(process_t *process)
{
	// I need to grab the root_pd first and then swap it out
	void *pd_phys_addr = alloc_phys_page(); // Our directory! ayyy
	void *pd_virt_addr = page_kalloc(1024, 0x3, (uint32_t) pd_phys_addr);

	void *vmm_store_phys = alloc_phys_page();
	void *vmm_store_addr = page_kalloc(1024, 0x3, (uint32_t) vmm_store_phys); // TODO: Make these concurrent (is that the right word?)
	
	int pd_index_to_copy = (uint32_t) pd_virt_addr >> 22; // We can just copy the page directory because awesome!
	int vmm_index = (uint32_t) vmm_store_addr >> 22;

	// It is possible for them not to be the samme (stupid programs!)
	
	vmm *new_vmm = create_vmm(pd_virt_addr, 0x10000, 0xC0000000, vmm_store_addr);

	set_current_vmm(new_vmm);

	logf("Babyloni--------------------------------- New_vmm locetion: %x\n", new_vmm->root);
	void *process_pd_virt = page_alloc(4096, 0x3, (uint32_t) pd_phys_addr);
	void *process_vmm_virt = page_alloc(4096, 0x3, (uint32_t) vmm_store_phys);

	copy_higher_half_kernel_pd(pd_virt_addr);	
	logf("PD Index for pd_index: %d\n", pd_index_to_copy);

	((uint32_t* )pd_virt_addr)[1023] = (uint32_t) pd_phys_addr | 3;

	process->vmm_obj = new_vmm;
}

process_t *create_process(char *name, void(*function)(void), void *arg, int privileged) 
{
	asm volatile ("cli");
	process_t *process = kalloc(sizeof(process_t));

	configure_page_directory(process);
	logf("Page directory: %x\n", process->vmm_obj->root_pd[1023] & ~0x3);

	strncpy(process->name, name, P_NAME_MAX_LEN);
	process->pid = next_free_pid++;
	process->status = READY;
	
	// Allocate new context
	process->context = kalloc(sizeof(cpu_status_t));
	process->context->ss = 0x10; // KERNEL_DS
	

	// Create new page directory

	process->context->esp = (uint32_t) alloc_stack(process->vmm_obj);
	process->context->eflags = 0x202;
	process->context->cs = 0x08;
	process->context->eip = (uint32_t)function;
//	process->context->ebp = 0;

//	add_process(process);

	asm volatile ("sti");

	return process;
}

cpu_status_t *schedule(cpu_status_t *context)
{
	current_process->context = context;
	current_process->status = READY;
	while (1) {
		if (current_process->next != NULL) {
			current_process = current_process->next;
		} else {
			current_process = process_list_head;
		}	

		if (current_process != NULL && current_process->status == DEAD) {
			delete_process(current_process);
		} else {
			current_process->status = READY;
			break;
		}
	}

	return current_process->context;
}
