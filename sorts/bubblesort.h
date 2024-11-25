#pragma once
#include "sort_common.h"


static void bubblesort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    char* key = (char*)malloc(elem_size);
    int write_counter = 0;
    for (size_t i = 0; i < elem_len - 1; i++) {
        for (size_t j = 0; j < elem_len - i - 1; j++) {
            if (cmp_cb(userdata, &elemsc[j*elem_size], &elemsc[(j+1)*elem_size]) == Ordering_GreaterThan) {
                memcpy(key, &elemsc[j*elem_size], elem_size);
                memcpy(&elemsc[j*elem_size], &elemsc[(j+1)*elem_size], elem_size);
                memcpy(&elemsc[(j+1)*elem_size], key, elem_size);
                write_counter += 2;
                if (write_counter >= n_every) {
                    write_cb(userdata, elems, 0, elem_len);
                    write_counter -= n_every;
                }
            }
            
        }
    }
    write_cb(userdata, elems, 0, elem_len);
}

static void heap_bubblesort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    char* key = (char*)malloc(elem_size);
    int write_counter = 0;
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify_min(userdata, elems, elem_size, elem_len, i, n_every, write_cb, cmp_cb, key, &write_counter);
    }
    for (size_t i = 0; i < elem_len - 1; i++) {
        for (size_t j = 0; j < elem_len - i - 1; j++) {
            if (cmp_cb(userdata, &elemsc[j*elem_size], &elemsc[(j+1)*elem_size]) == Ordering_GreaterThan) {
                memcpy(key, &elemsc[j*elem_size], elem_size);
                memcpy(&elemsc[j*elem_size], &elemsc[(j+1)*elem_size], elem_size);
                memcpy(&elemsc[(j+1)*elem_size], key, elem_size);
                write_counter += 2;
                if (write_counter >= n_every) {
                    write_cb(userdata, elems, 0, elem_len);
                    write_counter -= n_every;
                }
            }
            
        }
    }
    write_cb(userdata, elems, 0, elem_len);
}
