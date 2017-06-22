#ifndef ASSEMBLER_MEMCPY_H
#define ASSEMBLER_MEMCPY_H

#include <cassert>
#include <cstdint>
#include <emmintrin.h>

bool is_aligned(void const *ptr, size_t alignment) {
    return (reinterpret_cast<size_t>(ptr) % alignment) == 0;
}

void *memcpy_simple(void *dest, void const *src, size_t num) {
    for (size_t i = 0; i < num; ++i) {
        *((char *) dest + i) = *((char *) src + i);
    }
    return dest;
}

void *memcpy_asm(void *dest, void const *src, size_t num) {

    if (num <= sizeof(__m128i) * 2) {
        return memcpy_simple(dest, src, num);
    }

    void *const result_dest = dest;

    size_t offset = (16 - reinterpret_cast<size_t>(dest) % 16) % 16;
    memcpy_simple(dest, src, offset);

    dest = (char *) dest + offset;
    src = (char *) src + offset;
    num -= offset;

    size_t rest = num % 16;
    num -= rest;

    assert(is_aligned(dest, sizeof(__m128i)));
    assert(is_aligned(src, sizeof(__m128i)));
    assert((num % sizeof(__m128i)) == 0);

    __m128i tmp;
    __asm__ volatile(
    "1:"
            "movdqa     (%0), %3\n"
            "movntdq    %3, (%1)\n"
            "add        $16, %0\n"
            "add        $16, %1\n"
            "sub        $16, %2\n"
            "jnz        1b\n"
    : "=r"(dest), "=r"(src), "=r"(num), "=x"(tmp)
    : "0"(dest), "1"(src), "2"(num)
    : "memory", "cc"
    );

    _mm_sfence();

    memcpy_simple((char *) dest + num, (char *) src + num, rest);
    return result_dest;
}

template<typename T>
void *element_wise_copy(void *dest, void const *src, size_t num) __attribute__((noinline));

template<typename T>
void *element_wise_copy(void *dest, void const *src, size_t num) {
    assert(is_aligned(dest, sizeof(T)));
    assert(is_aligned(src, sizeof(T)));
    assert((num % sizeof(T)) == 0);

    T *cdest = static_cast<T *>(dest);
    T const *csrc = static_cast<T const *>(src);
    size_t n = num / sizeof(T);

    for (size_t i = 0; i != n; ++i)
        cdest[i] = csrc[i];
    return dest;
}

void *copy_nt(void *dest, void const *src, size_t num) __attribute__((noinline));

void *copy_nt(void *dest, void const *src, size_t num) {
    typedef __m128i T;

    assert(is_aligned(dest, sizeof(T)));
    assert(is_aligned(src, sizeof(T)));
    assert((num % sizeof(T)) == 0);

    T *cdest = static_cast<T *>(dest);
    T const *csrc = static_cast<T const *>(src);
    size_t n = num / sizeof(T);

    for (size_t i = 0; i != n; ++i)
        _mm_stream_si128(cdest + i, csrc[i]);
    return dest;
}

void *copy_nt_asm(void *dest, void const *src, size_t num) {
    assert(is_aligned(dest, sizeof(__m128i)));
    assert(is_aligned(src, sizeof(__m128i)));
    assert((num % sizeof(__m128i)) == 0);

    __m128i tmp;
    __asm__ volatile(
    "1:"
            "movdqa     (%0), %3\n"
            "movntdq    %3, (%1)\n"
            "add        $16, %0\n"
            "add        $16, %1\n"
            "sub        $16, %2\n"
            "jnz        1b\n"
    : "=r"(dest), "=r"(src), "=r"(num), "=x"(tmp)
    : "0"(dest), "1"(src), "2"(num)
    : "memory", "cc"
    );
    return dest;
}

#define declare_element_wise_asm(size, type)                                          \
    void *element_wise_copy_##size##_asm(void* dest, void const* src, size_t num) {   \
        static_assert(sizeof(type) == size);                                          \
        assert(is_aligned(dest, size));                                               \
        assert(is_aligned(src, size));                                                \
        assert((num % size) == 0);                                                    \
                                                                                      \
        type tmp;                                                                     \
                                                                                      \
        __asm__ volatile(                                                             \
            "lea        (%0, %2, " #size "), %0\n"                                    \
            "lea        (%1, %2, " #size "), %1\n"                                    \
            "neg        %2\n"                                                         \
            "jnc        2f\n"                                                         \
        "1:"                                                                          \
            "mov        (%1, %2, " #size "), %3\n"                                    \
            "mov        %3, (%0, %2, " #size ")\n"                                    \
            "inc        %2\n"                                                         \
            "jnz        1b\n"                                                         \
        "2:"                                                                          \
            : "=r"(dest), "=r"(src), "=r"(num), "=r"(tmp)                             \
            : "0"(dest),  "1"(src),  "2"(num / size)                                  \
            : "memory", "cc"                                                          \
        );                                                                            \
        return dest;                                                                  \
    }

declare_element_wise_asm(1, uint8_t)

declare_element_wise_asm(2, uint16_t)

declare_element_wise_asm(4, uint32_t)

declare_element_wise_asm(8, uint64_t)

#endif //ASSEMBLER_MEMCPY_H