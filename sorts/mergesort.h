#pragma once
#include "sort_common.h"


static void merge(
    void* user_data,
    Ordering (*cmp_cb)(void* userdata, void* a, void* b),
    size_t elem_size,
    void* as_void,
    size_t len_as,
    void* bs_void,
    size_t len_bs,
    void* out_void
) {
    size_t ai = 0;
    size_t bi = 0;
    size_t oi = 0;
    char* as = (char*)as_void;
    char* bs = (char*)bs_void;
    char* out = (char*)out_void;
    while (ai < len_as && bi < len_bs) {
        if ((int)cmp_cb(user_data, (void*)(as+ai*elem_size), (void*)(bs+bi*elem_size)) <= 0) {
            memcpy(out+oi*elem_size, as+ai*elem_size, elem_size);
            ai += 1;
            oi += 1;
        } else {
            memcpy(out+oi*elem_size, bs+bi*elem_size, elem_size);
            bi += 1;
            oi += 1;
        }
    }
    assert(bi == len_bs || ai == len_as);
    while (ai < len_as) {
        memcpy(out+oi*elem_size, as+ai*elem_size, elem_size);
        ai += 1;
        oi += 1;
    }
    while (bi < len_bs) {
        memcpy(out+oi*elem_size, bs+bi*elem_size, elem_size);
        bi += 1;
        oi += 1;
    }
    assert(ai == len_as && bi == len_bs);
}

static void mergesort_impl(
    void* userdata,
    char* elems,
    size_t elem_size,
    size_t elem_len,
    char* tmp,
    int n_every,
    int* write_counter,
    void* write_elems, 
    size_t write_len,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    if (elem_len <= 1) {
        return;
    }
    else {
        // TODO: use these pointers instead of lengths
        char* start = elems;
        char* middle = elems+(elem_len/2)*elem_size;
        mergesort_impl(
            userdata,
            start, elem_size, elem_len/2,
            tmp, n_every, write_counter, write_elems, write_len,
            write_cb, cmp_cb
        );
        mergesort_impl(
            userdata,
            middle, elem_size, elem_len-elem_len/2,
            tmp, n_every, write_counter, write_elems, write_len,
            write_cb, cmp_cb
        );
        merge(userdata, cmp_cb, elem_size, start, elem_len/2, middle, elem_len-elem_len/2, tmp);
        for (size_t it = 0; it < elem_len; it++) {
            memcpy(elems+it*elem_size, tmp+it*elem_size, elem_size);
            *write_counter += 1;
            if (*write_counter > n_every) {
                write_cb(userdata, write_elems, 0, write_len);
                *write_counter -= n_every;
            }
        }
    }
}

static void mergesort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* tmp = (char*)malloc(sizeof(char)*elem_size*elem_len);
    int write_counter = 0;
    mergesort_impl(userdata, (char*)elems, elem_size, elem_len, tmp, n_every, &write_counter, elems, elem_len, write_cb, cmp_cb);
    free(tmp);
}

#ifdef TEST_MERGESORT

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
    char left[] = {2, 4, 6, 8, 10};
    char right[] = {1, 5, 9};
    char out[20] = {0};
    merge(0, cmp_char, 1, (void*)left, 5, (void*)right, 3, out);
    for (size_t i = 0; i < 10; i++) {
        printf("%d\n", (int)out[i]);
    }
    shuffle(out, 10);
    printf("Shuffle\n");
    for (size_t i = 0; i < 10; i++) {
        printf("%d\n", (int)out[i]);
    }
    mergesort(0, 0, out, 1, 10, 0, cmp_char);
    printf("Sorted\n");
    for (size_t i = 0; i < 10; i++) {
        printf("%d\n", (int)out[i]);
    }

    int numbers[20] = {2, 5060, 10, 200, 1000, 3000, 54, 1400, 1, 6, 5060, 922222, 20, 90, 123};
    mergesort(0, 0, numbers, 4, 20, 0, cmp_int);
    printf("Sorted int\n");
    for (size_t i = 0; i < 20; i++) {
        printf("%d\n", (int)numbers[i]);
    }
    return 0;
}
#endif

