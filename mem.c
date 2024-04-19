// Student information
// Student : [First Name] [Last Name] - [Student Number]
// Student : [First Name] [Last Name] - [Student Number]
// Student : [First Name] [Last Name] - [Student Number]

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define E_NO_SPACE            1
#define E_CORRUPT_FREESPACE   2
#define E_PADDING_OVERWRITTEN 3
#define E_BAD_ARGS            4
#define E_BAD_POINTER         5

#define M_BESTFIT   0
#define M_WORSTFIT  1
#define M_FIRSTFIT  2

int m_error;

// data structure definition

typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

static Block *free_list = NULL;

static void *heap_start = NULL;
static int heap_size = 0;

int mem_error = 0;

int mem_init(int size_of_region) {
    if (size_of_region <= 0) {
        mem_error = E_BAD_ARGS;
        fprintf(stderr, "Error: Invalid size_of_region, mem_error = %d\n", mem_error);
        return -1;
    }

    // Round up size_of_region to page size
    size_of_region = (size_of_region + getpagesize() - 1) & ~(getpagesize() - 1);

    // Request memory from the OS
    heap_start = mmap(NULL, size_of_region, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap_start == MAP_FAILED) {
        mem_error = E_BAD_ARGS;
        fprintf(stderr, "Error: mmap failed, mem_error = %d\n", mem_error);
        return -1;
    }

    heap_size = size_of_region;
    // Initialize your data structures here

    // Initially, the entire region is free
    free_list = (Block *)heap_start;
    free_list->size = heap_size - sizeof(Block);
    free_list->next = NULL;

    return 0;
}

void *mem_alloc(int size, int style) {
    if (size <= 0) {
        mem_error = E_BAD_ARGS;
        fprintf(stderr, "Error: Invalid size, mem_error = %d\n", mem_error);
        return NULL;
    }

    // Ensure 8-byte alignment
    size = (size + 7) & ~7;

    // Implement mem_alloc() based on the allocation style
    if (free_list == NULL) {
        mem_error = E_NO_SPACE;
        fprintf(stderr, "Error: No free space available, mem_error = %d\n", mem_error);
        return NULL;
    }

    Block *prev = NULL;
    Block *curr = free_list;
    Block *best_fit = NULL;

    switch (style) {
        case M_BESTFIT:
          // implement the best fit
            while (curr != NULL) {
                if (curr->size >= size && (best_fit == NULL || curr->size < best_fit->size)) {
                    best_fit = curr;
                }
                curr = curr->next;
            }
            break;
        case M_WORSTFIT:
            // Implement worst-fit allocation
            while (curr != NULL) {
                if (curr->size >= size && (best_fit == NULL || curr->size > best_fit->size)) {
                    best_fit = curr;
                }
                curr = curr->next;
            }
            break;
        case M_FIRSTFIT:
            // Implement first-fit allocation
            while (curr != NULL) {
                if (curr->size >= size) {
                    best_fit = curr;
                    break;
                }
                curr = curr->next;
            }
            break;
        default:
            mem_error = E_BAD_ARGS;
            fprintf(stderr, "Error: Invalid allocation style, mem_error = %d\n", mem_error);
            return NULL;
    }

    if (best_fit == NULL) {
        mem_error = E_NO_SPACE;
        fprintf(stderr, "Error: No free space available, mem_error = %d\n", mem_error);
        return NULL;
    }

    // Split the block if it's larger than needed
    if (best_fit->size >= size + sizeof(Block)) {
        Block *new_block = (Block *)((char *)best_fit + size + sizeof(Block));
        new_block->size = best_fit->size - size - sizeof(Block);
        new_block->next = best_fit->next;
        best_fit->size = size;
        best_fit->next = new_block;
    }

    // Remove the allocated block from the free list
    if (prev == NULL) {
        free_list = best_fit->next;
    } else {
        prev->next = best_fit->next;
    }

    return (void *)(best_fit + 1);
}

int mem_free(void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Error: Trying to free a NULL pointer\n");
        return -1;
    }

    // Ensure 8-byte alignment
    ptr = (void *)(((size_t)ptr + 7) & ~7);

    // Find the block to free
    Block *block_to_free = (Block *)ptr - 1;

    // Add the block back to the free list
    block_to_free->next = free_list;
    free_list = block_to_free;

    // Implement coalescing
    Block *curr = free_list;

    while (curr != NULL && curr->next != NULL) {
        if ((char *)curr + curr->size + sizeof(Block) == (char *)curr->next) {
            // Coalesce adjacent blocks
            curr->size += curr->next->size + sizeof(Block);
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }

    return 0;
}

void mem_dump() {
    printf("Memory Dump:\n");
    Block *curr = free_list;
    while (curr != NULL) {
        printf("[Size: %zu bytes]\n", curr->size);
        curr = curr->next;
    }
}

int main() {
    // Test the memory allocator
    if (mem_init(4096) != 0) {
        fprintf(stderr, "Error: mem_init failed\n");
        return 1;
    }

    void *ptr1 = mem_alloc(16, M_BESTFIT);
    if (ptr1 == NULL) {
        fprintf(stderr, "Error: mem_alloc failed for ptr1\n");
        return 1;
    }

    void *ptr2 = mem_alloc(32, M_BESTFIT);
    if (ptr2 == NULL) {
        fprintf(stderr, "Error: mem_alloc failed for ptr2\n");
        return 1;
    }

    void *ptr3 = mem_alloc(64, M_BESTFIT);
    if (ptr3 == NULL) {
        fprintf(stderr, "Error: mem_alloc failed for ptr3\n");
        return 1;
    }

    void *ptr4 = mem_alloc(128, M_BESTFIT);
    if (ptr4 == NULL) {
        fprintf(stderr, "Error: mem_alloc failed for ptr4\n");
        return 1;
    }

    printf("Memory Dump After Allocations:\n");
    mem_dump();

    if (mem_free(ptr1) != 0) {
        fprintf(stderr, "Error: mem_free failed for ptr1\n");
        return 1;
    }

    if (mem_free(ptr3) != 0) {
        fprintf(stderr, "Error: mem_free failed for ptr3\n");
        return 1;
    }

    printf("Memory Dump After Freeing ptr1 and ptr3:\n");
    mem_dump();

    if (mem_free(ptr2) != 0) {
        fprintf(stderr, "Error: mem_free failed for ptr2\n");
        return 1;
    }

    if (mem_free(ptr4) != 0) {
        fprintf(stderr, "Error: mem_free failed for ptr4\n");
        return 1;
    }

    printf("Final Memory Dump:\n");
    mem_dump();

    return 0;
}