#pragma once
#include "sort_common.h"

static bool circlesort_impl(
    char* tmp,
    void* userdata,
    void* elems,
    ssize_t elem_size,
    ssize_t elem_len,
    ssize_t low,
    ssize_t high,
    int n_every,
    int* write_counter,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    bool swapped = false;
    char* elemsc = (char*)elems;
    if (low == high) {
        return false;
    }
    ssize_t lo = low;
    ssize_t hi = high;
    while (lo < hi) {
        if (cmp_cb(userdata, &elemsc[lo*elem_size], &elemsc[hi*elem_size]) == Ordering_GreaterThan) {
            mem_swap(tmp, elemsc, elem_size, lo, hi);
            *write_counter += 2;
            if (*write_counter > n_every) {
                write_cb(userdata, elems, 0, elem_len);
                *write_counter -= n_every;
            }
            swapped = true;
        }
        lo += 1;
        hi -= 1;
    }

    if (lo == hi) {
        if (cmp_cb(userdata, &elemsc[lo*elem_size], &elemsc[(hi+1)*elem_size]) == Ordering_GreaterThan) {
            mem_swap(tmp, elemsc, elem_size, lo, hi+1);
            *write_counter += 2;
            if (*write_counter > n_every) {
                write_cb(userdata, elems, 0, elem_len);
                *write_counter -= n_every;
            }
            swapped = true;
        }
    }
    ssize_t mid = low + (high - low) / 2;
    bool swapped_first_half = circlesort_impl(tmp, userdata, elems, elem_size, elem_len, low, mid, n_every, write_counter, write_cb, cmp_cb);
    bool swapped_second_half = circlesort_impl(tmp, userdata, elems, elem_size, elem_len, mid+1, high, n_every, write_counter, write_cb, cmp_cb);
    return swapped || swapped_first_half || swapped_second_half;
}

static void circlesort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* tmp = malloc(elem_size);
    int write_counter = 0;
    while (circlesort_impl(tmp, userdata, elems, elem_size, elem_len, 0, elem_len-1, n_every, &write_counter, write_cb, cmp_cb)) {};
    free(tmp);
    return;
}
