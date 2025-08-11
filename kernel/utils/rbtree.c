/*`RED BLACK TREE C
 * 07/08/25
 * WORDOS
 */
#include "rbtree.h"
#include "../kernel/kernel.h"
#include "../memory/vmm.h"

void *alloc_node_obj(struct vmm *cvmm)
{
	rbnode_t *start = (rbnode_t *) (cvmm->vm_obj_store_addr + sizeof(vmm));

	while (start->flags & 1) {
		start++;
	}

	start->flags = 3;

	return (void *) start;
}

void free_node_obj(void *base, struct vmm *cvmm)
{	

	rbnode_t *obj = (rbnode_t *) base;

	if (!(obj->flags & 2)) {
		logf("Node with base %x does not exist\n", base);
		return;
	}

	logf("Freed %x\n", base);
	obj->flags &= ~2;
	obj->base = 0;
}

void rb_left_rotation(rbnode_t **root, rbnode_t *node)
{
	rbnode_t *y = node->right;

	node->right = y->left;
	
	if (y->left != NULL) {
		y->left->parent = node;
	}

	y->parent = node->parent;

	if (!y->parent) {
		*root = y;
	} else if (node == node->parent->left) {
		node->parent->left = y;
	} else {
		node->parent->right = y;
	}

	y->left = node;
	node->parent = y;
}

void rb_right_rotation(rbnode_t **root, rbnode_t *node)
{
	rbnode_t *y = node->left;

	node->left = y->right;
	
	if (y->right != NULL) {
		y->right->parent = node;
	}

	y->parent = node->parent;

	if (!y->parent) {
		*root = y;
	} else if (node == node->parent->left) {
		node->parent->left = y;
	} else {
		node->parent->right = y;
	}

	y->right = node;
	node->parent = y;
}
void insert_violations(rbnode_t **root, rbnode_t *node)
{
	if (!node)
		return;
	if (!node->parent) {
		node->color = BLACK;
		//root
		return;
	}
	if (node->parent->color == BLACK)
		return;
	logf("Node size %x Node Address %x\n", node->size, node->base);
	rbnode_t *uncle = node->parent->parent->left;

	if (node->parent->parent->left == node->parent) {
		uncle = node->parent->parent->right;
	}
	
	int temp_color = BLACK;
	if (uncle)
		temp_color = uncle->color;

	// Case 1
		if (temp_color == RED) {
			node->parent->color = BLACK;
			uncle->color = BLACK;
			node->parent->parent->color = RED;
			insert_violations(root, node->parent->parent);
		} else {
		if (node->parent->parent->left == node->parent) {
			if (node->parent->right == node) {
				// Left-right child
				rb_left_rotation(root, node->parent);
				node = node->left;
			}
		} else {
			if (node->parent->left == node) {
				// Right-Left Child
				rb_right_rotation(root, node->parent);
				node = node->right;
			}
		}


		if (node->parent->left == node) {
			// Left-Left child
			node->parent->color = BLACK;
			node->parent->parent->color = RED;
			rb_right_rotation(root, node->parent->parent);
		} else {
			//Right-Right child
			node->parent->color = BLACK;
			node->parent->parent->color = RED;
			rb_left_rotation(root, node->parent->parent);
		}
	}

	(*root)->color = BLACK;
}

void insert_node(rbnode_t **root, rbnode_t *node)
{
	// Start with BST insertion
	//
	logf("Node inserted at base %x\n", node->base);
	
	if (*root == NULL)
		return;
	
	rbnode_t *cur = *root;
	rbnode_t *prev = NULL;

	uint8_t dir = 0;

	while (cur) {
		prev = cur;
		if (node->size > cur->size) {
			// Right move
			dir = 1;
			cur = cur->right;
		} else {
			// Left move
			dir = 0;
			cur = cur->left;
		}
	}


	if (dir)
		prev->right = node;
	else
		prev->left = node;
	node->parent = prev;
	node->color = RED;
	logf("Node parent color %d\n", node->parent->parent);

	if (node->parent->color != BLACK) {
		insert_violations(root, node);	
	}
}

