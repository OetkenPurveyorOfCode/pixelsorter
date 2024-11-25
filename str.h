#ifndef STR_H
#define STR_H

#include <stdbool.h>
#include <stdint.h>

#ifndef SV_ASSERT 
#include <assert.h>
#define SV_ASSERT assert
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;
typedef uint32_t u32;
typedef size_t usize;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef ptrdiff_t isize;

#define SV(literal) ((sv){.data=("" literal), .len=sizeof(literal)-1})
#define SV_U64(number) sv_from_u64(number, (char[65]){0})
#define SV_U64B(number) sv_from_u64_binary(number, (char[65]){0})

typedef struct {
    const char* data;
    size_t len;
} sv;

sv sv_from_parts(const char* data, size_t length);
usize fmt_u64(u64 num, char* buffer);
sv sv_from_u64(u64 num, char* buffer);
usize fmt_u64_binary(u64 num, char* buffer);
sv sv_from_u64_binary(u64 num, char* buffer);
void sv_trim_in_place(sv* stringview);
void print_sv(sv stringview);
#endif

//////////////////////////////////////////////////
#ifndef NO_IMPLEMENTATION
sv sv_from_parts(const char* data, size_t len) {
    SV_ASSERT(data);
    return (sv){
        .data=data,
        .len=len,
    };
}

//internal
u32 sv_num_digits10(u64 num) {
    u32 num_digits = 1;
    for(;;) {
        if (num < 10) return num_digits;
        if (num < 100) return num_digits + 1;
        if (num < 1000) return num_digits + 2;
        if (num < 10000) return num_digits + 3;
        num /= 10000;
        num_digits += 4;
    }
    return num_digits;
}

#if defined(_MSC_VER) && !defined(__clang__)
    #include <intrin.h>
    uint32_t sv_num_digits2(uint64_t number) {
        return (uint32_t)(64 - __lzcnt64(number));
    }
#else 
    uint32_t sv_num_digits2(uint64_t number) {
        return (uint32_t)(64 - __builtin_clzll(number)); 
    }
#endif

usize fmt_u64(u64 num, char* buffer) {
    u32 num_digits = sv_num_digits10(num);
    u32 pos = num_digits - 1;
    while(num >= 10) {
        buffer[pos] = num % 10 + '0';
        num /= 10;
        pos -= 1;
    }
    SV_ASSERT(pos == 0);
    buffer[0] = (char)num + '0';
    return num_digits;
}

sv sv_from_u64(u64 num, char* buffer) {
    usize len = fmt_u64(num, buffer);
    return (sv){.data=buffer, .len=len};
}

usize fmt_u64_binary(u64 num, char* buffer) {
    u32 num_digits = sv_num_digits2(num);
    u32 pos = num_digits - 1;
    while (num >= 2) {
        buffer[pos] = num % 2 + '0';
        num /= 2;
        pos -= 1;
    }
    SV_ASSERT(pos == 0);
    buffer[0] = (char)num + '0';
    return num_digits;
}

sv sv_from_u64_binary(u64 num, char* buffer) {
    usize len = fmt_u64_binary(num, buffer);
    return (sv){.data=buffer, .len=len};
}

usize fmt_i64(i64 num, char* buffer) {
    if (num < 0) {
        buffer[0] = '-';
        return fmt_u64((u64)(-num), &buffer[1]);
    }
    else return fmt_u64((u64)(num), buffer);
}

bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c== '\f' || c == '\r';
}

void sv_trim_in_place(sv* stringview) {
    size_t begin = 0;
    while (begin < stringview->len && is_space(stringview->data[begin])) {
        begin++;
    }
    size_t end = stringview->len-1;
    while (end >= begin && end < stringview->len && is_space(stringview->data[end])) {
        end--;
    }
    *stringview = (sv){
        .data = &stringview->data[begin],
        .len = end-begin+1,
    };
}
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

#if TEST 
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#define assert_eq_int(a, b) assert_eq_int_impl(__FILE__, __LINE__, (a) == (b), a, b)

void assert_eq_int_impl(const char* file, int line, bool cond, int64_t a, int64_t b) {
    if (!cond) {
        fprintf(stderr, "%s:line %d: Assertion failed: %" PRIi64 " == %" PRIi64 "\n", file, line, a, b);
        exit(-1);
    }
}
#include "xoshiro256plusplus.h"

