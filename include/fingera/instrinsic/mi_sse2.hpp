#pragma once

#include <cstdint>
#include <functional>
#include <immintrin.h>
#include <fingera/endian.hpp>

// CPUID: SSE2
// FOR: SHA2(256) RIPEMD160
// mm128 => uint32_t
namespace fingera {
namespace instrinsic {

class mi_sse2 {
public:
    using type = __m128i;
    using impl_type = __m128i;
    using target_type = uint32_t;

    static inline constexpr int way() {
        return sizeof(impl_type) / sizeof(target_type);
    }

    static inline impl_type op_broadcast(target_type value) {
        return _mm_set1_epi32(value);
    }

    static inline impl_type op_add(impl_type x, impl_type y) {
        return _mm_add_epi32(x, y);
    }

    static inline impl_type op_xor(impl_type x, impl_type y) {
        return _mm_xor_si128(x, y);
    }
    static inline impl_type op_or(impl_type x, impl_type y) {
        return _mm_or_si128(x, y);
    }
    static inline impl_type op_and(impl_type x, impl_type y) {
        return _mm_and_si128(x, y);
    }
    static inline impl_type op_andnot(impl_type x, impl_type y) {
        return _mm_andnot_si128(x, y);
    }

    template<int N>
    static inline impl_type op_shr(impl_type x) {
        return _mm_srli_epi32(x, N);
    }
    template<int N>
    static inline impl_type op_shl(impl_type x) {
        return _mm_slli_epi32(x, N);
    }
    template<int N>
    static inline impl_type op_rol(impl_type x) {
        return op_or(op_shl<N>(x), op_shr<32 - N>(x));
    }

    template<bool ReadLittleEndian = true>
    static inline impl_type load(const void *mem, size_t blk_size, size_t offset) {
        const char *ptr = static_cast<const char *>(mem) + offset;
        if (ReadLittleEndian) {
            return _mm_set_epi32(
                read_little<uint32_t>(ptr + blk_size * 0),
                read_little<uint32_t>(ptr + blk_size * 1),
                read_little<uint32_t>(ptr + blk_size * 2),
                read_little<uint32_t>(ptr + blk_size * 3)
            );
        }
        return _mm_set_epi32(
            read_big<uint32_t>(ptr + blk_size * 0),
            read_big<uint32_t>(ptr + blk_size * 1),
            read_big<uint32_t>(ptr + blk_size * 2),
            read_big<uint32_t>(ptr + blk_size * 3)
        );
    }
    template<bool WriteLittleEndian = true>
    static inline void save(impl_type value, void *out, size_t blk_size, size_t offset) {
        union {
            __m128i mm;
            uint32_t data[4];
        };
        _mm_storeu_si128(&mm, value);
        char *ptr = static_cast<char *>(out) + offset;
        for (int i = 0; i < 4; i++) {
            if (WriteLittleEndian) {
                write_little(ptr + blk_size * i, data[3 - i]);
            } else {
                write_big(ptr + blk_size * i, data[3 - i]);
            }
        }
    }
};

} // namespace instrinsic
} // namespace fingera
