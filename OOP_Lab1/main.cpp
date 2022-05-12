#include <iostream>
#include <time.h>

#include "Allocator.h"
#include "arena.h"

#define G   *(1073741824)
#define M   *(1048576)
#define K   *(1024)

using namespace std;

const int PTR_ARR_SIZE = 10000;

struct mem_block {
    void* _ptr;
    size_t _size;
    uint32_t _checksum;
};

static uint32_t checksum(void* ptr, size_t size);
static uint32_t fill_data(void* ptr, size_t size);
static void tester(size_t random_mem, int iterations);

int main()
{
    time_t t = time(NULL);
    srand(t);

#if 0
    void* tt = malloc(sizeof(header));
    setters(tt, ROUND_BYTES_GENERIC(12345), ROUND_BYTES_GENERIC(5431), 1, 1, 1);
    printf("%10d |%10c |%10p |%10llu |%10llu |%10p |%10d |%10d\n",
        0,
        is_free(tt) ? 'F' : 'B',
        payload_to_block(tt),
        get_block_size(tt),
        get_block_prev_size(tt),
        tt,
        get_FAB(tt),
        get_LAB(tt));
#endif
#if 0
    void *p = mem_alloc(100 M);
    mem_tree_show(); mem_show();

    void *q = mem_realloc(p, 1 G);
    mem_tree_show(); mem_show();

    /*void *pp = mem_alloc(8);
    mem_tree_show();

    mem_free(p);
    mem_tree_show();

    mem_free(q);
    mem_tree_show();

    void *ppp = mem_alloc(41);
    mem_tree_show();

    mem_realloc(ppp, 96);
    mem_tree_show();*/
#endif

    tester(5 K, 1000);
    return 0;
}

static uint32_t checksum(void* ptr, size_t size) {
    uint32_t crc = 0;
    char* tmp_ptr = (char*)ptr;

    for (size_t i = 0; i < size; i++) {
        crc += *(tmp_ptr + i) * 211;
    }

    return crc;
}

static uint32_t fill_data(void* ptr, size_t size) {
    uint32_t crc = 0;
    char* tmp_ptr = (char*)ptr;

    for (size_t i = 0; i < size; i++) {
        *(tmp_ptr + i) = rand() % 255;
        crc += *(tmp_ptr + i) * 211;
    }

    return crc;
}

static void tester(size_t random_mem, int iterations) {
    mem_block test_arr[PTR_ARR_SIZE] = {};

    for (int i = 0; i < iterations; i++) {
        int rand_ptr = rand() % PTR_ARR_SIZE;

        switch (rand() % 3) {
        case 0: {
            size_t mem = rand() % (random_mem + 1);
            void* tmp = mem_alloc(mem);

            if (tmp != NULL) {
                test_arr[rand_ptr]._size = mem;
                test_arr[rand_ptr]._ptr = tmp;
                test_arr[rand_ptr]._checksum = fill_data(test_arr[rand_ptr]._ptr, test_arr[rand_ptr]._size);
            }
            break;
        }

        case 1: {
            if (test_arr[rand_ptr]._ptr != NULL) {
                uint32_t chck = checksum(test_arr[rand_ptr]._ptr, test_arr[rand_ptr]._size);
                if (test_arr[rand_ptr]._checksum != chck) {
                    printf("ERROR! INCORRECT CHECKSUM: \n%d -- %d\n", test_arr[rand_ptr]._checksum, chck);
                    return;
                }
            }
            mem_free(test_arr[rand_ptr]._ptr); 
            test_arr[rand_ptr]._ptr = NULL;
            break;
        }

        case 2: {
            if (test_arr[rand_ptr]._ptr != NULL) {
                uint32_t chck = checksum(test_arr[rand_ptr]._ptr, test_arr[rand_ptr]._size);
                if (test_arr[rand_ptr]._checksum != chck) {
                    printf("ERROR! INCORRECT CHECKSUM: \n%d -- %d\n", test_arr[rand_ptr]._checksum, chck);
                    return;
                }
            }
            size_t mem = rand() % (random_mem + 1);
            void* tmp = mem_realloc(test_arr[rand_ptr]._ptr, mem);
            if (tmp != NULL) {
                test_arr[rand_ptr]._size = mem;
                test_arr[rand_ptr]._ptr = tmp;
                test_arr[rand_ptr]._checksum = fill_data(test_arr[rand_ptr]._ptr, test_arr[rand_ptr]._size);
            }
        }
        }
        if (i > iterations - 2) {
            mem_tree_show();
            mem_show();
        }
    }
}