/* 
 * Array List Code
 *
 * WORDOS
 *
 * 11/30/25
 *
 *
 * NOTE: Requires Heap to be initialized...
 */

#include <stdint.h>
#include "../memory/heap.h"
#include "../memory/string.h"

#include "arraylist.h"


struct array_list *create_array_list(uint16_t data_type_size) 
{
	struct array_list *list = kalloc(data_type_size + sizeof(struct array_list));

	memset(list, 0, data_type_size + sizeof(struct array_list));
	
	list->max_count = 1;
	list->dt_size = data_type_size;
	
	return list;
}

void al_add_item(struct array_list **array, void *item) 
{
	struct array_list *al = *array;

	if (al->count == al->max_count) {
		// Realloc necessary

		uint32_t new_max = al->max_count * 2;
		uint32_t new_size = new_max * al->dt_size
							+ sizeof(struct array_list);

		struct array_list *new_al = rekalloc(al, new_size); 

		new_al->max_count = new_max;
		al = new_al;
		*(array) = al;
	}

	void *data_addr = (void *) ((uint64_t) &(al->data) + (al->count * al->dt_size));
	memcpy(data_addr, 
			item, al->dt_size);
	al->count += 1;
}

void *al_get(struct array_list *al, uint32_t index)
{
	if (index > al->count) {
		return 0;
	}
	void *data_addr = (void *) ((uint64_t) &(al->data) + (index * al->dt_size));
	return data_addr;
}

void al_delete_item(struct array_list *al, uint32_t index) 
{
	void *index_start = al_get(al, index);
	
	for (int i = index; i < al->count - 1; i++) {
		memcpy(index_start, index_start + al->dt_size, al->dt_size);

		index_start += al->dt_size;
	}

	memset(index_start, 0, al->dt_size);

	al->count--;
}
/*
void print_array_list(struct array_list *al)
{
	printf("AL MAX COUNT: %d, Cur Count %d, data_size %x\n", al->max_count, al->count, al->dt_size);
	for (int i = 0; i < al->count; i++) {
		if (!(i % 10))
			printf("\n");
		printf("%d, ", *(int *)al_get(al, i));
	}
}*/
