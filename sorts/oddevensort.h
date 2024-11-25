#pragma once
#include "sort_common.h"

static void odd_even_sort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    bool sorted = false;
    char* elemsc = (char*)elems;
    char* tmp = malloc(elem_size);
    int write_counter = 0;
    while (!sorted) {
        sorted = true;
        for (size_t i = 1; i < elem_len - 1; i += 2) {
            if (cmp_cb(userdata, &elemsc[i*elem_size], &elemsc[(i + 1)*elem_size]) == Ordering_GreaterThan) {
                memcpy(tmp, &elemsc[i*elem_size], elem_size);
                memcpy(&elemsc[i*elem_size], &elemsc[(i + 1)*elem_size], elem_size);
                memcpy(&elemsc[(i + 1)*elem_size], tmp, elem_size);
                write_counter += 2;
                if (write_counter > n_every) {
                    write_cb(userdata, elems, 0, elem_len);
                    write_counter -= n_every;
                }
                sorted = false;
            }
        }
        for (size_t i = 0; i < elem_len - 1; i += 2) {
            if (cmp_cb(userdata, &elemsc[i*elem_size], &elemsc[(i + 1)*elem_size]) == Ordering_GreaterThan) {
                memcpy(tmp, &elemsc[i*elem_size], elem_size);
                memcpy(&elemsc[i*elem_size], &elemsc[(i + 1)*elem_size], elem_size);
                memcpy(&elemsc[(i + 1)*elem_size], tmp, elem_size);
                write_counter += 2;
                if (write_counter > n_every) {
                    write_cb(userdata, elems, 0, elem_len);
                    write_counter -= n_every;
                }
                sorted = false;
            }
        }
    }
    write_cb(userdata, elems, 0, elem_len);
}
/*
typedef ptrdiff_t ssize_t;


static void oheap_partition_r(
    void* userdata,
    void* elems,
    ssize_t elem_size,
    ssize_t low,
    ssize_t high,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    int* write_counter,
    char* tmp
)
{
    char* elemsc = (char*)elems;
    while (low < high) {
        if (cmp_cb(userdata, &elemsc[low*elem_size], &elemsc[high*elem_size]) == Ordering_GreaterThan) {
            memcpy(tmp, &elemsc[low*elem_size], elem_size);
            memcpy(&elemsc[low*elem_size], &elemsc[high*elem_size], elem_size);
            memcpy(&elemsc[high*elem_size], tmp, elem_size);
            *write_counter += 2;
            if (*write_counter > n_every) {
                write_cb(userdata, elems, 0, high);
                *write_counter -= n_every;
            }
        }
        low += 1;
        high -= 1;
    }
}

static void oheap_quicksort_impl(
    void* userdata,
    void* elems,
    ssize_t elem_size,
    ssize_t elem_low,
    ssize_t elem_high,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    int* write_counter,
    char* tmp
) {
    if (elem_low < elem_high) {
        printf("%zu %zu\n", elem_low, elem_high);
        ssize_t middle = elem_low + (elem_high-elem_low)/2;
        if (elem_low < middle && middle < elem_high) {
            oheap_partition_r(userdata, elems, elem_size, elem_low, elem_high, n_every, write_cb, cmp_cb, write_counter, tmp);
            oheap_quicksort_impl(userdata, elems, elem_size, elem_low, middle/2, n_every, write_cb, cmp_cb, write_counter, tmp);
            oheap_quicksort_impl(userdata, elems, elem_size, middle/2, elem_high, n_every, write_cb, cmp_cb, write_counter, tmp);
        }
    }
}

static void oodd_even_shell_sort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    int write_counter = 0;
    char* key = malloc(elem_size);
    oheap_quicksort_impl(userdata, elems, elem_size, 0, (ssize_t)elem_len-1, n_every, write_cb, cmp_cb, &write_counter, key);
    optimized_insertionsort3((void*)&userdata, elems, elem_size, elem_len, n_every,  write_cb, cmp_cb);
}

static void odd_even_shell_sort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    ptrdiff_t gap = elem_len/2;
    printf("gapdada");
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int write_counter = 0;
    bool sorted = false;
    while(!(gap == 2 && sorted)) {
        sorted = true;
        for (size_t start = 0; start < elem_len-gap; start+=gap) {
            for (size_t j = start+1; j < start+gap; j++) {
                if (cmp_cb(userdata, &elemsc[start*elem_size], &elemsc[j*elem_size]) == Ordering_GreaterThan) {
                    memcpy(tmp, &elemsc[start*elem_size], elem_size);
                    memcpy(&elemsc[start*elem_size], &elemsc[j*elem_size], elem_size);
                    memcpy(&elemsc[j*elem_size], tmp, elem_size);
                    write_counter += 1;
                    if (write_counter > n_every) {
                        write_cb(userdata, elems, 0, elem_len);
                        write_counter -= n_every;
                    }
                    sorted = false;
                }
            }
        } 
        for (size_t start = 1; start < elem_len-gap; start+=gap) {
            for (size_t j = start+1; j < start+gap; j++) {
                if (cmp_cb(userdata, &elemsc[start*elem_size], &elemsc[j*elem_size]) == Ordering_GreaterThan) {
                    memcpy(tmp, &elemsc[start*elem_size], elem_size);
                    memcpy(&elemsc[start*elem_size], &elemsc[j*elem_size], elem_size);
                    memcpy(&elemsc[j*elem_size], tmp, elem_size);
                    write_counter += 1;
                    if (write_counter > n_every) {
                        write_cb(userdata, elems, 0, elem_len);
                        write_counter -= n_every;
                    }
                    sorted = false;
                }
            }
        }  
        if (sorted) {
            gap = gap/2;
            if (gap < 2) {
                gap = 2;
            }
        }
        printf("gap %zu %d\n", gap, sorted); 
    }
    write_cb(userdata, elems, 0, elem_len);
    getchar();
    free(tmp);
}*/
