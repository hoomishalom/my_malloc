#include "../headers/my_malloc.h"  

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

/*
print stats
*/
static void print_stats(meowheap *heap) 
{
    printf("free memory:        %d\n", heap -> size_free);
    printf("used memory:        %d\n", heap -> size_used);
    printf("overhead memory:    %d\n", heap -> size_overhead);
    printf("total:              %d\n", heap -> size_overhead + heap -> size_used + heap -> size_free);
}

/*
prints all chunk sizes
*/
static void print_sizes(meowchunk *start) 
{
    if (!start) return;

    printf("Sizes: ");
    while (start -> next) {
        printf("%zu -> ", start -> size);
        start = start -> next;
    }
    printf("%zu\n", start -> size);
}

/*
aligns sizes
*/
static void align(size_t *size) 
{
    if (*size % sizeof(uintptr_t) != 0) {
        *size += sizeof(uintptr_t) - *size % sizeof(uintptr_t);   // ensures that the size of the heap is a multiple of the size of a word
    }
}

/*
checks if a chunk is splitable, used when allocating a chunk
*/
static int is_splitable(meowchunk *chunk, size_t size) 
{
    if (chunk -> size >= size + get_overhead_size()) {
        return 1;
    }
    return 0;
}

/*
inserts to free list, keeps sorted by address
@return previous chunk in free list
*/
static meowchunk* insert_free(meowheap *heap, meowchunk *chunk) 
{
    meowchunk *prev = NULL;
    meowchunk *iterator = heap -> first_free;
    
    while(iterator && (iterator < chunk)) {
        prev = iterator;
        iterator = iterator -> next;
    }

    if (prev == NULL) {
        chunk -> next = heap -> first_free;
        heap -> first_free = chunk;
    } else {
        chunk -> next = iterator;
        prev -> next = chunk;
    }

    return prev;
}

/*
removes chunk from used list
*/
static void remove_used(meowheap *heap, meowchunk *chunk) {
    meowchunk *prev = NULL;
    meowchunk *iterator = heap -> first_used;

    while(iterator != chunk) {
        prev = iterator;
        iterator = iterator -> next;
    }

    if (prev) {
        prev -> next = iterator -> next;
    } else {
        heap -> first_used = iterator -> next;
    }
}

// functions

void init_meowheap(meowheap *heap, size_t size) 
{
    align(&size);

    void *allocated_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    meowchunk *chunk_free = (meowchunk *)allocated_start;
    chunk_free -> next = NULL;
    chunk_free -> size = size - get_overhead_size();
    
    heap -> first_free = chunk_free;
    heap -> first_used = NULL;
    heap -> size_free = chunk_free -> size;
    heap -> size_used = 0;
    heap -> size_overhead = get_overhead_size();
}

void *meowalloc(meowheap *heap, size_t size) 
{
    align(&size);

    if (heap -> size_free < size) {   // makes sure there is enough memory to allocate and to create the next chunk
        fprintf(stderr, "%s:%d - not enough memory to allocate", __func__, __LINE__);
        return NULL;
    }

    size_t min_fitting_size = SIZE_MAX;
    meowchunk *prev = NULL;
    meowchunk *chunk = NULL;
    meowchunk *iterator = heap -> first_free;

    while (iterator) {
        if (iterator -> size < size || iterator -> size > min_fitting_size) {   // too small or worse fit
            iterator = iterator -> next;
        } else {    // found better fitting
            min_fitting_size = iterator -> size;
            chunk = iterator;
            iterator = iterator -> next;
        }
    }

    while (iterator && iterator != chunk) {
        prev = iterator;
        iterator = iterator -> next;
    }

    if (chunk -> size > size) {
        if (is_splitable(chunk, size)) {
            meowchunk *splitted_chunk = (meowchunk *)((char *)chunk + get_overhead_size() + size);  // splitted chunk 
            splitted_chunk -> next = chunk -> next;
            splitted_chunk -> size = chunk -> size - size - get_overhead_size();
            
            meowchunk *returned_chunk = chunk; // create chunk
            returned_chunk -> next = heap -> first_used;    // insert to used list
            heap -> first_used = returned_chunk;
            returned_chunk -> size = size;

            if (prev) {
                prev -> next = splitted_chunk;
            } else {
                heap -> first_free = splitted_chunk;
            }

            heap -> size_free -= (size + get_overhead_size());  // used chunk + overhead for new splitted chunk
            heap -> size_used += size;                          // used chunk
            heap -> size_overhead += get_overhead_size();       // overhead for new splitted chunk

            return (void *)((char *)returned_chunk + get_overhead_size());    // pointer to the actual data storage
        } else {
            meowchunk *returned_chunk = chunk;

            if (prev) {
                prev -> next = chunk -> next;
            } else {
                heap -> first_free = chunk -> next;
            }
            
            returned_chunk -> next = heap -> first_used;    // insert to used list
            heap -> first_used = returned_chunk;
            
            heap -> size_free -= chunk -> size;     // used chunk
            heap -> size_used += chunk -> size;     // used chunk
            heap -> size_overhead += 0;             // no need for more overhead if not splitting :)
            
            return (void *)((char *)returned_chunk + get_overhead_size());    // pointer to the actual data storage
        }

    }
    
    fprintf(stderr, "%s:%d - did not find a large enough chunk", __func__, __LINE__);
    return NULL;
}

void meowfree(meowheap *heap, void *ptr) 
{
    meowchunk *chunk = (meowchunk *)((char *)ptr - get_overhead_size());    // gets chunk form
    
    remove_used(heap, chunk);
    meowchunk *prev = insert_free(heap, chunk);
    
    heap -> size_used -= chunk -> size;
    heap -> size_free += chunk -> size;
    
    // try to coalesce chunks with next
    if (chunk -> next && (meowchunk *)((char *)chunk + chunk -> size + get_overhead_size()) == chunk -> next) {
        chunk -> size += chunk -> next -> size + get_overhead_size();
        chunk -> next = chunk -> next -> next;
        
        heap -> size_free += get_overhead_size();
        heap -> size_overhead -= get_overhead_size();
    }
    
    // try to coalesce chunks with prev
    if (prev && (meowchunk *)((char *)prev + prev -> size + get_overhead_size()) == chunk) {
        prev -> size += chunk -> size + get_overhead_size();
        prev -> next = chunk -> next;
        
        heap -> size_free += get_overhead_size();
        heap -> size_overhead -= get_overhead_size();
    }
}

int main() 
{
    meowheap heap;
    init_meowheap(&heap, 1024);
    meowchunk *ptr1 = (meowchunk *)meowalloc(&heap, 128);
    meowchunk *ptr2 = (meowchunk *)meowalloc(&heap, 128);
    meowchunk *ptr3 = (meowchunk *)meowalloc(&heap, 128);
    
    meowfree(&heap, ptr1);
    meowfree(&heap, ptr2);
    meowfree(&heap, ptr3);

    print_stats(&heap);
    return 0;
}