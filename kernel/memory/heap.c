#include "vmm.h"
#include "string.h"
#include "heap.h"
#include "../kernel/kernel.h"
#include "../programs/terminal.h"
#include "../utils/asm_tools.h"

uintptr_t *heap_start;
uintptr_t *cur_heap_pos;

void init_heap()
{
	// Will need a method to differentiate heaps later
	heap_start = page_kalloc(HEAP_START_SIZE, 0x3, 0);
	memset(heap_start, 0, HEAP_START_SIZE);
	cur_heap_pos = (uintptr_t *) heap_start;
	logf("Heap initialized\n");
}

void *malloc(size_t size)
{
	// Save for userspace	
}

void free(void *mem)
{
	// Save for userspace
}

void *kalloc(size_t size)
{
	// We will add an alignment of 0x20 for ALL sizes
	// The first -1 makes sure we dont bring over 0x20 threshold while the second makes us grab all bits BUT the important ones
	// For the bitwise roundup to work, it must be to a power of 2
	size = (0x20 - 1 + size ) & ~(0x20 - 1);
	struct mem_header *cur_node = (struct mem_header *) heap_start;	
	struct mem_header *past_node = 0;

	while ((uint8_t *) cur_node < (uint8_t *) cur_heap_pos) {
		if (cur_node->size >= size && cur_node->status == HEAP_FREE) {
			// We only need to worry about splitting if it isnt a new node
			if (cur_node->size - size >= HEAP_MIN_SPLIT_SIZE) {
				struct mem_header *split_node = (struct mem_header *) ((uint8_t*) cur_node + size + sizeof(struct mem_header));
				split_node->status = HEAP_FREE;
				split_node->size = cur_node->size - size - sizeof(struct mem_header);
				split_node->next = cur_node->next;
				split_node->prev = cur_node->prev;
				cur_node->next = split_node;
				cur_node->size = size;
				logf("Split heap node: %x with size %x\n", split_node, cur_node->size);
			}
			
			cur_node->status = HEAP_USED;
			cur_node->prev = past_node;
			logf("Allocated heap node at %x with size %x\n", cur_node, cur_node->size);
			return (void *) (cur_node + 1);
		}
		past_node = cur_node;
		cur_node = (struct mem_header *)((uint8_t *)cur_node + (cur_node->size + sizeof(struct mem_header)));
	}

	
	cur_node = (struct mem_header *) cur_heap_pos;
	cur_node->prev = past_node;
	if (past_node)
		past_node->next = cur_node;

	cur_node->size = size;
	cur_node->status = HEAP_USED;
	
	void *return_addr = (void *) ((uint8_t *) cur_node + sizeof(struct mem_header));	

	cur_heap_pos = (uintptr_t *) ((uint8_t *) cur_node + cur_node->size + sizeof(struct mem_header));
	logf("Allocated heap node at %x with size %x\n", return_addr, cur_node->size);
	return return_addr;
}

void kfree(void *mem)
{
	// Size does NOT include sizeof(struct mem_header)
	struct mem_header *node = (struct mem_header *) (mem - sizeof(struct mem_header));
	node->status = HEAP_FREE;
	struct mem_header *node_next = node->next;
	struct mem_header *node_prev = node->prev;

	if (node_next && node_next->status == HEAP_FREE) {
		node->size += node_next->size + sizeof(struct mem_header);	
		node_next->size = 0;
		
		node->next = node_next->next;
		logf("Defragmented node %t30%x%t10 with %t30%x%t10. New size: %x\n", node, node_next, node->size);
	}

	if (node_prev && node_prev->status == HEAP_FREE) {
		node_prev->size += node->size + sizeof(struct mem_header);
		node->size = 0;
		node->prev = 0;
		node_prev->next = node->next;
		if (node->next) {
			node->next->prev = node_prev; // Swapping nexts and prevs
			node->next = 0;
		}
		logf("Defragmented node %t30%x%t10 with %t30%x%t10. New size: %x\n", node_prev, node, node_prev->size);
	}
}

void *rekalloc(void *ptr, size_t size)
{	
	size = (0x20 - 1 + size ) & ~(0x20 - 1);
	struct mem_header *node = (struct mem_header *) (ptr - sizeof(struct mem_header));

	// We will look in front first
	struct mem_header *node_next = node->next;

	if (!node_next) goto free_memory;

	if (node_next->status == HEAP_FREE) {
		if (node_next->size + node->size >= size) {
			// Can merge with next node
			size_t old_size = node->size;
			node->size = size;
			if (node_next->size + old_size == size) {
				node->next = node_next->next;
				memset(node_next, 0, node_next->size + sizeof(struct mem_header));
				return (void*) node + sizeof(struct mem_header);
			}
			struct mem_header *new_next = (struct mem_header *) ((void *) node + node->size + sizeof(struct mem_header));
			new_next->status = HEAP_FREE;
			new_next->size = (node_next->size + old_size) - size;
			new_next->next = node->next->next;
			new_next->prev = node;
			node->next = new_next;
			return (void *) node + sizeof(struct mem_header);
		}
	} else {
free_memory:
		void *new_addr = kalloc(size);
		memcpy(new_addr, ptr, node->size);
		kfree(ptr);
		return new_addr;
	}

	return (void *) 0;

}
