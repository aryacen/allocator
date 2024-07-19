# Allocator: A Simple Sub-Allocator

## Overview
This project is an implementation of a simple memory allocator in C, designed as part of the COMP1521 course. The allocator manages memory allocation and deallocation within a simulated heap environment. This assignment was awarded a 100% mark, reflecting its completeness and accuracy.

## Features
- **Heap Initialization**: Sets up a memory heap with a specified size, ensuring alignment and minimum size constraints.
- **Memory Allocation**: Allocates chunks of memory from the heap, handling requests of various sizes and managing free space efficiently.
- **Memory Deallocation**: Frees allocated memory and merges adjacent free chunks to minimize fragmentation.
- **Debugging Support**: Includes functions to print the heap's state for testing and debugging purposes.

## Functions
1. **init_heap**: Initializes the heap with a given size, setting up the free list and heap metadata.
2. **my_malloc**: Allocates a chunk of memory large enough to store the specified number of bytes, handling splitting of free chunks if necessary.
3. **my_free**: Deallocates a chunk of memory, inserting it back into the free list and merging adjacent free chunks.
4. **free_heap**: Releases resources associated with the heap.
5. **heap_offset**: Returns the offset of a pointer from the start of the heap if it is within the heap, or -1 otherwise.
6. **dump_heap**: Prints the contents of the heap for debugging purposes, with varying levels of verbosity.

### Helper Functions
- **round_up**: Rounds up a size to the next multiple of 4.
- **minimum_heap_size**: Ensures the heap size is at least the minimum required and rounded up to the next multiple of 4.
- **minimum_index**: Finds the minimum index in the free list that can satisfy a given allocation request.
- **insert_chunk**: Inserts a new free chunk into the free list in the correct position.
- **merge_adjacent_chunk**: Merges adjacent free chunks to reduce fragmentation.

## How to Run
To run this allocator, you need a C compiler such as GCC. Compile the code using the provided `allocator.h` header file and link it with your test program. Use the functions provided to initialize the heap, allocate memory, and free memory as needed.

### Example Usage
```c
#include "allocator.h"

int main() {
    init_heap(4096);       // Initialize heap with 4096 bytes
    void *ptr1 = my_malloc(100); // Allocate 100 bytes
    void *ptr2 = my_malloc(200); // Allocate 200 bytes
    my_free(ptr1);         // Free the first allocation
    my_free(ptr2);         // Free the second allocation
    dump_heap(1);          // Print the heap state
    free_heap();           // Release all resources
    return 0;
}
