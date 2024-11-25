#pragma once
#include "sort_common.h"


static void insertionsort(
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
    for (ptrdiff_t it = 1; it < (ptrdiff_t)elem_len; it++) {
        memcpy(key, elemsc+it*elem_size, elem_size);
        ptrdiff_t j = it - 1;
        while (j >= 0 && (cmp_cb(userdata, (void*)&elemsc[j*elem_size], key) == Ordering_GreaterThan)) {
            memcpy(elemsc+(j+1)*elem_size, elemsc+j*elem_size, elem_size);
            write_counter += 1;
            if (write_counter == n_every) {
                write_cb(userdata, elems, 0, elem_len);
                write_counter = 0;
            }
            j -= 1;
        }
        memcpy(elemsc+(j+1)*elem_size, key, elem_size);
        write_counter += 1;
        if (write_counter == n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter = 0;
        }
    }
    write_cb(userdata, elems, 0, elem_len);
    free(key);
}



static void middle_out_sort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    if (elem_len <= 1) return;
    char* elemsc = (char*)elems;
    char* key = (char*)malloc(elem_size);
    int write_counter = 0;
    size_t first = elem_len / 2 - 1;
    size_t right = first + 1 + (elem_len & 1);
    for (; right < elem_len; ++right, --first) {
        
        if(cmp_cb(userdata, &elemsc[first*elem_size], &elemsc[right*elem_size]) == Ordering_GreaterThan) {
            memcpy(key, &elemsc[first*elem_size], elem_size);
            memcpy(&elemsc[first*elem_size], &elemsc[right*elem_size], elem_size);
            memcpy(&elemsc[right*elem_size], key, elem_size);
            write_counter += 2;
        }
        // linear insert right element from right to left
        memcpy(key, elemsc+right*elem_size, elem_size);
        size_t j = right - 1;
        while (cmp_cb(userdata, &elemsc[j*elem_size], key) == Ordering_GreaterThan) {
            memcpy(elemsc+(j+1)*elem_size, elemsc+j*elem_size, elem_size);
            write_counter += 1;
            if (write_counter > n_every) {
                write_cb(userdata, elems, 0, elem_len);
                write_counter -= n_every;
            }
            j -= 1;
        }
        memcpy(elemsc+(j+1)*elem_size, key, elem_size);
        write_counter += 1;
        if (write_counter == n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter = 0;
        }
        // linear insert first element from left to right
        memcpy(key, elemsc+first*elem_size, elem_size);
        j = first + 1;
        while (cmp_cb(userdata, &elemsc[j*elem_size], key) == Ordering_LessThan) {
            memcpy(elemsc+(j-1)*elem_size, elemsc+j*elem_size, elem_size);
            write_counter += 1;
            if (write_counter > n_every) {
                write_cb(userdata, elems, 0, elem_len);
                write_counter -= n_every;
            }
            j += 1;
        }
        memcpy(elemsc+(j-1)*elem_size, key, elem_size);
        write_counter += 1;
        if (write_counter == n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter = 0;
        }
    }
    write_cb(userdata, elems, 0, elem_len);
}

static void optimized_insertionsort3(
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
    for (ptrdiff_t it = 1; it < (ptrdiff_t)elem_len; it++) {
        memcpy(key, elemsc+it*elem_size, elem_size);
        ptrdiff_t j = it-1;
        if (j-it/2 >= 0 && (cmp_cb(userdata, (void*)&elemsc[(j-it/2)*elem_size], key) == Ordering_GreaterThan)) {
            j-=it/2;
        }
        if (j-it/4 >= 0 && (cmp_cb(userdata, (void*)&elemsc[(j-it/4)*elem_size], key) == Ordering_GreaterThan)) {
            j-=it/4;
        }
        if (j-it/8 >= 0 && (cmp_cb(userdata, (void*)&elemsc[(j-it/8)*elem_size], key) == Ordering_GreaterThan)) {
            j-=it/8;
        }
        if (j-it/16 >= 0 && (cmp_cb(userdata, (void*)&elemsc[(j-it/16)*elem_size], key) == Ordering_GreaterThan)) {
            j-=it/16;
        }
        bool loop_entered = false;
        while (j >= 0 && (cmp_cb(userdata, (void*)&elemsc[j*elem_size], key) == Ordering_GreaterThan)) {
            j -= 1;
            loop_entered = true;
        }
        if (loop_entered) {
            j += 1;
        }
        memmove(elemsc+(j+1)*elem_size, elemsc+j*elem_size, elem_size*(it-j));
        if (loop_entered) {
            j -= 1;
        }
        memcpy(elemsc+(j+1)*elem_size, key, elem_size);

        write_counter += (it-j)+1;
        if (write_counter >= n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
    }
    write_cb(userdata, elems, 0, elem_len);
    free(key);
}




static void heap_insertionsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* key = (char*)malloc(elem_size);
    int write_counter = 0;
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify_min(userdata, elems, elem_size, elem_len, i, n_every, write_cb, cmp_cb, key, &write_counter);
    }
    optimized_insertionsort3(userdata, elems, elem_size, elem_len, n_every, write_cb, cmp_cb);
    write_cb(userdata, elems, 0, elem_len);
    free(key);
}


