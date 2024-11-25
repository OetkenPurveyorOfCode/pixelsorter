#pragma once
#include <stdint.h>

void shuffle(void* elems_void, size_t elem_size, size_t elem_count) {
    char* elems = (char*)elems_void;
    void* tmp = malloc(elem_size);
    for (size_t i = elem_count-1; i >= 1; i--) {
        uint32_t j = pcg32_boundedrand(i+1);
        memcpy(tmp,                 elems+i*elem_size,  elem_size);
        memcpy(elems+i*elem_size,   elems+j*elem_size,  elem_size);
        memcpy(elems+j*elem_size,   tmp,                elem_size);
    }
    free(tmp);
}

#if 0
#define shuffle(Ts, len) \
    for (size_t i = (len)-1; i >= 1; i--) { \
        uint32_t j = pcg32_boundedrand(i+1); \
        typeof(*(Ts)) T = Ts[i]; \
        Ts[i] = Ts[j]; \
        Ts[j] = T; \
    }
#endif
