#include "block.h"

/**
 * Split block to two blocks
 * first with particular size and second with rest of it
 * return pointer to second block or NULL if cannot divide
 * 
 * void* ptr - pointer to big, or first block
 * size_t size - size which is needed on first block
*/
void* block_split(void* ptr, size_t size) {
    if (HEADER_SIZE >= get_block_size(ptr) - size) {
        return NULL;
    }

    void* new_ptr = (char*)ptr + size + HEADER_SIZE;

    setters(new_ptr, get_block_size(ptr) - size - HEADER_SIZE, size, false, false, false);

    if (get_LAB(ptr)) {
        set_LAB(new_ptr, true);
        set_LAB(ptr, false);
    }
    else
        set_block_prev_size(block_next(new_ptr), get_block_size(new_ptr));

    set_block_size(ptr, size);

    return new_ptr;
}

/**
 * Merge free block with right block
 * 
 * void* ptr - pointer to block
*/
void block_merge(block_tree* tree, void* ptr) {
    void* next_block = block_next(ptr);

    set_LAB(ptr, get_LAB(next_block));
    set_block_size(ptr, get_block_size(ptr) + get_block_size(next_block) + HEADER_SIZE);

    if (!get_LAB(ptr))
        set_block_prev_size(block_next(ptr), get_block_size(ptr));
  
    // merge with left
    //if (!get_FAB(ptr) && is_free(block_prev(ptr)))
    //    block_merge(tree, block_prev(ptr));
    
}