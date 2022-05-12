#ifndef BLOCK_H
#define BLOCK_H
#include <cstddef>
#include "allocator_impl.h"
#include "tree.h"

#define SIZE_T_SIZE_BIT                 (sizeof(size_t) << 3)
#define FIRST_BIT                       ((size_t)1 << (SIZE_T_SIZE_BIT - 1))

#define HEADER_SIZE                     ROUND_BYTES(sizeof(header*))

// header structure of allocated block
typedef struct header {
    // _size first bit is business flag:                0 - free    1 - busy
    // _prev_size first bit is last arena block flag:   0 - not     1 - is
    size_t _size;
    size_t _prev_size;
} header;

// prototypes for block.cpp file
void* block_split(void*, size_t);
void block_merge(block_tree*, void*);

// methods to go between block and payload pointers
inline void* block_to_payload(header* block) { return (char*)block + HEADER_SIZE; };
inline header* payload_to_block(void* ptr) { return (header*)((char*)ptr - HEADER_SIZE); };

// methods to go between block and tree node pointers
inline void* block_to_node(header* block) { return block_to_payload(block); }
inline header* node_to_block(void* node) { return payload_to_block(node); }

// methods to go between arena and payload pointers
inline void* arena_to_payload(header* arena) { return block_to_payload(arena); };
inline header* payload_to_arena(void* ptr) { return payload_to_block(ptr); };

// set some state
inline void set_state(size_t *size, bool state, size_t bit) { *size = state ? *size | bit : *size & ~(bit); };

// setter and getter for business flag
inline void set_busy(void* ptr, bool state) { set_state(&(payload_to_block(ptr)->_size), state, FIRST_BIT); };
inline bool is_free(void* ptr) { return !((payload_to_block(ptr)->_size & FIRST_BIT) >> (SIZE_T_SIZE_BIT-1)); };

// setter and getter for last arena block (LAB) flag
inline void set_LAB(void* ptr, bool state) { set_state(&(payload_to_block(ptr)->_prev_size), state, FIRST_BIT); };
inline bool get_LAB(void* ptr) { return (payload_to_block(ptr)->_prev_size & FIRST_BIT); };

// setter and getter for block size
inline size_t get_block_size(void* ptr) { return (size_t)(payload_to_block(ptr)->_size & ~(FIRST_BIT)) * MEMORY_ALIGNMENT; };
inline void set_block_size(void* ptr, size_t size) {
    bool state = !is_free(ptr);
    size_t tmp = size / MEMORY_ALIGNMENT;
    payload_to_block(ptr)->_size = size / MEMORY_ALIGNMENT;
    header* hd = payload_to_block(ptr);
    set_busy(ptr, state);
};

// setter and getter for previous block size
inline size_t get_block_prev_size(void* ptr) { return (size_t)(payload_to_block(ptr)->_prev_size & ~(FIRST_BIT)) * MEMORY_ALIGNMENT; };
inline void set_block_prev_size(void* ptr, size_t size) {
    bool    lab = get_LAB(ptr);
    payload_to_block(ptr)->_prev_size = size / MEMORY_ALIGNMENT;
    set_LAB(ptr, lab);
};

// getter for first arena block (FAB) flag
//inline void set_FAB(void* ptr, bool state) { set_state(&(payload_to_block(ptr)->_prev_size), state, FIRST_BIT); };
inline bool get_FAB(void* ptr) { return !get_block_prev_size(ptr); };

// methods to go to neighbours
inline void* block_next(void* ptr) { return (char*)ptr + get_block_size(ptr) + HEADER_SIZE; };
inline void* block_prev(void* ptr) { return (char*)ptr - get_block_prev_size(ptr) - HEADER_SIZE; };

// all setters in one method
inline void setters(void* ptr, size_t size, size_t prev_size, bool busy, bool fab, bool lab) {
    set_block_prev_size(ptr, prev_size);
    set_block_size(ptr, size);
    set_busy(ptr, busy);
    set_LAB(ptr, lab);
};
#endif