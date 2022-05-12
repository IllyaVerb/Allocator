#if defined(_WIN32) || defined(_WIN64)

#include "windows.h"
#include "kernel.h"

void* kernel_alloc(size_t size) {
	return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

bool kernel_free(void* ptr, size_t size) {
	(void)size;
	return (bool)VirtualFree(ptr, 0, MEM_RELEASE);
}

void* kernel_return_memory(void* ptr, size_t size) {
	return VirtualAlloc(ptr, size, MEM_RESET, PAGE_READWRITE);
}

#else

#include "sys/mman.h"
#include "kernel.h"

#if defined(MAP_ANONYMOUS)
#	define FLAG_ANON MAP_ANONYMOUS
#elif defined(MAP_ANON)
#	define FLAG_ANON MAP_ANON
#else
#	error Incorrect operating system (NO Windows & NO Unix)
#endif

void* kernel_alloc(size_t size) {
	void* ptr = mmap(NULL, length, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return ptr == MAP_FAILED ? NULL : ptr;
}

bool kernel_free(void* ptr, size_t size) {
	return munmap(ptr, size) == -1 ? false : true;
}

void* kernel_return_memory(void* ptr, size_t size) {
	return madvise(ptr, size, MADV_DONTNEED) == -1 ? NULL : ptr;
}

#endif