void test_sv_intrinsics(void) {
    srand((unsigned int)time(NULL));
    s[0] = (uint64_t)rand();
    s[1] = (uint64_t)rand();
    s[2] = (uint64_t)rand();
    s[3] = (uint64_t)rand();
    for (int64_t i = 0; i < 100000000; i++) {
        int64_t a = (int64_t)next();
        int64_t b = (int64_t)next();

        //printf("a %" PRIi64 ", b %" PRIi64"\n", a, b);
        //exit(-1);
        int64_t custom_result = 0;
        int64_t built_in_result = 0;
        bool custom = (sv_i64_i64_mul_overflow)(a, b, &custom_result);
        bool built_in = __builtin_mul_overflow(a, b, &built_in_result);
        assert_eq_int(custom, built_in);
        if (custom == false && custom_result != built_in_result) {
            fprintf(stderr, "Failed: %" PRIi64 " == %" PRIi64 " == %" PRIi64 " * %" PRIi64 " \n", built_in_result, custom_result, a, b);
            exit(-1);
        }
    }

    for (int64_t i = 0; i < 100000000; i++) {
        int64_t a = (int64_t)next();
        int64_t b = (int64_t)next();

        //printf("a %" PRIi64 ", b %" PRIi64"\n", a, b);
        //exit(-1);
        int64_t custom_result = 0;
        int64_t built_in_result = 0;
        bool custom = (sv_i64_i64_add_overflow)(a, b, &custom_result);
        bool built_in = __builtin_add_overflow(a, b, &built_in_result);
        assert_eq_int(custom, built_in);
        if (custom == false && custom_result != built_in_result) {
            fprintf(stderr, "Failed: %" PRIi64 " == %" PRIi64 " == %" PRIi64 " + %" PRIi64 " \n", built_in_result, custom_result, a, b);
            exit(-1);
        }
    }
}

void test_sv_parse_int() {
    sv sv1 = SV("1234");
    ParseIntResult r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_OK);
    assert_eq_int(r.val, 1234);

    sv1 = SV("-999");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_OK);
    assert_eq_int(r.val, -999);

    sv1 = SV(" 3");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 0);

    sv1 = SV("3 ");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 1);

    sv1 = SV("0");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_OK);
    assert_eq_int(r.val, 0);

    sv1 = SV("");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_EMPTY_STRING);
    assert_eq_int(r.val, 0);

    sv1 = SV("--2");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 1);

    sv1 = SV("2-3");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 1);

    sv1 = SV("zur");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 0);

    sv1 = SV("44zur");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_INVALID_DIGIT);
    assert_eq_int(r.val, 2);

    sv1 = SV_U64(0xFFFFFFFFFFFFFFFF);
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_OVERFLOW);

    sv1 = SV("444444444444444444444444444444444444444444444444444444444444444");
    r = sv_parse_int(sv1);
    assert(r.err == PARSE_INT_ERR_OVERFLOW);
}

void test_sv_trim_in_place() {
    // Test case 1: String with leading and trailing spaces
    sv sv1 = SV("  hello world  ");
    sv_trim_in_place(&sv1);
    assert_eq_int((int)sv1.len, 11);
    assert(strncmp(sv1.data, "hello world", sv1.len) == 0);

    // Test case 2: String with no leading or trailing spaces
    sv sv2 = SV("helloworld");
    sv_trim_in_place(&sv2);
    assert(sv2.len == 10);
    assert(strncmp(sv2.data, "helloworld", sv2.len) == 0);

    // Test case 3: String with only spaces
    char test3[] = "      ";
    sv sv3 = {test3, strlen(test3)};
    sv_trim_in_place(&sv3);
    assert(sv3.len == 0);
    assert(sv3.data == test3 + strlen(test3));  // Points to the end of the string

    // Test case 4: Empty string
    char test4[] = "";
    sv sv4 = {test4, strlen(test4)};
    sv_trim_in_place(&sv4);
    assert(sv4.len == 0);
    assert(sv4.data == test4);  // Should point to the original start

    // Test case 5: String with only leading spaces
    char test5[] = "   hello";
    sv sv5 = {test5, strlen(test5)};
    sv_trim_in_place(&sv5);
    assert(sv5.len == 5);
    assert(strncmp(sv5.data, "hello", sv5.len) == 0);

    // Test case 6: String with only trailing spaces
    char test6[] = "hello   ";
    sv sv6 = {test6, strlen(test6)};
    sv_trim_in_place(&sv6);
    assert(sv6.len == 5);
    assert(strncmp(sv6.data, "hello", sv6.len) == 0);
}

int main() {
    test_sv_trim_in_place();
    test_sv_parse_int();
    test_sv_intrinsics();
    printf("All tests passed\n");
    return 0;
}
#endif
#endif

