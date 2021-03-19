#include "Allocator.h"

Allocator::Allocator(size_t size)
{
    HEADER_SIZE = sizeof(header);
    MEM_ALIGNMENT = 4;

    void* ptr = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    start_ptr = (uint8_t*)ptr + HEADER_SIZE;
    end_ptr = (uint8_t*)ptr + size;

    start_block = (header*)ptr;
    start_block->_prev_size = NULL;
    start_block->_size = size - HEADER_SIZE;

    set_state(start_block, false);
}

Allocator::~Allocator()
{
    VirtualFree(start_ptr, 0, MEM_RELEASE);
}

void Allocator::show(){
    uint8_t*ptr = (uint8_t*)start_ptr;
    size_t i = 0;
    printf("N\tSTATE\tHEADER\t\tSIZE\tP_SIZE\tPTR\n\n");
    while (ptr != NULL && ptr < end_ptr) {
        header *tmp_header = (header*)(ptr - HEADER_SIZE);

        printf("%d\t%c\t%p\t%d\t%d\t%p\n\n", i, (tmp_header->_size & 1)? 'B':'F', 
            tmp_header, get_header_size(tmp_header), tmp_header->_prev_size, ptr);

        ptr += get_header_size(tmp_header) + HEADER_SIZE;
        i++;
    }
}

void *Allocator::malloc(size_t size) {
    uint8_t* ptr = (uint8_t*)start_ptr;
    size_t size_align = (size % MEM_ALIGNMENT != 0)? size - (size % MEM_ALIGNMENT) + MEM_ALIGNMENT: size;

    while (ptr < end_ptr) {
        header *tmp_header = (header*)(ptr - HEADER_SIZE);
        if (is_free(tmp_header) && tmp_header->_size >= size_align) {
            // divide to Busy and Free blocks
            if (tmp_header->_size > size_align + HEADER_SIZE) {
                header* rest_free = (header*)(ptr + size_align);
                rest_free->_prev_size = size_align;
                rest_free->_size = tmp_header->_size - size_align - HEADER_SIZE;
                set_state(rest_free, false);

                if (ptr + tmp_header->_size + HEADER_SIZE < end_ptr &&
                    ptr + tmp_header->_size + HEADER_SIZE != NULL) {
                    header* tmp_next = (header*)(ptr + tmp_header->_size);
                    tmp_next->_prev_size = rest_free->_size;
                }

                tmp_header->_size = size_align;
            }
            set_state(tmp_header, true);
            return ptr;
        }

        // try next block
        ptr += get_header_size(tmp_header) + HEADER_SIZE;
    }
    return NULL;
}

void* Allocator::realloc(void* ptr, size_t size) {
    header* tmp_header = (header*)((uint8_t*)ptr - HEADER_SIZE);

    if (ptr == NULL) {
        return malloc(size);
    }

    if (size == 0) {
        return NULL;
    }

    size_t size_align = (size % MEM_ALIGNMENT != 0) ? size - (size % MEM_ALIGNMENT) + MEM_ALIGNMENT : size;
    if (size_align == get_header_size(tmp_header)) {
        return ptr;
    }
    else {
        // decrease size
        if (size_align <= get_header_size(tmp_header)) {
            return decrease_block_size(ptr, tmp_header, size_align);
        }
        // increase size
        if (size_align > get_header_size(tmp_header)) {
            return increase_block_size(ptr, tmp_header, size_align);
        }
    }
}

void* Allocator::decrease_block_size(void* prev_ptr, header* prev_header, size_t size_align) {
    if (size_align + HEADER_SIZE >= get_header_size(prev_header)) {
        return prev_ptr;
    }

    uint8_t* ptr = (uint8_t*)prev_ptr;
    header* rest_free = (header*)(ptr + size_align);
    
    rest_free->_prev_size = size_align;
    rest_free->_size = get_header_size(prev_header) - size_align - HEADER_SIZE;
    set_state(rest_free, false);

    if (ptr + get_header_size(prev_header) + HEADER_SIZE < end_ptr && 
        ptr + get_header_size(prev_header) + HEADER_SIZE != NULL) {
        header *tmp_next = (header*)(ptr + get_header_size(prev_header));
        tmp_next->_prev_size = rest_free->_size;
    }

    prev_header->_size = size_align;
    set_state(prev_header, true);

    // merge rest_free block with free neighbour
    merge_free((uint8_t*)rest_free + HEADER_SIZE, rest_free);

    return ptr;
}

