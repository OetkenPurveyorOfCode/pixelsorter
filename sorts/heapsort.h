#pragma once
#include "sort_common.h"

void heapify(
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
	size_t maximum = i; 
	size_t right_child_index = 2 * i + 2; 
	size_t left_child_index = 2 * i + 1; 
    
	if (left_child_index < elem_len && cmp_cb(userdata, &elemsc[left_child_index*elem_size], &elemsc[maximum*elem_size]) == Ordering_GreaterThan) {
		maximum = left_child_index; 
    }
	if (right_child_index < elem_len && cmp_cb(userdata, &elemsc[right_child_index*elem_size], &elemsc[maximum*elem_size])== Ordering_GreaterThan) {
		maximum = right_child_index; 
    }
    //printf("heapify %zu %zu\n", maximum, i);
	// checking if we needed swaping the elements or not 
	if (maximum != i) { 
		memcpy(tmp, &elemsc[i*elem_size], elem_size); 
		memcpy(&elemsc[i*elem_size], &elemsc[maximum*elem_size], elem_size);
		memcpy(&elemsc[maximum*elem_size], tmp, elem_size);
        *write_counter += 2;
        if (*write_counter >= n_every) {
            write_cb(userdata, elems, 0, elem_len);
            *write_counter -= n_every;
        }
		heapify(userdata, elems, elem_size, elem_len, maximum, n_every, write_cb, cmp_cb, tmp, write_counter); 
	} 
}

static void heapsort(
    void* userdata,
    void* elems,
    size_t elem_size,
    size_t elem_len,
    int n_every,
    void (*write_cb)(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end),
    Ordering (*cmp_cb)(void* userdata_void, void* elem1, void* elem2)
) {
    char* elemsc = (char*)elems;
    int write_counter = 0;
    char* tmp = malloc(elem_size);
    for (size_t i = elem_len / 2 - 1; i < elem_len/2; i--) { 
        heapify(userdata, elems, elem_size, elem_len, i, n_every, write_cb, cmp_cb, tmp, &write_counter);
    }
    #ifdef TEST_HEAPSORT
    int* elemsi = (int*) elems;
    printf("Heap Array : "); 
	for (int i = 0; i < elem_len; i++) { 
		printf("%d ", elemsi[i]); 
	} 
	#endif
    for (size_t i = elem_len - 1; i > 0; i--) {
        memcpy(tmp, &elemsc[0*elem_size], elem_size);
        memcpy(&elemsc[0*elem_size], &elemsc[i*elem_size], elem_size);
        memcpy(&elemsc[i*elem_size], tmp, elem_size);
        write_counter += 2;
        if (write_counter >= n_every) {
            write_cb(userdata, elems, 0, elem_len);
            write_counter -= n_every;
        }
        heapify(userdata, elems, elem_size, i, 0, n_every, write_cb, cmp_cb, tmp, &write_counter);
    }

}



#ifdef TEST_HEAPSORT
Ordering cmp_int(void* ud, void* a, void* b) {
    int ia = *((int*)a);
    int ib = *((int*)b);
    if (ia < ib) {return Ordering_LessThan;}
    else if (ia == ib) {return Ordering_Equal;}
    else { return Ordering_GreaterThan;}
}

void write_cb(void* userdata_void, void* elems, size_t elem_begin, size_t elem_end) {
    return;
}
int main() 
{ 
	// initializing the array 
	int arr[] = {1, 11, 20, 18, 5, 15, 3, 2 }; 

	// Displaying original array 
	printf("Original Array : "); 
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) { 
		printf("%d ", arr[i]); 
	} 

	printf("\n"); 
	heapsort(0, arr, sizeof(arr[0]), sizeof(arr)/sizeof(arr[0]), 1000, write_cb, cmp_int);

	// Displaying sorted array 
	printf("Array after performing heap sort: "); 
	for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) { 
		printf("%d ", arr[i]); 
	} 
	return 0; 
}
#endif

