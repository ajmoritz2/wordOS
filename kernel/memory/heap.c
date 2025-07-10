#include "vmm.h"
#include "heap.h"
#include "../kernel/kernel.h"
#include "../programs/terminal.h"

uintptr_t *heap_start;
uintptr_t *cur_heap_pos;

void init_heap()
{
	// Will need a method to differentiate heaps later
	heap_start = page_kalloc(HEAP_START_SIZE, 0x3, 0);
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
	logf("Size of memory alloced %x\n", size);
	struct mem_header *cur_node = (struct mem_header *) heap_start;	
	struct mem_header *past_node = 0;

	while ((uint8_t *) cur_node < (uint8_t *) cur_heap_pos) {
		logf("Cur node %x\n", cur_node);
		if (cur_node->size >= size && cur_node->status == HEAP_FREE) {
			// We only need to worry about splitting if it isnt a new node
			if (cur_node->size - size >= HEAP_MIN_SPLIT_SIZE) {
				struct mem_header *split_node = (struct mem_header *) ((uint8_t*) cur_node + cur_node->size + sizeof(struct mem_header));
				split_node->status = HEAP_FREE;
				split_node->size = cur_node->size - size - sizeof(struct mem_header);
				split_node->next = cur_node->next;
				split_node->prev = cur_node->prev;
				cur_node->next = split_node;
				logf("Split heap node: %x with size %x\n", split_node, size);
			}
			
			cur_node->status = HEAP_USED;
			cur_node->prev = past_node;
			if (past_node) {
				cur_node->next = past_node->next;
				past_node->next = cur_node;
			}
			return (void *) (cur_node);
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
	logf("CUR: %t30%x %t5aNODE PREV: %x%t10\n", cur_node, cur_node->prev);
	return return_addr;
}

void kfree(void *mem)
{
	// Size does NOT include sizeof(struct mem_header)
	struct mem_header *node = (struct mem_header *) (mem - sizeof(struct mem_header));

	node->status = HEAP_FREE;
	struct mem_header *node_next = node->next;
	struct mem_header *node_prev = node->prev;

		printf("Sizeof mem_header: %x\n", sizeof(struct mem_header));
	if (node_next && node_next->status == HEAP_FREE) {
		node->size += node_next->size + sizeof(struct mem_header);	
		node_next->size = 0;
		
		node->next = node_next->next;
		printf("Defragmented node %t30%x%t10 with %t30%x%t10. New size: %x\n", node, node_next, node->size);
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
		printf("Defragmented node %t30%x%t10 with %t30%x%t10. New size: %x\n", node_prev, node, node_prev->size);
	}
}
