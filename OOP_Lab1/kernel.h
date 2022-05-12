#pragma once

// get memory from kernel
void* kernel_alloc(size_t);

// free memory using kernel
bool kernel_free(void*, size_t);

// return memory to kernel
void* kernel_return_memory(void*, size_t);