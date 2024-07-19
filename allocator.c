////////////////////////////////////////////////////////////////////////////////
// COMP1521 22T1 --- Assignment 2: `Allocator', a simple sub-allocator        //
// <https://www.cse.unsw.edu.au/~cs1521/22T1/assignments/ass2/index.html>     //
//                                                                            //
// Written by Arya Bodhi Gunawan (z5240037) on 10 APRIL 2022.                 //
//                                                                            //
// 2021-04-06   v1.0    Team COMP1521 <cs1521 at cse.unsw.edu.au>             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocator.h"

// DO NOT CHANGE CHANGE THESE #defines

/** minimum total space for heap */
#define MIN_HEAP 4096

/** minimum amount of space to split for a free chunk (excludes header) */
#define MIN_CHUNK_SPLIT 32

/** the size of a chunk header (in bytes) */
#define HEADER_SIZE (sizeof(struct header))

/** constants for chunk header's status */
#define ALLOC 0x55555555
#define FREE 0xAAAAAAAA

// ADD ANY extra #defines HERE

// DO NOT CHANGE these struct defintions

typedef unsigned char byte;

/** The header for a chunk. */
typedef struct header {
    uint32_t status; /**< the chunk's status -- shoule be either ALLOC or FREE */
    uint32_t size;   /**< number of bytes, including header */
    byte     data[]; /**< the chunk's data -- not interesting to us */
} header_type;


/** The heap's state */
typedef struct heap_information {
    byte      *heap_mem;      /**< space allocated for Heap */
    uint32_t   heap_size;     /**< number of bytes in heap_mem */
    byte     **free_list;     /**< array of pointers to free chunks */
    uint32_t   free_capacity; /**< maximum number of free chunks (maximum elements in free_list[]) */
    uint32_t   n_free;        /**< current number of free chunks */
} heap_information_type;

// Footnote:
// The type unsigned char is the safest type to use in C for a raw array of bytes
//
// The use of uint32_t above limits maximum heap size to 2 ** 32 - 1 == 4294967295 bytes
// Using the type size_t from <stdlib.h> instead of uint32_t allowing any practical heap size,
// but would make struct header larger.


// DO NOT CHANGE this global variable
// DO NOT ADD any other global  variables

/** Global variable holding the state of the heap */
static struct heap_information my_heap;

// ADD YOUR FUNCTION PROTOTYPES HERE
uint32_t round_up(uint32_t size);
uint32_t minimum_heap_size(uint32_t size);
int minimum_index(uint32_t size);
void insert_chunk(uintptr_t chunk);
void merge_adjacent_chunk(void);

// Initialise my_heap
int init_heap(uint32_t size) {
    // Round up the size
    size = minimum_heap_size(size);

    // Allocate memory for the heap
    my_heap.heap_mem = malloc(size * HEADER_SIZE);
    if (my_heap.heap_mem == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }

    // Enter the size and status of the heap
    ((header_type *) my_heap.heap_mem)->size = size;
    ((header_type *) my_heap.heap_mem)->status = FREE;

    // Initialise the heap
    uint32_t array_size = size / HEADER_SIZE;
    my_heap.free_capacity = array_size;
    my_heap.heap_size = size;
    my_heap.n_free = 1;

    // Allocate memory for the free_list and initialise it
    my_heap.free_list = malloc(my_heap.free_capacity * sizeof(my_heap.free_list));
    if (my_heap.free_list == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return -1;
    }
    my_heap.free_list[0] = my_heap.heap_mem;

    return 0;
}


// Allocate a chunk of memory large enough to store `size' bytes
void *my_malloc(uint32_t size) {
    if (size < 1) {
        return NULL;
    }

    size = round_up(size);
    // find the minimum index available on the free_list
    int index = minimum_index(size);

    header_type *chunk = (header_type *) (my_heap.free_list[index]);
    uint32_t chunk_size = chunk->size;
    // set the status of the chunk as allocated
    chunk->status = ALLOC;
    // if chunk size is less than size + header_size + min_chunk_split then
    // allocate the whole chunk
    if (chunk_size < (size + HEADER_SIZE + MIN_CHUNK_SPLIT)) {
        while (index < (my_heap.free_capacity - 1)) {
            my_heap.free_list[index] = my_heap.free_list[index + 1];
            index++;
        }
        my_heap.free_list[index] = NULL;
        my_heap.n_free--;
    // if the chunk size is more than size + header_size + min_chunk_split then
    // split it into 2 chunks. set the new chunk to free
    } else {
        chunk->size = size + HEADER_SIZE;
        header_type *new_chunk = (header_type *) (my_heap.free_list[index] +
                                (size + HEADER_SIZE));
        new_chunk->status = FREE;
        new_chunk->size = chunk_size - (size + HEADER_SIZE);
        my_heap.free_list[index] = (byte *)new_chunk;
    }

    void *output = (void *)(chunk + 1);
    return output;
}


// Deallocate chunk of memory referred to by `ptr'
void my_free(void *ptr) {
    // if ptr is equals to NULL, then memory is unallocated
    if (ptr == NULL) {
        fprintf(stderr, "Attempt to free unallocated chunk\n");
        exit(EXIT_FAILURE);
    }
    
    header_type *chunk = (header_type *) ptr - 1;

    // if chunk's status is FREE, then there is no need to free it
    if (chunk->status == FREE) {
        fprintf(stderr, "Attempt to free unallocated chunk\n");
        exit(EXIT_FAILURE);
    }

    // set the chunk's status is free if it's allocated
    chunk->status = FREE;

    // inserts the free chunk and merges the adjacent free chunks
    insert_chunk((uintptr_t) chunk);
    merge_adjacent_chunk();

}


