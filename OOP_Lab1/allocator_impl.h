#pragma once
#define MEMORY_ALIGNMENT                (alignof(max_align_t))
#define ROUND_BYTES_GENERIC(s)          ((((s) + (MEMORY_ALIGNMENT - 1)) / MEMORY_ALIGNMENT) * MEMORY_ALIGNMENT)
#define ROUND_BYTES(s)                  (((s) + (MEMORY_ALIGNMENT - 1)) & ~(MEMORY_ALIGNMENT - 1))