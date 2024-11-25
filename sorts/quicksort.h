#pragma once
#include "sort_common.h"

static void quicksort_swap(char* tmp, void* elems, ssize_t i, ssize_t j, size_t elem_size) {
    char* elemsc = (char*)elems;
    memcpy(tmp, &elemsc[i*elem_size], elem_size);
    memcpy(&elemsc[i*elem_size], &elemsc[j*elem_size], elem_size);
    memcpy(&elemsc[j*elem_size], tmp, elem_size);
}

static ssize_t partition(
    void* userdata,
    void* elems_base,
    size_t elem_len,
    void* elems_void,
    ssize_t elem_size,
    ssize_t low,
    ssize_t high,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    int* write_counter,
    char* tmp
) {
    char* elems = (char*)elems_void;
    char* pivot = elems+high*elem_size;
    ssize_t i = low-1;
    for(ssize_t j = low; j < high; j++) {
        char* a = elems+j*elem_size;
        if (cmp_cb(userdata, a, pivot) <= 0) {
            i += 1;
            quicksort_swap(tmp, elems, i, j, elem_size);
            *write_counter += 2;
            if (*write_counter > n_every) {
                write_cb(userdata, elems_base, 0, elem_len);
                *write_counter -= n_every;
            }
        }
    }
    quicksort_swap(tmp, elems, i+1, high, elem_size);
    *write_counter += 2;
    if (*write_counter > n_every) {
        write_cb(userdata, elems_base, 0, elem_len);
        *write_counter -= n_every;
    }
    return i+1;
}

static int partition_r(
    void* userdata,
    void* elems_base,
    size_t elem_len,
    void* elems_void,
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
    char* elems = (char*)elems_void;
    int random = 0;
    if (high - low > 0) {
        random = low + rand() % (high - low);
    }
    quicksort_swap(tmp, elems, random, high, elem_size);
    *write_counter += 2;
    return partition(userdata, elems_base, elem_len, elems_void, elem_size, low, high, n_every, write_cb, cmp_cb, write_counter, tmp);
}

static void quicksort_impl(
    void* userdata,
    void* elems_base,
    size_t elem_len,
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
        ssize_t index = partition_r(userdata, elems_base, elem_len, elems, elem_size, elem_low, elem_high, n_every, write_cb, cmp_cb, write_counter, tmp);
        quicksort_impl(userdata, elems_base, elem_len, elems, elem_size, elem_low, index-1, n_every, write_cb, cmp_cb, write_counter, tmp);
        quicksort_impl(userdata, elems_base, elem_len, elems, elem_size, index+1, elem_high, n_every, write_cb, cmp_cb, write_counter, tmp);
    }
}

static void quicksort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    srand(time(NULL));
    int write_counter = 0;
    char* key = malloc(elem_size);
    quicksort_impl(userdata, elems, elem_len, elems, elem_size, 0, (ssize_t)elem_len-1, n_every, write_cb, cmp_cb, &write_counter, key);
}

static void heap_quicksort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    srand(time(NULL));
    int write_counter = 0;
    char* key = malloc(elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify_min(userdata, elems, elem_size, elem_len, i, n_every, write_cb, cmp_cb, key, &write_counter);
    }
    quicksort_impl(userdata, elems, elem_len, elems, elem_size, 0, (ssize_t)elem_len-1, n_every, write_cb, cmp_cb, &write_counter, key);
}

#ifdef TEST_QUICKSORT

Ordering cmp_char(void* userdata, void* a_void, void* b_void) {
    (void)userdata;
    char a = *(char*)a_void;
    char b = *(char*)b_void;
    if (a == b) return Ordering_Equal;
    else if (a < b) return Ordering_LessThan;
    else return Ordering_GreaterThan;
}

Ordering cmp_int(void* userdata, void* a_void, void* b_void) {
    (void)userdata;
    int a = *(int*)a_void;
    int b = *(int*)b_void;
    if (a == b) return Ordering_Equal;
    else if (a < b) return Ordering_LessThan;
    else return Ordering_GreaterThan;
}

int main(void) {
    char out[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    shuffle(out, 1, 20);
    printf("Shuffle\n");
    for (size_t i = 0; i < 20; i++) {
        printf("%d\n", (int)out[i]);
    }
    quicksort(0, 0, out, 1, 20, 0, cmp_char);
    printf("Sorted\n");
    for (size_t i = 0; i < 20; i++) {
        printf("%d\n", (int)out[i]);
    }

    int numbers[20] = {2, 5060, 10, 200, 1000, 3000, 54, 1400, 1, 6, 5060, 922222, 20, 90, 123};
    quicksort(0, 0, numbers, 4, 20, 0, cmp_int);
    printf("Sorted int\n");
    for (size_t i = 0; i < 20; i++) {
        printf("%d\n", (int)numbers[i]);
    }
    return 0;
}
#endif