// DO NOT CHANGE CHANGE THiS FUNCTION
//
// Release resources associated with the heap
void free_heap(void) {
    free(my_heap.heap_mem);
    free(my_heap.free_list);
}


// DO NOT CHANGE CHANGE THiS FUNCTION

// Given a pointer `obj'
// return its offset from the heap start, if it is within heap
// return -1, otherwise
// note: int64_t used as return type because we want to return a uint32_t bit value or -1
int64_t heap_offset(void *obj) {
    if (obj == NULL) {
        return -1;
    }
    int64_t offset = (byte *)obj - my_heap.heap_mem;
    if (offset < 0 || offset >= my_heap.heap_size) {
        return -1;
    }

    return offset;
}


// DO NOT CHANGE CHANGE THiS FUNCTION
//
// Print the contents of the heap for testing/debugging purposes.
// If verbosity is 1 information is printed in a longer more readable form
// If verbosity is 2 some extra information is printed
void dump_heap(int verbosity) {

    if (my_heap.heap_size < MIN_HEAP || my_heap.heap_size % 4 != 0) {
        printf("ndump_heap exiting because my_heap.heap_size is invalid: %u\n", my_heap.heap_size);
        exit(1);
    }

    if (verbosity > 1) {
        printf("heap size = %u bytes\n", my_heap.heap_size);
        printf("maximum free chunks = %u\n", my_heap.free_capacity);
        printf("currently free chunks = %u\n", my_heap.n_free);
    }

    // We iterate over the heap, chunk by chunk; we assume that the
    // first chunk is at the first location in the heap, and move along
    // by the size the chunk claims to be.

    uint32_t offset = 0;
    int n_chunk = 0;
    while (offset < my_heap.heap_size) {
        struct header *chunk = (struct header *)(my_heap.heap_mem + offset);

        char status_char = '?';
        char *status_string = "?";
        switch (chunk->status) {
        case FREE:
            status_char = 'F';
            status_string = "free";
            break;

        case ALLOC:
            status_char = 'A';
            status_string = "allocated";
            break;
        }

        if (verbosity) {
            printf("chunk %d: status = %s, size = %u bytes, offset from heap start = %u bytes",
                    n_chunk, status_string, chunk->size, offset);
        } else {
            printf("+%05u (%c,%5u) ", offset, status_char, chunk->size);
        }

        if (status_char == '?') {
            printf("\ndump_heap exiting because found bad chunk status 0x%08x\n",
                    chunk->status);
            exit(1);
        }

        offset += chunk->size;
        n_chunk++;

        // print newline after every five items
        if (verbosity || n_chunk % 5 == 0) {
            printf("\n");
        }
    }

    // add last newline if needed
    if (!verbosity && n_chunk % 5 != 0) {
        printf("\n");
    }

    if (offset != my_heap.heap_size) {
        printf("\ndump_heap exiting because end of last chunk does not match end of heap\n");
        exit(1);
    }

}

// ADD YOUR EXTRA FUNCTIONS HERE

// This function rounds up the size to the next multiple of 4
uint32_t round_up(uint32_t size) {
    if (size % 4 == 0) {
        return size;
    } else {
        return ((size + 4) - (size % 4));
    }
}


// This function sets the size to be the minimum heap size if it is less
// than 4096. The value of size will also be rounded up to the next
// multiple of 4.
uint32_t minimum_heap_size(uint32_t size) {

    // sets size to be the minimum heap value if the size is less than it
    if (size < MIN_HEAP) {
        return MIN_HEAP;
    }

    // round up the size to the next multiple of 4
    return (round_up(size));
    
}

// This function finds the minimum index available in the free_list to allocate
// the memory
int minimum_index(uint32_t size) {
    int min = -1;
    for (int i = 0; i < my_heap.n_free; i++) {
        header_type *curr = (header_type *) my_heap.free_list[i];
        // if min is less than 0, then minimum index is not set yet
        if (min < 0 && curr->size >= (size + HEADER_SIZE)) {
            min = i;
        // if min is greater than 0, then check if size of current is less than
        // the minimum
        } else if (min >= 0 && curr->size >= (size + HEADER_SIZE)) {
            header_type *headr = (header_type *) my_heap.free_list[min];
            if (curr->size < headr->size) {
                min = i;
            }
        }
    }
    // if min is less than 0 after the loop, then there is no free chunk available
    if (min < 0) {
        fprintf(stderr, "Could not find free chunk\n");
        exit(EXIT_FAILURE);
    }
    return min;
}

// This function inserts a new free chunk
void insert_chunk(uintptr_t chunk) {
    int i;
    for (i = my_heap.n_free - 1; (i >= 0 && (uintptr_t) my_heap.free_list[i] > chunk); i--) {
        my_heap.free_list[i+1] = my_heap.free_list[i];
 
    }
    my_heap.free_list[i+1] = (byte *)chunk;
    my_heap.n_free++;
}

// This function merges the adjacent free chunks
void merge_adjacent_chunk(void) {
    for (int i = 0; i < my_heap.n_free - 1; i++) {
        header_type *curr = (header_type *) my_heap.free_list[i];
        header_type *next = (header_type *) my_heap.free_list[i+1];

        // when the chunks are adjacent, then
        while ((uintptr_t) ((byte *)curr + curr->size) == (uintptr_t) next) {
            // increments the size of the free chunks to current and set it to free
            curr->size += next->size;
            next->status = FREE;
            // deletes the individual free chunks
            for (int j = i+1; j < my_heap.n_free - 1; j++) {
                my_heap.free_list[j] = my_heap.free_list[j+1];
            }
            next = (header_type *)my_heap.free_list[i+1];
            my_heap.n_free--;
        }
    }
}
