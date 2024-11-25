#pragma once
#include "sort_common.h"

#define countof(x) (sizeof(x)/sizeof(*x))

static void selectionsort_impl(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2),
    size_t to_sort_begin,
    size_t to_sort_end
) {
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int write_counter = 0;
    for (size_t n = to_sort_begin; n < to_sort_end; n++) {
        size_t minimum_index = n;
        for (size_t i = to_sort_begin+1; i < elem_len; i++) {
            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimum_index*elem_size) == Ordering_LessThan) {
                minimum_index = i;
            }
        }
        memcpy(tmp, elemsc+to_sort_begin*elem_size, elem_size);
        memcpy(elemsc+to_sort_begin*elem_size, elemsc+minimum_index*elem_size, elem_size);
        memcpy(elemsc+minimum_index*elem_size, tmp, elem_size);
        to_sort_begin += 1;
        write_counter += 2;
        if (write_counter > n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
        
    }
    write_cb(userdata, elems, 0, elem_len);
    free(tmp);
}

static void selectionsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    return selectionsort_impl(userdata, elems, elem_size, elem_len, n_every, write_cb, cmp_cb, 0, elem_len-1);
}

static void selectionsort_swap(char* elemsc, size_t i1, size_t i2, size_t elem_size, char* tmp) {
    if (i1 != i2) {
    memcpy(tmp, elemsc+i1*elem_size, elem_size);
    memcpy(elemsc+i1*elem_size, elemsc+i2*elem_size, elem_size);
    memcpy(elemsc+i2*elem_size, tmp, elem_size);
    }
}

static void double_selectionsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int write_counter = 0;
    size_t to_sort_begin = 0;
    size_t to_sort_end = elem_len;
    while (to_sort_begin < to_sort_end) {
        size_t minimum_index = to_sort_begin;
        size_t maximum_index = to_sort_begin;
        for (size_t i = to_sort_begin+1; i < to_sort_end; i++) {
            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimum_index*elem_size) == Ordering_LessThan) {
                minimum_index = i;
            }
            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+maximum_index*elem_size) == Ordering_GreaterThan) {
                maximum_index = i;
            }
        }
        //printf("%zu %zu, min %zu %zu max %zu %zu \n", to_sort_begin, to_sort_end, minimum_index, ((SPixel*)(&elemsc[minimum_index*elem_size]))->index, maximum_index, ((SPixel*)(&elemsc[maximum_index*elem_size]))->index);
        
        memcpy(tmp, elemsc+to_sort_begin*elem_size, elem_size);
        memcpy(elemsc+to_sort_begin*elem_size, elemsc+minimum_index*elem_size, elem_size);
        memcpy(elemsc+minimum_index*elem_size, tmp, elem_size);
        to_sort_begin += 1;
        if (maximum_index != to_sort_begin-1) {
            to_sort_end -= 1; // it pointed past the end, now in range
            memcpy(tmp, elemsc+to_sort_end*elem_size, elem_size);
            memcpy(elemsc+to_sort_end*elem_size, elemsc+maximum_index*elem_size, elem_size);
            memcpy(elemsc+maximum_index*elem_size, tmp, elem_size);
        }

        write_counter += 10;
        if (write_counter > n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
        
    }
    write_cb(userdata, elems, 0, elem_len);
    free(tmp);
}

static void moving_selectionsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int write_counter = 0;
    size_t to_sort_begin = 0;
    size_t to_sort_end = elem_len;
    while (to_sort_begin < to_sort_end) {
        void* minimum = &elemsc[to_sort_begin*elem_size];
        for (size_t i = to_sort_begin+1; i < to_sort_end; i++) {
            if (cmp_cb(userdata, elemsc+i*elem_size, minimum) == Ordering_LessThan) {
                memcpy(tmp, elemsc+i*elem_size, elem_size);
                memcpy(elemsc+i*elem_size, minimum, elem_size);
                memcpy(minimum, tmp, elem_size);
            }
        }
        //printf("%zu %zu, min %zu %zu max %zu %zu \n", to_sort_begin, to_sort_end, minimum_index, ((SPixel*)(&elemsc[minimum_index*elem_size]))->index, maximum_index, ((SPixel*)(&elemsc[maximum_index*elem_size]))->index);
        to_sort_begin += 1;

        write_counter += 10;
        if (write_counter > n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
        
    }
    write_cb(userdata, elems, 0, elem_len);
    free(tmp);
}




