#ifndef ASSEMBLER_WORDCOUNT_H
#define ASSEMBLER_WORDCOUNT_H

#include <cstddef>
#include <string>
#include <emmintrin.h>
#include <x86intrin.h>
#include <assert.h>

bool is_aligned(void const *ptr, size_t alignment) {
    return (reinterpret_cast<size_t>(ptr) % alignment) == 0;
}

int wordcount_slow(const char *str, size_t size) {
    int count = 0;
    bool was_space = true;
    for (size_t i = 0; i < size; i++) {
        if (was_space && str[i] != ' ') {
            count++;
        }
        was_space = str[i] == ' ';
    }
    return count;
}

int wordcount_asm(const char *str, size_t size) {
    assert(is_aligned(str, sizeof(__m128i)));
    assert((size % sizeof(__m128i)) == 0);

    int words = str[0] != ' ' ? 1 : 0;

    const __m128i SPACE_MASK = _mm_set1_epi8(' ');

    __m128i accumulator = _mm_set1_epi8(0);
    __m128i spaces = _mm_cmpeq_epi8(_mm_load_si128((__m128i *) str), SPACE_MASK);

    for (size_t i = sizeof(__m128i); i < size; i += sizeof(__m128i)) {
        __m128i spaces_left = spaces;
        spaces = _mm_cmpeq_epi8(_mm_load_si128((__m128i *) (str + i)), SPACE_MASK);
        __m128i spaces_left_shifted = _mm_alignr_epi8(spaces, spaces_left, 1);
        __m128i beginnings_mask = _mm_andnot_si128(spaces_left_shifted, spaces_left);

        accumulator = _mm_adds_epu8(_mm_and_si128(_mm_set1_epi8(1), beginnings_mask), accumulator);

        if (_mm_movemask_epi8(accumulator) != 0 || i + 16 >= size - 16) {
            accumulator = _mm_sad_epu8(_mm_set1_epi8(0), accumulator);
            words += _mm_cvtsi128_si32(accumulator);
            accumulator = _mm_srli_si128(accumulator, 8);
            words += _mm_cvtsi128_si32(accumulator);
            accumulator = _mm_set1_epi8(0);
        }
    }

    size_t offset = size - sizeof(__m128i);
    if (str[offset] != ' ') {
        words--;
    }

    words += wordcount_slow(str + offset, sizeof(__m128i));

    return words;
}

int wordcount_fast(const char *str, size_t size) {
    if (size <= sizeof(__m128i) * 2) {
        return wordcount_slow(str, size);
    }

    size_t offset = (16 - reinterpret_cast<size_t>(str) % 16) % 16;
    int count = wordcount_slow(str, offset);
    if (offset != 0 && str[offset - 1] != ' ' && str[offset] != ' ') count--;
    str += offset;
    size -= offset;

    size_t rest = size % 16;
    size -= rest;

    count += wordcount_asm(str, size);

    count += wordcount_slow(str + size, rest);
    if (rest != 0 && str[size - 1] != ' ' && str[size] != ' ') count--;

    return count;
}

#endif //ASSEMBLER_WORDCOUNT_H
