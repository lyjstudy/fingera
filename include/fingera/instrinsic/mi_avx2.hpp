#pragma once

#include <fingera/config.hpp>

#if defined(FINGERA_USE_AVX2)

#include <cstdint>
#include <functional>
#include <immintrin.h>
#include <fingera/endian.hpp>

// CPUID: AVX2
// FOR: SHA2(256) RIPEMD160
// mm256 => uint32_t
namespace fingera {
namespace instrinsic {

class mi_avx2 {
public:
    using type = __m256i;
    using impl_type = __m256i;
    using target_type = uint32_t;

    static inline constexpr int way() {
        return sizeof(impl_type) / sizeof(target_type);
    }

    static inline impl_type op_broadcast(target_type value) {
        return _mm256_set1_epi32(value);
    }

    static inline impl_type op_add(impl_type x, impl_type y) {
        return _mm256_add_epi32(x, y);
    }

    static inline impl_type op_xor(impl_type x, impl_type y) {
        return _mm256_xor_si256(x, y);
    }
    static inline impl_type op_or(impl_type x, impl_type y) {
        return _mm256_or_si256(x, y);
    }
    static inline impl_type op_and(impl_type x, impl_type y) {
        return _mm256_and_si256(x, y);
    }
    static inline impl_type op_andnot(impl_type x, impl_type y) {
        return _mm256_andnot_si256(x, y);
    }

    template<int N>
    static inline impl_type op_shr(impl_type x) {
        return _mm256_srli_epi32(x, N);
    }
    template<int N>
    static inline impl_type op_shl(impl_type x) {
        return _mm256_slli_epi32(x, N);
    }
    template<int N>
    static inline impl_type op_rol(impl_type x) {
        return op_or(op_shl<N>(x), op_shr<32 - N>(x));
    }


    template<bool ReadLittleEndian = true>
    static inline impl_type load(const void *mem, size_t blk_size, size_t offset) {
        const char *ptr = static_cast<const char *>(mem) + offset;
        if (ReadLittleEndian) {
            return _mm256_set_epi32(
                read_little<uint32_t>(ptr + blk_size * 0),
                read_little<uint32_t>(ptr + blk_size * 1),
                read_little<uint32_t>(ptr + blk_size * 2),
                read_little<uint32_t>(ptr + blk_size * 3),
                read_little<uint32_t>(ptr + blk_size * 4),
                read_little<uint32_t>(ptr + blk_size * 5),
                read_little<uint32_t>(ptr + blk_size * 6),
                read_little<uint32_t>(ptr + blk_size * 7)
            );
        }
        return _mm256_set_epi32(
            read_big<uint32_t>(ptr + blk_size * 0),
            read_big<uint32_t>(ptr + blk_size * 1),
            read_big<uint32_t>(ptr + blk_size * 2),
            read_big<uint32_t>(ptr + blk_size * 3),
            read_big<uint32_t>(ptr + blk_size * 4),
            read_big<uint32_t>(ptr + blk_size * 5),
            read_big<uint32_t>(ptr + blk_size * 6),
            read_big<uint32_t>(ptr + blk_size * 7)
        );
    }
    template<bool WriteLittleEndian = true>
    static inline void save(impl_type value, void *out, size_t blk_size, size_t offset) {
        char *ptr = static_cast<char *>(out) + offset;
        if (WriteLittleEndian) {
            write_little<uint32_t>(ptr + blk_size * 0, _mm256_extract_epi32(value, 7));
            write_little<uint32_t>(ptr + blk_size * 1, _mm256_extract_epi32(value, 6));
            write_little<uint32_t>(ptr + blk_size * 2, _mm256_extract_epi32(value, 5));
            write_little<uint32_t>(ptr + blk_size * 3, _mm256_extract_epi32(value, 4));
            write_little<uint32_t>(ptr + blk_size * 4, _mm256_extract_epi32(value, 3));
            write_little<uint32_t>(ptr + blk_size * 5, _mm256_extract_epi32(value, 2));
            write_little<uint32_t>(ptr + blk_size * 6, _mm256_extract_epi32(value, 1));
            write_little<uint32_t>(ptr + blk_size * 7, _mm256_extract_epi32(value, 0));
        } else {
            write_big<uint32_t>(ptr + blk_size * 0, _mm256_extract_epi32(value, 7));
            write_big<uint32_t>(ptr + blk_size * 1, _mm256_extract_epi32(value, 6));
            write_big<uint32_t>(ptr + blk_size * 2, _mm256_extract_epi32(value, 5));
            write_big<uint32_t>(ptr + blk_size * 3, _mm256_extract_epi32(value, 4));
            write_big<uint32_t>(ptr + blk_size * 4, _mm256_extract_epi32(value, 3));
            write_big<uint32_t>(ptr + blk_size * 5, _mm256_extract_epi32(value, 2));
            write_big<uint32_t>(ptr + blk_size * 6, _mm256_extract_epi32(value, 1));
            write_big<uint32_t>(ptr + blk_size * 7, _mm256_extract_epi32(value, 0));
        }
    }
};

} // namespace instrinsic
} // namespace fingera

#endif // FINGERA_USE_AVX2