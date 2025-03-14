#include "my_malloc.h"  

/*
gets the size of the oversize needed for the meowchunk struct
*/
static size_t get_overhead_size() {
    size_t overhead_size = sizeof(meowchunk);

    if (overhead_size % sizeof(uintptr_t) != 0) {
        overhead_size += sizeof(uintptr_t) - overhead_size % sizeof(uintptr_t);
    }

    return overhead_size;
}

void init_meowheap(meowheap *heap, size_t size) {
    if (size % sizeof(uintptr_t) != 0) {
        size += sizeof(uintptr_t) - size % sizeof(uintptr_t);   // ensures that the size of the heap is a multiple of the size of a word
    }

    void *allocated_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    meowchunk *chunk = (meowchunk *)allocated_start;
    chunk -> next = NULL;
    chunk -> size = size - get_overhead_size();
    
    heap -> first_free = chunk;
    heap -> first_used = NULL;
    heap -> size_free = size - get_overhead_size();
    heap -> size_used = 0;
    heap -> size_overhead = get_overhead_size();
}

void *meowalloc(size_t size) {
    UNIMPLEMENTED;
}

void meowfree(void *ptr) {
    UNIMPLEMENTED;
}

int main() {
    meowheap heap;
    init_meowheap(&heap, 1024);
    return 0;
}