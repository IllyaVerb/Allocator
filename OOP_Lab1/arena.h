#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>

#include "config.h"
#include "block.h"

#if ARENA_PAGES >= SIZE_MAX / PAGE_SIZE
# define ARENA_SIZE SIZE_MAX
# define ARENA_BLOCK_SIZE_MAX (ARENA_SIZE - HEADER_SIZE - MEMORY_ALIGNMENT + 1)
#else
# define ARENA_SIZE ((size_t)PAGE_SIZE * ARENA_PAGES)
# define ARENA_BLOCK_SIZE_MAX (ARENA_SIZE - HEADER_SIZE)
#endif

void* arena_create(size_t, block_tree*);
void arena_remove(header*);

#endif // ARENA_H