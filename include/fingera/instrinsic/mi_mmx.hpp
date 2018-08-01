#pragma once

#include <fingera/config.hpp>

#if defined(FINGERA_USE_MMX)

#include <cstdint>
#include <functional>
#include <mmintrin.h>
#include <fingera/endian.hpp>

// CPUID: MMX
// FOR: SHA2(256) RIPEMD160
// mm64 => uint32_t
namespace fingera {
namespace instrinsic {

class mi_mmx {
public:
    using type = __m64;
    using impl_type = __m64;
    using target_type = uint32_t;

    static inline constexpr int way() {
        return sizeof(impl_type) / sizeof(target_type);
    }

    static inline impl_type op_broadcast(target_type value) {
        return _mm_set1_pi32(value);
    }

    static inline impl_type op_add(impl_type x, impl_type y) {
        return _mm_add_pi32(x, y);
    }

    static inline impl_type op_xor(impl_type x, impl_type y) {
        return _mm_xor_si64(x, y);
    }
    static inline impl_type op_or(impl_type x, impl_type y) {
        return _mm_or_si64(x, y);
    }
    static inline impl_type op_and(impl_type x, impl_type y) {
        return _mm_and_si64(x, y);
    }
    static inline impl_type op_andnot(impl_type x, impl_type y) {
        return _mm_andnot_si64(x, y);
    }

    template<int N>
    static inline impl_type op_shr(impl_type x) {
        return _mm_srli_pi32(x, N);
    }
    template<int N>
    static inline impl_type op_shl(impl_type x) {
        return _mm_slli_pi32(x, N);
    }
    template<int N>
    static inline impl_type op_rol(impl_type x) {
        return op_or(op_shl<N>(x), op_shr<32 - N>(x));
    }

    template<bool ReadLittleEndian = true>
    static inline impl_type load(const void *mem, size_t blk_size, size_t offset) {
        const char *ptr = static_cast<const char *>(mem) + offset;
        if (ReadLittleEndian) {
            return _mm_set_pi32(
                read_little<uint32_t>(ptr + blk_size * 0),
                read_little<uint32_t>(ptr + blk_size * 1)
            );
        }
        return _mm_set_pi32(
            read_big<uint32_t>(ptr + blk_size * 0),
            read_big<uint32_t>(ptr + blk_size * 1)
        );
    }
    template<bool WriteLittleEndian = true>
    static inline void save(impl_type value, void *out, size_t blk_size, size_t offset) {
        union {
            int64_t mm;
            uint32_t data[2];
        };
        mm = _m_to_int64(value);
        char *ptr = static_cast<char *>(out) + offset;
        for (int i = 0; i < 2; i++) {
            if (WriteLittleEndian) {
                write_little(ptr + blk_size * i, data[1 - i]);
            } else {
                write_big(ptr + blk_size * i, data[1 - i]);
            }
        }
    }
};

} // namespace instrinsic
} // namespace fingera

#endif // FINGERA_USE_MMX