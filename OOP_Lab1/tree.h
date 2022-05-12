#ifndef TREE_H
#define TREE_H
#include <stdio.h>

#include "avl.h"

#define NODE_SIZE                     ROUND_BYTES(sizeof(tree_data))

#define block_tree_add(t, n)			avl_add((t), (n))
#define block_tree_del(t, n)			avl_remove((t), (n))
#define block_tree_is_empty(t)			avl_is_empty((t))
#define block_tree_create(t)			avl_create((t), data_cmp, offsetof(tree_data, node))

typedef avl_tree block_tree;

inline void init_tree_node(void* ptr, size_t size) {
	tree_data* ret = (tree_data*)ptr;
	ret->size = size;
	ret->next = NULL;
	ret->prev = NULL;
}

inline void block_tree_init_add(block_tree* tree, void* node, size_t size) {
	init_tree_node(node, size);
	block_tree_add(tree, node);
}

inline bool block_tree_exist(block_tree* tree, void* node) {
	return avl_find(tree, node, NULL) != NULL ? true : false;
}

static int data_cmp(const void* p1, const void* p2) {
	const tree_data* d1 = (tree_data*)p1;
	const tree_data* d2 = (tree_data*)p2;

	if (d1->size < d2->size)
		return -1;
	if (d1->size > d2->size)
		return 1;
	return 0;
}
#endif