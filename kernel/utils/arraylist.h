/* 
 * Array List utility
 *
 * WORDOS
 *
 * 11/30/25
 *
 * NOTE FOR USERS:
 * Requires the initialization of a heap to work properly
 */

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stdint.h>

struct array_list {
	uint32_t dt_size; // Size of the data type
	uint16_t count;   // Count of items in array
	uint16_t max_count; // Max count in array
	void *data;
};

typedef struct array_list array_list_t;

struct array_list *create_array_list(uint16_t data_type_size);

void al_add_item(struct array_list **al, void *item);
void al_delete_item(struct array_list *al, uint32_t index);
void *al_get(struct array_list *al, uint32_t index);


#endif