void* Allocator::increase_block_size(void* prev_ptr, header* prev_header, size_t size_align) {
    uint8_t* prev_n_ptr = (uint8_t*)prev_ptr - prev_header->_prev_size - HEADER_SIZE;
    header* prev_n_header = (header*)((uint8_t*)prev_n_ptr - HEADER_SIZE);

    uint8_t* next_ptr = (uint8_t*)prev_ptr + get_header_size(prev_header) + HEADER_SIZE;
    header* next_header = (header*)(next_ptr - HEADER_SIZE);

    // merge with next
    if (next_ptr < end_ptr && next_ptr != NULL && 
        is_free(next_header) && next_header->_size + get_header_size(prev_header) >= size_align) {
        // divide to Busy and Free blocks
        if (next_header->_size + get_header_size(prev_header) > size_align) {
            header* rest_free = (header*)((uint8_t*)prev_ptr + size_align);
            rest_free->_prev_size = size_align;
            rest_free->_size = next_header->_size + get_header_size(prev_header) - size_align;
            set_state(rest_free, false);

            if (next_ptr + get_header_size(next_header) + HEADER_SIZE < end_ptr &&
                next_ptr + get_header_size(next_header) + HEADER_SIZE != NULL) {
                header* tmp_next = (header*)(next_ptr + get_header_size(next_header));
                tmp_next->_prev_size = rest_free->_size;
            }
        }
        else {
            if (next_ptr + get_header_size(next_header) + HEADER_SIZE < end_ptr &&
                next_ptr + get_header_size(next_header) + HEADER_SIZE != NULL) {
                header* tmp_next = (header*)(next_ptr + get_header_size(next_header));
                tmp_next->_prev_size = size_align;
            }
        }

        prev_header->_size = size_align;
        set_state(prev_header, true);
        return prev_ptr;
    }

    // merge with previous
    if (is_free(prev_n_header) && prev_n_header->_size + get_header_size(prev_header) >= size_align) {
        // divide to Busy and Free blocks
        if (prev_n_header->_size + get_header_size(prev_header) > size_align + HEADER_SIZE) {
            header* rest_free = (header*)((uint8_t*)prev_n_ptr + size_align);
            rest_free->_prev_size = size_align;
            rest_free->_size = prev_n_header->_size + get_header_size(prev_header) - size_align - HEADER_SIZE;
            set_state(rest_free, false);

            if ((uint8_t*)prev_ptr + get_header_size(prev_header) + HEADER_SIZE < end_ptr &&
                (uint8_t*)prev_ptr + get_header_size(prev_header) + HEADER_SIZE != NULL) {
                header* tmp_next = (header*)((uint8_t*)prev_ptr + get_header_size(prev_header));
                tmp_next->_prev_size = rest_free->_size;
            }

            prev_n_header->_size = size_align;
        }
        set_state(prev_n_header, true);

        copy_data(prev_ptr, prev_n_ptr, get_header_size(prev_header));

        return prev_n_ptr;
    }

    void* new_ptr = malloc(size_align);
    
    if (new_ptr == NULL) {
        return NULL;
    }

    copy_data(prev_ptr, new_ptr, get_header_size(prev_header));

    free(prev_ptr);

    return new_ptr;
}

void Allocator::copy_data(void* old_ptr, void* new_ptr, size_t len) {
    uint8_t delta = 0;
    uint8_t* new_data = (uint8_t*)new_ptr, * old_data = (uint8_t*)old_ptr;

    for (size_t i = 0; i < len; i++)
        *new_data = *old_data;
}

void Allocator::free(void *ptr) {
    if (ptr != NULL) {
        header* tmp_header = (header*)((uint8_t*)ptr - HEADER_SIZE);
        set_state(tmp_header, false);

        // merge free block with free neighbour
        merge_free((uint8_t*)ptr, tmp_header);
    }
}

void Allocator::merge_free(void* current_ptr, header* current_header) {
    // merge with next
    header* next_header = (header*)((uint8_t*)current_ptr + get_header_size(current_header));
    if (next_header + HEADER_SIZE < end_ptr && next_header + HEADER_SIZE != NULL && is_free(next_header)) {
        current_header->_size += next_header->_size + HEADER_SIZE;

        if ((uint8_t*)current_ptr + get_header_size(current_header) + HEADER_SIZE < end_ptr &&
            (uint8_t*)current_ptr + get_header_size(current_header) + HEADER_SIZE != NULL) {
            header *tmp_next = (header*)((uint8_t*)current_ptr + get_header_size(current_header));
            tmp_next->_prev_size = current_header->_size;
        }
    }

    // merge with previous
    uint8_t* prev_ptr = (uint8_t*)current_ptr - current_header->_prev_size - HEADER_SIZE;
    header* prev_header = (header*)(prev_ptr - HEADER_SIZE);
    if (prev_ptr >= start_ptr && is_free(prev_header))
        merge_free(prev_ptr, prev_header);
}

// set block busyness: 0 - free, 1 - busy
void Allocator::set_state(header *header, bool state) {
    if (state) // set 1
        header->_size |= 1;
    else // set 0
        header->_size &= ~1;
}

bool Allocator::is_free(header *header) {
    return !(header->_size & 1);
}

size_t Allocator::get_header_size(header *header) {
    return  header->_size & ~1;
}
