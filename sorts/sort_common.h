#pragma once
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef ORDERING_DEFINED
#define ORDERING_DEFINED
typedef enum {
    Ordering_LessThan = -1,
    Ordering_Equal = 0,
    Ordering_GreaterThan = 1,
} Ordering;
#endif

typedef ptrdiff_t ssize_t;

static inline void mem_swap(char* tmp, char* elemsc, size_t elem_size, size_t i, size_t j) {
    memcpy(tmp, &elemsc[i*elem_size], elem_size);
    memcpy(&elemsc[i*elem_size], &elemsc[j*elem_size], elem_size);
    memcpy(&elemsc[j*elem_size], tmp, elem_size);
    
}

void heapify_min(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len, 
    size_t i,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    char* tmp,
    int* write_counter
    ) 
{
    char* elemsc = (char*)elems;
	size_t minimum = i; 
	size_t right_child_index = 2 * i + 2; 
	size_t left_child_index = 2 * i + 1; 
    
	if (left_child_index < elem_len && cmp_cb(userdata, &elemsc[left_child_index*elem_size], &elemsc[minimum*elem_size]) == Ordering_LessThan) {
		minimum = left_child_index; 
    }
	if (right_child_index < elem_len && cmp_cb(userdata, &elemsc[right_child_index*elem_size], &elemsc[minimum*elem_size])== Ordering_LessThan) {
		minimum = right_child_index; 
    }
	if (minimum != i) { 
		memcpy(tmp, &elemsc[i*elem_size], elem_size); 
		memcpy(&elemsc[i*elem_size], &elemsc[minimum*elem_size], elem_size);
		memcpy(&elemsc[minimum*elem_size], tmp, elem_size);
        *write_counter += 2;
        if (*write_counter >= n_every) {
            write_cb(userdata, elems, 0, elem_len);
            *write_counter -= n_every;
        }
		heapify_min(userdata, elems, elem_size, elem_len, minimum, n_every, write_cb, cmp_cb, tmp, write_counter); 
	} 
}
