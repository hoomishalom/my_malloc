#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <sys/mman.h>

#define MAP_ANONYMOUS 0x20  // fixing a problem with VScode

#define UNIMPLEMENTED \
    do { \
        fprintf(stderr, "%s:%d: %s is not implemented yet\n", \
                __FILE__, __LINE__, __func__); \
        abort(); \
    } while(0)

// variables

struct meowchunk {
    struct meowchunk *next;
    size_t size;
};

struct meowheap {
    struct meowchunk *first_free;
    struct meowchunk *first_used;
    size_t size_used;
    size_t size_free;
    size_t size_overhead;
};

typedef struct meowchunk meowchunk;
typedef struct meowheap meowheap;



// functions

/*
inits a new heap
@param heap the heap to initialize
@param size the size of the heap
*/
void init_meowheap(meowheap *heap, size_t size);

/*
allocates memory (malloc)
@param size size of the memory to allocate
@return pointer to the allocated memory
*/
void *meowalloc(meowheap *heap, size_t size);

/*
free memory allocated by meowalloc (free)
@param ptr pointer to memory to free
*/
void meowfree(meowheap *heap, void *ptr);