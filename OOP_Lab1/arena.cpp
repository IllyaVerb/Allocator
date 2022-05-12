#include "arena.h"
#include "kernel.h"
#include "tree.h"

/**
 * Create new arena
 * by asked size (for BIG blocks), or for default (ARENA_SIZE) size
 * return pointer to arena payload
*/
void* arena_create(size_t size, block_tree* tree) {
    size_t kernel_alloc_size = size + HEADER_SIZE > ARENA_SIZE ? size + HEADER_SIZE : ARENA_SIZE;

    void* block = kernel_alloc(kernel_alloc_size);
    if (block == NULL) {
        printf("ERROR! kernel_alloc return NULL\n");
        return NULL;
    }

    void* payload = arena_to_payload((header*)block);
    setters(payload, kernel_alloc_size - HEADER_SIZE, NULL, false, true, true);

    if (block_tree_is_empty(tree))
        block_tree_create(tree);
    block_tree_init_add(tree, payload, kernel_alloc_size - HEADER_SIZE);

    return payload;
}

/**
 * Remove arena
 * and release memory to kernel
*/
void arena_remove(header* arena)
{
    kernel_free(arena, 0);
}