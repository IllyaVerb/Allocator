#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "config.h"
#include "arena.h"
#include "Allocator.h"

#define DEBUG_MODE 1

using namespace std;

static block_tree tree;
#if DEBUG_MODE
static int count_arena;
static void* arenas[1000];
#endif

/**
 * Function that prints information about block
 * 
 * size_t id - number of block (0 for tree blocks)
 * void* ptr - pointer to block
*/
static void print_tree_data(size_t id, void* ptr) {
    printf(" %10zu | %10c | %10p | %10zu | %10zu | %10p | %10d | %10d\n",
        id,
        is_free(ptr) ? 'F' : 'B',
        payload_to_block(ptr),
        get_block_size(ptr),
        get_block_prev_size(ptr),
        ptr,
        get_FAB(ptr),
        get_LAB(ptr));
}

#if DEBUG_MODE
/**
 * Show all blocks in each arena
 * allocated with this allocator
*/
void mem_show(){
    printf("=== ARENAS DATA START ===\n");

    void* ptr = NULL;

    if (count_arena == 0) {
        printf("No arenas were allocated.\n");
        return;
    }
    
    for (int j = 0; j < count_arena; j++) {
        ptr = arenas[j];

        if (ptr == NULL) {
            printf("No blocks were allocated.\n");
            continue;
        }

        size_t i = 0;
        printf(" %10s | %10s | %10s | %10s | %10s | %10s | %10s | %10s\n\n",
            "N", "STATE", "HEADER", "SIZE", "P_SIZE", "PTR", "IS_FIRST", "IS_LAST");
        do {
            print_tree_data(i, ptr);

            if (get_LAB(ptr))
                break;
            ptr = block_next(ptr);
            i++;
        } while (1);

        printf("\n");
    }

    printf("===  ARENAS DATA END  ===\n\n");
}
#endif

/**
 * Show tree of free blocks
 * at this moment
*/
void mem_tree_show() {
    printf("===  TREE DATA START  ===\n\n");
    if (block_tree_is_empty(&tree))
        printf("Tree is empty.\n");
    else {
        printf(" %10s | %10s | %10s | %10s | %10s | %10s | %10s | %10s\n\n",
            "N", "STATE", "HEADER", "SIZE", "P_SIZE", "PTR", "IS_FIRST", "IS_LAST");
        avl_traverse(&tree, print_tree_data);
        printf("\n");
    }
    printf("===   TREE DATA END   ===\n\n");
}

/**
 * Allocate memory with size
 * return pointer to memory
 *
 * size_t size - size for getting memory
*/
void *mem_alloc(size_t size) {
    size_t size_align = max(ROUND_BYTES(size), NODE_SIZE);

    void* fitted = best_fit(&tree, size_align);
    if (fitted == NULL) {
        void* ptr = arena_create(size_align, &tree);
#if DEBUG_MODE
        arenas[count_arena++] = ptr;
#endif
        if (ptr == NULL)
            return NULL;
        fitted = ptr;
    }

    void* removed = block_tree_del(&tree, fitted);

    if ((!get_LAB(removed) && is_free(block_next(removed))) ||
        get_block_size(removed) - size_align >= HEADER_SIZE + NODE_SIZE) {
        void* rest = block_split(removed, size_align);

        if (rest != NULL) {
            if (!get_LAB(rest) && is_free(block_next(rest)))
                block_merge(&tree, rest);

            block_tree_init_add(&tree, rest, get_block_size(rest));
        }
    }

    set_busy(removed, true);

    return removed;
}

/**
 * Re-allocate memory with new size
 * return pointer to new memory
 *
 * void* ptr - pointer to block for reallocating
 * size_t size - new size for block
*/
void* mem_realloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return mem_alloc(size);
    }

    if (size == 0 || is_free(ptr)) {
        return NULL;
    }

    size_t size_align = max(ROUND_BYTES(size), NODE_SIZE);

    // realloc BIG blocks
    if (get_block_size(ptr) > ARENA_BLOCK_SIZE_MAX) {
        void* new_ptr = mem_alloc(size_align);

        if (new_ptr == NULL) {
            return NULL;
        }

        memcpy(ptr, new_ptr, min(get_block_size(new_ptr), get_block_size(ptr)));

        mem_free(ptr);

        return new_ptr;
    }

    // decrease size
    if (size_align < get_block_size(ptr)) {
        if ((!get_LAB(ptr) && is_free(block_next(ptr))) ||
            get_block_size(ptr) - size_align >= HEADER_SIZE + NODE_SIZE) {
            void* new_ptr = block_split(ptr, size_align);
            if (new_ptr != NULL) {
                if (!get_LAB(new_ptr) && is_free(block_next(new_ptr))) {
                    block_tree_del(&tree, block_next(new_ptr));
                    block_merge(&tree, new_ptr);
                }
                block_tree_init_add(&tree, new_ptr, get_block_size(new_ptr));
            }
        }
        return ptr;
    }

    // increase size
    if (size_align > get_block_size(ptr)) {
        // merge with next
        if (!get_LAB(ptr) && is_free(block_next(ptr)) &&
            get_block_size(block_next(ptr)) + get_block_size(ptr) + HEADER_SIZE >= size_align) {
            block_tree_del(&tree, block_next(ptr));

            if ((!get_LAB(block_next(ptr)) && is_free(block_next(block_next(ptr)))) ||
                get_block_size(block_next(ptr)) - (size_align - get_block_size(ptr)) >= NODE_SIZE) {
                void* splitted2 = block_split(block_next(ptr), size_align - HEADER_SIZE - get_block_size(ptr));
                if (splitted2 != NULL) {
                    if (!get_LAB(splitted2) && is_free(block_next(splitted2))) {
                        block_tree_del(&tree, block_next(splitted2));
                        block_merge(&tree, splitted2);
                    }
                    block_tree_init_add(&tree, splitted2, get_block_size(splitted2));
                }
            }

            block_merge(&tree, ptr);
            return ptr;
        }

        void* new_ptr = mem_alloc(size_align);

        if (new_ptr == NULL) {
            return NULL;
        }

        memcpy(ptr, new_ptr, get_block_size(ptr));

        mem_free(ptr);

        return new_ptr;
    }

    // equal size
    return ptr;
}

/**
 * Make memory free
 *
 * void* ptr - pointer to block for making it free
*/
void mem_free(void *ptr) {
    if (ptr != NULL) {
        set_busy(ptr, false);

        // merge free block with free next neighbour
        if (!get_LAB(ptr) && is_free(block_next(ptr))) {
            block_tree_del(&tree, block_next(ptr));
            block_merge(&tree, ptr);
        }

        // merge free block with free previous neighbour
        if (!get_FAB(ptr) && is_free(block_prev(ptr))) {
            block_tree_del(&tree, block_prev(ptr));
            block_merge(&tree, block_prev(ptr));
            ptr = block_prev(ptr);
        }

        block_tree_init_add(&tree, ptr, get_block_size(ptr));

        if (get_FAB(ptr) && get_LAB(ptr)) {
            block_tree_del(&tree, ptr);
            arena_remove(payload_to_arena(ptr));
#if DEBUG_MODE
            for (int i = 0; i < count_arena; i++)
                if (arenas[i] == ptr)
                    arenas[i] = NULL;
#endif
        }
    }
}