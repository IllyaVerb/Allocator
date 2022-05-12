#ifndef ALLOCATOR_H
#define ALLOCATOR_H

void* mem_alloc(size_t);
void* mem_realloc(void*, size_t);
void mem_free(void*);
void mem_show();
void mem_tree_show();

static void print_tree_data(size_t id, void* ptr);

#endif // ALLOCATOR_H
