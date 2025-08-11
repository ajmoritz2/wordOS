/*	RB_TREE_H
 *	07/08/25
 *	WORDOS
 */
#ifndef H_RBTREE
#define H_RBTREE

#include "../memory/vmm.h"

#define RED	0
#define BLACK 1
#define DOUBLE_BLACK 2

typedef struct rbtree_node {
	uint32_t id; // DEBUG
	uint32_t base;
	uint32_t size;
	size_t flags;
	int color; 
	struct rbtree_node *left;
	struct rbtree_node *right;
	struct rbtree_node *parent;
} rbnode_t;

void insert_node(rbnode_t **root, rbnode_t *node);
void delete_node(vmm *cvmm, rbnode_t **root, rbnode_t *node);
rbnode_t *find_node(rbnode_t *root, size_t size);
rbnode_t *make_node(vmm *cvmm, size_t base, size_t size, int id);
#endif
