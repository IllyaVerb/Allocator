#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef>
#include <stdint.h>
#include <stdio.h>
#include "windows.h"

using namespace std;

// header structure of allocated block
struct header{
    // size first bit is business flag: 0 - free  1 - busy
    size_t _size;
    size_t _prev_size;
};

class Allocator
{
    public:
        Allocator(size_t n_cells);
        virtual ~Allocator();
        void *malloc(size_t size);
        void *realloc(void *ptr, size_t size);
        void free(void *ptr);
        void show();

    protected:

    private:
        size_t HEADER_SIZE; // structure size
        size_t MEM_ALIGNMENT; // memory alignment
        void *start_ptr;
        void *end_ptr;
        header *start_block;

        void set_state(header *header, bool state);
        bool is_free(header *header);
        size_t get_header_size(header* header);
        void* decrease_block_size(void *ptr, header *header, size_t size_align);
        void* increase_block_size(void *ptr, header *header, size_t size_align);
        void merge_free(void* ptr, header *header);
        void copy_data(void* old_ptr, void* new_ptr, size_t len);
};

#endif // ALLOCATOR_H
