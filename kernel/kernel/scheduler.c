#include "kernel.h"
#include "idt.h"
#include "../memory/vmm.h"
#include "../memory/pmm.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "../memory/string.h"
#include "../programs/terminal.h"
#include <stdint.h>
#include "scheduler.h"

#define PRIV_NO_ADD		2

process_t *process_list_head;
process_t *current_process;
process_t *idle_process;

int initialized = 0;

size_t next_free_pid = 0;

void run_idle()
{
	while (1)
		asm("hlt");
}

void set_current_process_idle()
{
	current_process->status = IDLE;
	asm ("hlt");
}

void init_idle_process()
{
	idle_process = create_process("idle", &run_idle, 0, PRIV_NO_ADD);	
}

void init_scheduler()
{
	process_list_head = NULL;
	current_process = NULL;
	init_idle_process();
	initialized = 1;
}

void add_process(process_t *process)
{
	logf("Adding process %x to the list with next %x.\n", process, process->next);
	process_t *current = process_list_head;

	logf("Must cut o\n");
	logf("Cutoff------------------------ PID: %x\n", process->pid);
	logf("New process' Context stored at %x\n", process->context);
	if (!current) {
		process_list_head = process;
		return;
	}
	while (current->next) {
		current = current->next; 
	}
	
	current->next = process;
}

void delete_process(process_t *process)
{
	logf("Delted\n");
	if (process_list_head == process) {
		
		process_t *next = process_list_head->next;
		process_list_head = next;
		kfree(process);
		return;
	}
	process_t *prev_process = process_list_head;

	// Searches for the previous process so we can set next
	while (prev_process->next) {
		if (prev_process->next == process)
			break;
		prev_process = prev_process->next;
	}

	prev_process->next = process->next; // Should be 0 if NULL
	logf("Process %d freed\n", process->pid);
	logf("Process deleting context at %x\n", process->context);
	kfree(process->context);	
	kfree(process);
}

process_t *get_process_head()
{
	return process_list_head;
}

void create_new_pdptes(process_t *process)
{
	uint64_t phys_pdlow = alloc_phys_page();
	uint64_t phys_pdmed = alloc_phys_page();
	uint64_t phys_pdhigh = alloc_phys_page();

	uint64_t vmm_store_phys = alloc_phys_page();

	uint32_t *vmm_kspace = pae_kalloc(4096, 0x3, vmm_store_phys);

	vmm *new_vmm = create_vmm(0, 0x10000, 0xc0000000, vmm_kspace);

	process->vmm_obj = new_vmm;
	logf("pdpt: %x, low_phys: %x\n", new_vmm->root_pdpt, phys_pdlow);

	uint64_t *temp_page = (uint64_t *) TEMP_PAGE_VIRT;

	pae_mmap(process->vmm_obj->root_pdpt, phys_pdlow, (uint32_t *) TEMP_PAGE_VIRT, 0x3);
	asm ("movl %cr3, %eax\n\ 
			movl %eax, %cr3"); // TLB Flush
	memset(temp_page, 0, 4096);
	temp_page[511] = phys_pdlow | 1;

	pae_mmap(process->vmm_obj->root_pdpt, phys_pdmed, (uint32_t *) TEMP_PAGE_VIRT, 0x3);
	asm ("movl %cr3, %eax\n\ 
			movl %eax, %cr3"); // TLB Flush
	memset(temp_page, 0, 4096);
	temp_page[511] = phys_pdmed | 1;

	pae_mmap(process->vmm_obj->root_pdpt, phys_pdhigh, (uint32_t *) TEMP_PAGE_VIRT, 0x3);
	asm ("movl %cr3, %eax\n\
			movl %eax, %cr3");
	temp_page[511] = phys_pdhigh | 1;

	pae_unmap((uint32_t *) temp_page);

	new_vmm->pd_low = phys_pdlow;
	new_vmm->pd_mid = phys_pdmed;
	new_vmm->pd_high = phys_pdhigh;

	set_current_vmm(new_vmm); // New stack ---- CANNOT USE VARIABLES OR RETURN FUNCTION, BUT ESP IS SAME SO WE GET JUNK!
	uint32_t *vmm_pspace = pae_alloc(4096, 0x3, (uint32_t) vmm_store_phys);
	
	uint32_t root_offset = (uint32_t) new_vmm->root - (uint32_t) new_vmm->vm_obj_store_addr; 
	new_vmm->vm_obj_store_addr = (uint32_t) vmm_pspace + sizeof(vmm);
	
	// Set the root node 28 offset
	new_vmm->root = (void *) (uint32_t) new_vmm->vm_obj_store_addr + root_offset;

	process->context->esp = (uint32_t) alloc_stack(new_vmm) - 0x20;

}

void kill_current_process()
{
	current_process->status = DEAD;
	logf("Process Killed hahahha %x\n", current_process);
	current_process->next = 0;
	while (1)
		asm volatile ("hlt");
}

process_t *create_process(char *name, void(*function)(void), void *arg, int privileged) 
{
	vmm *parent_vmm = current_vmm;
	process_t *process = kalloc(sizeof(process_t));
	logf("kio\n");
	logf("Creating process at %x\n", process);
	memset(process, 0, sizeof(process));

	logf("Process name: %s\n", name);
	strncpy(process->name, name, P_NAME_MAX_LEN);
	process->pid = next_free_pid++;
	logf("Process pid: %d\n", process->pid);
	process->status = READY;
	
	// Allocate new context
	process->context = kalloc(sizeof(cpu_status_t));
	memset(process->context, 0, sizeof(cpu_status_t));

	create_new_pdptes(process);
	
//	process->context->ss = 0x10; // KERNEL_DS
	process->context->eflags = 0x200202;
	process->context->cs = 0x08;
	process->context->eip = (uint32_t)function;
	process->next = NULL;
	uint32_t init_cr0 = 0;
	uint32_t init_cr3 = 0;
	asm volatile ("mov %%cr0, %0\n\
			mov %%cr3, %1" : "=r" (init_cr0), "=r" (init_cr3));
	process->context->cr0 = init_cr0;
	process->context->cr2 = 0;
	process->context->cr3 = init_cr3;

	process->context->ebx = 0xbeef;

	
	if (privileged != PRIV_NO_ADD) {
		add_process(process);
	}
	logf("Process context stored at %x\n", process->context);
	if (parent_vmm)
		set_current_vmm(parent_vmm);

	logf("Process truly finished...\n");

	return process;
}


cpu_status_t *schedule(cpu_status_t *context)
{
	// Process switch explodes the world in this case...
	// Because of this return values are not consistent and will be set to 0
	if (current_process) {
		memcpy(current_process->context, context, sizeof(cpu_status_t));
	}
		
	while (current_process) {

		if (current_process->status == READY)
			break;
		
		if (current_process->status == DEAD) {
			delete_process(current_process);
			logf("Process dead\n");
		} else {
			current_process->status = READY;
		}
		// If the process is IDLE it will come to here and continue to next process.
		current_process = current_process->next;
	}

	if (!current_process) {
		if (process_list_head) {
			current_process = process_list_head;
		} else {
			current_process = idle_process;
		}
	}

	current_process->status = RUNNING;
	set_current_vmm(current_process->vmm_obj);


	return current_process->context;
}

cpu_status_t *handle_schedule(cpu_status_t *context)
{
	if (!initialized) {
		logf("Context returnsed\n");
		return context;
	}
	cpu_status_t *to_ret = schedule(context);

	return to_ret;
}
