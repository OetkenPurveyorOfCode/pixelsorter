#pragma once
#include "sort_common.h"

static void shellsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    ptrdiff_t gap = elem_len/2;
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int write_counter = 0;
    while(1) {
        gap = gap/2;
        if (gap < 1) {
            gap = 1;
        }
        for (ptrdiff_t i = gap; i < (ptrdiff_t)elem_len; i++) {
            memcpy(tmp, &elemsc[i*elem_size], elem_size);
            write_counter += 1;
            ptrdiff_t j = 0;
            for (j = i; (j >= gap) && cmp_cb(userdata, &elemsc[(j - gap)*elem_size], tmp) == Ordering_GreaterThan; j -= gap)
            {
                memcpy(&elemsc[j*elem_size], &elemsc[(j-gap)*elem_size], elem_size);
                write_counter += 1;
                if (write_counter > n_every) {
                    write_cb(userdata, elems, 0, elem_len);
                    write_counter -= n_every;
                }
            }
            memcpy(&elemsc[j*elem_size], tmp, elem_size);
            write_counter += 1;
        }
        if (gap == 1) {
            break;
        }
    }
    write_cb(userdata, elems, 0, elem_len);
    free(tmp);
}