static void optimized_selectionsort55(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    char* tmp = (char*)malloc(elem_size);
    int counters[8] = {0};
    size_t to_sort_begin = 0;
    size_t to_sort_end = elem_len;
    int write_counter = 0;
    size_t maximum_index_ = 0;
    for (size_t i = 0; i < elem_len; i++) {
        if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+maximum_index_*elem_size) == Ordering_GreaterThan) {
            maximum_index_ = i;
        }
    }
    selectionsort_swap(elemsc, to_sort_end-1, maximum_index_, elem_size, tmp);
    maximum_index_ = to_sort_end-1;
    while (to_sort_begin < to_sort_end-8) {
        size_t minimums[8] = {maximum_index_, maximum_index_, maximum_index_, maximum_index_, maximum_index_, maximum_index_, maximum_index_, maximum_index_};
        for (size_t i = to_sort_begin; i < to_sort_end; i++) {
            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[7]*elem_size) == Ordering_LessThan) {
                // i less than minimum 7
                if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[3]*elem_size) == Ordering_LessThan) {
                    // i less than minimum 3
                    if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[2]*elem_size) == Ordering_LessThan) {
                        // i less than minimum 2
                        if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[1]*elem_size) == Ordering_LessThan) { 
                            // i less than minimum 1
                            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[0]*elem_size) == Ordering_LessThan) { 
                                // i less than minimum 0
                                minimums[7] = minimums[6];
                                minimums[6] = minimums[5];
                                minimums[5] = minimums[4];
                                minimums[4] = minimums[3];
                                minimums[3] = minimums[2];
                                minimums[2] = minimums[1];
                                minimums[1] = minimums[0];
                                minimums[0] = i;
                            }
                            else {
                                // i less than minimum 1
                                minimums[7] = minimums[6];
                                minimums[6] = minimums[5];
                                minimums[5] = minimums[4];
                                minimums[4] = minimums[3];
                                minimums[3] = minimums[2];
                                minimums[2] = minimums[1];
                                minimums[1] = i;
                            }
                        }
                        else {
                            // i less than minimum 2
                            minimums[7] = minimums[6];
                            minimums[6] = minimums[5];
                            minimums[5] = minimums[4];
                            minimums[4] = minimums[3];
                            minimums[3] = minimums[2];
                            minimums[2] = i;
                        }
                    }
                    else {
                        // i less than minimum 3
                        minimums[7] = minimums[6];
                        minimums[6] = minimums[5];
                        minimums[5] = minimums[4];
                        minimums[4] = minimums[3];
                        minimums[3] = i;
                    }
                }
                else {
                    // i less than minimum 7
                    if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[6]*elem_size) == Ordering_LessThan) {
                        // i less than minimum 6
                        if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[5]*elem_size) == Ordering_LessThan) {
                            // i less than minimum 5
                            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimums[4]*elem_size) == Ordering_LessThan) { 
                                // i less than minimum 4
                                minimums[7] = minimums[6];
                                minimums[6] = minimums[5];
                                minimums[5] = minimums[4];
                                minimums[4] = i;
                            }
                            else {
                                // i less than minimum 5
                                minimums[7] = minimums[6];
                                minimums[6] = minimums[5];
                                minimums[5] = i;
                            }
                        }
                        else {
                            // i less than minimum 6
                            minimums[7] = minimums[6];
                            minimums[6] = i;
                        }
                    }
                    else {
                        // i less than minimum 7
                        minimums[7] = i;
                    }
                }
            }

            
        }
        selectionsort_swap(elemsc, to_sort_begin, minimums[0], elem_size, tmp);
        to_sort_begin += 1;
        counters[0] += 1;
        if (
            minimums[1] > to_sort_begin+7
            && minimums[2] > to_sort_begin+7
            && minimums[3] > to_sort_begin+7
            && minimums[4] > to_sort_begin+7
            && minimums[5] > to_sort_begin+7
            && minimums[6] >  to_sort_begin+7
            && minimums[7] > to_sort_begin+7
            
        ) {
            selectionsort_swap(elemsc, to_sort_begin, minimums[1], elem_size, tmp);
            to_sort_begin += 1;
            counters[1] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[2], elem_size, tmp);
            to_sort_begin += 1;
            counters[2] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[3], elem_size, tmp);
            to_sort_begin += 1;
            counters[3] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[4], elem_size, tmp);
            to_sort_begin += 1;
            counters[4] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[5], elem_size, tmp);
            to_sort_begin += 1;
            counters[5] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[6], elem_size, tmp);
            to_sort_begin += 1;
            counters[6] += 1;
            selectionsort_swap(elemsc, to_sort_begin, minimums[7], elem_size, tmp);
            to_sort_begin += 1;
            counters[7] += 1;
        }
        write_counter += 2*8;
        if (write_counter > n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
            for (size_t i = 0; i < countof(counters); i++) {
                printf("counter %zu %d\n", i, counters[i]);
            }
        }
    }

    to_sort_begin -= 10;
    while (to_sort_begin < to_sort_end) {
        //printf("to sort begin %d\n", to_sort_begin);
        size_t minimum_index = to_sort_begin;
        for (size_t i = to_sort_begin+1; i < elem_len; i++) {
            if (cmp_cb(userdata, elemsc+i*elem_size, elemsc+minimum_index*elem_size) == Ordering_LessThan) {
                minimum_index = i;
            }
        }
        memcpy(tmp, elemsc+to_sort_begin*elem_size, elem_size);
        memcpy(elemsc+to_sort_begin*elem_size, elemsc+minimum_index*elem_size, elem_size);
        memcpy(elemsc+minimum_index*elem_size, tmp, elem_size);
        to_sort_begin += 1;
        write_counter += 2;
        if (write_counter > n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
    }
    
    write_cb(userdata, elems, 0, elem_len);
    free(tmp);
    return;
}

#ifdef TEST_SELECTIONSORT
static bool is_minimums_sorted(Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2), 
void* userdata, char* elemsc, size_t elem_len, size_t elem_size, size_t* minimums, size_t minimum_len) {
    for (size_t i = 0; i < minimum_len -1; i++) {
        if (cmp_cb(userdata, elemsc+minimums[i]*elem_size, elemsc+minimums[i+1]*elem_size) == Ordering_GreaterThan) {
            return false;
        }
    }
    return true;
}
#endif