void delete_violations(rbnode_t **root, rbnode_t *node)
{
	if (!node->parent) {
		node->color = BLACK;
		return; // root
	}
	
	if (node->color == DOUBLE_BLACK)
		goto double_blacks;

	if (node->color == RED) { // Simple Case 
		return;
	}
	
double_blacks:
	// Sibling time
	int sdir = 0; // 0 -- left  | 1 -- right
	rbnode_t *sibling = node->parent->left;
	if (sibling == node) {
		sibling = node->parent->right;
		sdir = 1;
	}

	if (node->color == BLACK || node->color == DOUBLE_BLACK) {
		node->color = DOUBLE_BLACK;

		int sib_left_color = BLACK;
		int sib_right_color = BLACK;
		
		if (sibling->left)
			sib_left_color = sibling->left->color;
		if (sibling->right)
			sib_right_color = sibling->right->color;

		if (sibling->color == BLACK && sib_left_color == BLACK && sib_right_color == BLACK) {
			// Case 3
			node->color = BLACK;
			sibling->color = RED;
			if (node->parent->color == BLACK) {
				node->parent->color = DOUBLE_BLACK;
				delete_violations(root, node->parent);
			} else {
				node->parent->color = BLACK;
			}
		} else if (sibling->color == RED) {
			int temp = sibling->color;
			sibling->color = node->parent->color;
			node->parent->color = temp;


			if (sdir) {
				// DB is to the right
				rb_left_rotation(root, node->parent);
			} else {
				// DB is to the left
				rb_right_rotation(root, node->parent);
			}
			delete_violations(root, node);
		} else if (sibling->color == BLACK) {

			if (sib_left_color == BLACK && sib_right_color == RED && !sdir) {
				// Far nephew is BLACK and near is RED
				sibling->color = RED;
				sibling->right->color = BLACK;
				rb_left_rotation(root, sibling);
				sibling = sibling->left;
			} else if (sib_right_color == BLACK && sib_left_color == RED && sdir) {
				// Far nephew is BLACK and near is RED for other side
				sibling->color = RED;
				sibling->left->color = BLACK;
				rb_right_rotation(root, sibling);
				sibling = sibling->right;
			}


			sibling = node->parent->left;
			if (sibling == node) {
				sibling = node->parent->right;
				sdir = 1;
			}


			if (sib_left_color == RED && sdir) {
				int parent_color = node->parent->color;
				node->parent->color = sibling->color;
				sibling->color = parent_color;
				rb_left_rotation(root, node->parent);
				
				node->color = BLACK;
				node->parent->parent->right->color = BLACK; // Far nephew
			} else if (sib_right_color == RED && !sdir) {
				int parent_color = node->parent->color;
				node->parent->color = sibling->color;
				sibling->color = parent_color;
				rb_right_rotation(root, node->parent);
				node->color = BLACK;
				node->parent->parent->left->color = BLACK;
			}
		}
	}		
}

void delete_node(struct vmm *cvmm, rbnode_t **root, rbnode_t *node)
{
	if (node == *root && node->left == NULL && node->right == NULL) {
		// Empty root case
		*root = NULL;
		free_node_obj(node, cvmm);
	}

	if (!(node->left || node->right)) { // Case 1, no children
		rbnode_t *parent = node->parent;
		delete_violations(root, node);
		if (node->size > parent->size) {
			parent->right = NULL;
		} else {
			parent->left = NULL;
		}

		free_node_obj(node, cvmm);
		return;	
	}

	if (node->left && node->right) { // Case 3, both children
		// We will go for max on left
		rbnode_t *min = node->left;

		while (min) {
			if (min->right == NULL)
				break;

			min = min->right;
		}

		delete_violations(root, min);
		node->base = min->base;
		node->size = min->size;
		node->flags = min->flags;

		if (min == node->left)
			node->left = NULL;
		else
			min->parent->right = NULL;
		
		if (node == *root)
			node->color = BLACK;

		free_node_obj(min, cvmm);
		return;
	}

	// Must be case 2
	
	rbnode_t *to_copy = node->right;
	if (node->left)
		to_copy = node->left;

	node->base = to_copy->base;
	node->size = to_copy->size;
	node->flags = to_copy->flags;

	delete_violations(root, to_copy);
	node->color = to_copy->color;
	node->right = to_copy->right;
	node->left = to_copy->left;
	if (node == *root)
		node->color = BLACK;
	free_node_obj(to_copy, cvmm);
	 
}

rbnode_t *make_node(struct vmm *cvmm, size_t base, size_t size, int id)
{
	rbnode_t *new_node = alloc_node_obj(cvmm);
	new_node->size = size;
	new_node->id = id;
	new_node->base = base;
	if (cvmm->root == NULL) {
		new_node->color = BLACK;
		new_node->left = 0;
		new_node->right = 0;
		new_node->parent = 0;
		cvmm->root = new_node;
	} else {
		insert_node((rbnode_t **) &(cvmm->root), new_node);
	}
	return new_node;
}

rbnode_t *find_node(rbnode_t *root, size_t size) 
{
	rbnode_t *cur_node = root;

	int found = 0;

	while (cur_node) {
		if (cur_node->size == size) {
			found = 1;
			break;
		}

		if (cur_node->size <= size) {
			cur_node = cur_node->right;
		} else {	
			cur_node = cur_node->left;
		}
	}
	if (found)
		return cur_node;

	logf("Node of size %d was not found\n", size);
	return NULL;
}	
