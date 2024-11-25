#include <stdint.h>

typedef struct {
    const char* data;
    size_t len;
} sv;

typedef enum {
    PARSE_INT_OK = 0,
    PARSE_INT_ERR_INVALID_DIGIT,
    PARSE_INT_ERR_OVERFLOW,
    PARSE_INT_ERR_EMPTY_STRING,
} ParseIntError;

#if TEST || !__has_builtin(__builtin_mul_overflow) 
bool sv_i64_i64_mul_overflow(int64_t a, int64_t b, int64_t* c) {
    if (a == 0 || b == 0) {
        *c = 0;
        return false;
    }
    else if (a > 0) {
        if (b > 0) {
            if (a > INT64_MAX / b) {return true;}
        }
        else {
            if (b < INT64_MIN / a) {return true;}
        }
    }
    else {
        if (b > 0) {
            if (a < INT64_MIN / b) {return true;}
        }
        else {
            if (b < INT64_MAX / a) {return true;}
        }
    }
    *c = a * b;
    return false;
}
#else 
#define sv_i64_i64_mul_overflow __builtin_mul_overflow
#endif

#if TEST || !__has_builtin(__builtin_add_overflow) 
bool sv_i64_i64_add_overflow(int64_t a, int64_t b, int64_t* c) {
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) {
        return true;
    }
    else {
        *c = a + b;
        return false;
    }
}
#else 
#define sv_i64_i64_add_overflow __builtin_add_overflow
#endif

typedef struct {
    int64_t val; // also index in case of err
    ParseIntError err;
} ParseIntResult;

ParseIntResult sv_parse_int(sv stringview) {
    if (stringview.len == 0) {
        return (ParseIntResult){
            .err = PARSE_INT_ERR_EMPTY_STRING,
            .val = 0,
        };
    }
    int64_t result = 0;
    int64_t sign = 1;
    for (size_t i = 0; i < stringview.len; i++) {
        if (i == 0 && stringview.data[0] == '-') {
            sign = -1;
            continue;
        }
        else {
            if ('0' <= stringview.data[i] && stringview.data[i] <= '9') {
                if (sv_i64_i64_mul_overflow(result, 10, &result)) {
                    return (ParseIntResult){
                        .err = PARSE_INT_ERR_OVERFLOW,
                        .val = (int64_t)i,
                    };
                }
                if (sv_i64_i64_add_overflow(result, stringview.data[i] - '0', &result)) {
                    return (ParseIntResult){
                        .err = PARSE_INT_ERR_OVERFLOW,
                        .val = (int64_t)i,
                    };
                }
            }
            else {
                return (ParseIntResult) {
                    .err = PARSE_INT_ERR_INVALID_DIGIT,
                    .val = (int64_t)i,
                };
            }
        }
    }
    if (sv_i64_i64_mul_overflow(result, sign, &result)) {
        return (ParseIntResult){
            .err = PARSE_INT_ERR_OVERFLOW,
            .val = (int64_t)stringview.len,
        };
    }
    else {
        return (ParseIntResult){
            .err = PARSE_INT_OK,
            .val = result,
        };
    }
}
