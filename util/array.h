#define ARRAY_INIT_CAPACITY 8
#define append(array, elem) \
    do { \
        assert((array)->len >= 0); \
        assert((array)->capacity >= 0); \
        assert((array)->len <= (array)->capacity); \
        assert(sizeof(*((array)->data)) == sizeof(elem)); \
        if (((array))->len >= (array)->capacity) { \
            (array)->capacity = (array)->capacity == 0 ? ARRAY_INIT_CAPACITY : (array)->capacity*2; \
            (array)->data = realloc((array)->data, sizeof(elem)*(array)->capacity); \
            assert((array)->data); \
        } \
        ((array))->data[((array))->len] = (elem); \
        ((array))->len += 1; \
    } while (0);
