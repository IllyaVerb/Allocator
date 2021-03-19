#include <iostream>
#include <time.h>
#include "Allocator.h"

void tester(size_t allocator_mem, size_t random_mem, int iterations) {
    Allocator my_allocator = Allocator(allocator_mem);
    void *ptr_arr[10000] = { NULL };
    int iterator = 1;

    for (int i = 0; i < iterations; i++) {
        void* tmp_ptr;
        switch (rand() % 3) {
        case 0: ptr_arr[rand() % iterator] = my_allocator.malloc(rand() % (random_mem+1)); 
                iterator++; break;

        case 1: my_allocator.free(ptr_arr[rand() % iterator]); 
                break;

        case 2: int tmp_iter = rand() % iterator;
                ptr_arr[tmp_iter] = my_allocator.realloc(ptr_arr[tmp_iter], rand() % (random_mem+1));
        }
        my_allocator.show(); 
        cout << endl;
    }
    /*void *p = my_allocator.malloc(12);
    void *q = my_allocator.malloc(20);
    void *pp = my_allocator.malloc(8);
    my_allocator.free(q);
    my_allocator.realloc(q, 8);*/
}

int main()
{
    srand(time(NULL));
    tester(4096*10, 1024, 1000);
    return 0;
}
