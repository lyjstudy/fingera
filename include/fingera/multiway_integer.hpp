#pragma once

#include <cstdint>
#include <fingera/endian.hpp>
#include <fingera/config.hpp>

namespace fingera {

template<typename TargetType, typename ImplType>
class multiway_integer {
    static_assert((sizeof(ImplType) % sizeof(TargetType)) == 0 && sizeof(ImplType) >= sizeof(TargetType), "not allowed");

    typedef union {
        ImplType value;
        TargetType stub[sizeof(ImplType) / sizeof(TargetType)];

        FINGERA_FORCEINLINE void set(TargetType val, int &idx) {
            stub[idx++] = val;
        }
    } access_stub;
    static_assert(sizeof(access_stub) == sizeof(ImplType), "add pack(1)");

public:
    using type = ImplType;
    using impl_type = ImplType;
    using target_type = TargetType;

    static FINGERA_FORCEINLINE constexpr int way() {
        return sizeof(ImplType) / sizeof(TargetType);
    }

    static FINGERA_FORCEINLINE impl_type op_broadcast(target_type value) {
        access_stub r;
        for (int i = 0; i < way(); i++) {
            r.stub[i] = value;
        }
        return r.value;
    }

    static FINGERA_FORCEINLINE impl_type op_add(impl_type x, impl_type y) {
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        access_stub *py = reinterpret_cast<access_stub *>(&y);
        for (int i = 0; i < way(); i++) {
            px->stub[i] += py->stub[i];
        }
        return px->value;
    }
    static FINGERA_FORCEINLINE impl_type op_xor(impl_type x, impl_type y) {
        return x ^ y;
    }
    static FINGERA_FORCEINLINE impl_type op_or(impl_type x, impl_type y) {
        return x | y;
    }
    static FINGERA_FORCEINLINE impl_type op_and(impl_type x, impl_type y) {
        return x & y;
    }
    static FINGERA_FORCEINLINE impl_type op_andnot(impl_type x, impl_type y) {
        return ~x & y;
    }

    template<int N>
    static FINGERA_FORCEINLINE impl_type op_shr(impl_type x) {
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        for (int i = 0; i < way(); i++) {
            px->stub[i] >>= N;
        }
        return px->value;
    }
    template<int N>
    static FINGERA_FORCEINLINE impl_type op_shl(impl_type x) {
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        for (int i = 0; i < way(); i++) {
            px->stub[i] <<= N;
        }
        return px->value;
    }
    template<int N>
    static FINGERA_FORCEINLINE impl_type op_rol(impl_type x) {
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        for (int i = 0; i < way(); i++) {
            auto v = px->stub[i];
            px->stub[i] = (v << N) | (v >> (sizeof(target_type) * 8 - N));
        }
        return px->value;
    }

    template<bool ReadLittleEndian = true>
    static FINGERA_FORCEINLINE impl_type load(const void *mem, size_t blk_size, size_t offset) {
        const char *ptr = static_cast<const char *>(mem) + offset;
        access_stub r;
        for (int i = 0; i < way(); i++) {
            if (ReadLittleEndian) {
                r.stub[i] = read_little<target_type>(ptr + blk_size * i);
            } else {
                r.stub[i] = read_big<target_type>(ptr + blk_size * i);
            }
        }
        return r.value;
    }
    template<bool WriteLittleEndian = true>
    static FINGERA_FORCEINLINE void save(impl_type value, void *out, size_t blk_size, size_t offset) {
        access_stub *pvalue = reinterpret_cast<access_stub *>(&value);
        char *ptr = static_cast<char *>(out) + offset;
        for (int i = 0; i < way(); i++) {
            if (WriteLittleEndian) {
                write_little<target_type>(ptr + blk_size * i, pvalue->stub[i]);
            } else {
                write_big<target_type>(ptr + blk_size * i, pvalue->stub[i]);
            }
        }
    }
};

// simulate avx512 multiway_integer_slow<uint32_t, 16>
template<typename UsedType, int WayCount>
class multiway_integer_slow {
    static_assert(WayCount >= 1, "not allowed");
public:
    template<typename DataType>
    class data_wrap {
    public:
        DataType stub[WayCount];
        FINGERA_FORCEINLINE data_wrap() {
        }
        FINGERA_FORCEINLINE data_wrap(DataType value) {
            for (int i = 0; i < WayCount; i++)
                stub[i] = value;
        }
        FINGERA_FORCEINLINE data_wrap(const data_wrap &other) {
            memcpy(stub, other.stub, sizeof(stub));
        }
        FINGERA_FORCEINLINE data_wrap &operator=(const data_wrap &other) {
            memcpy(stub, other.stub, sizeof(stub));
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_add(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] += other.stub[i];
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_xor(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] ^= other.stub[i];
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_or(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] |= other.stub[i];
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_and(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] &= other.stub[i];
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_not() {
            for (int i = 0; i < WayCount; i++) {
                stub[i] = ~stub[i];
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_shr(int cnt) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] >>= cnt;
            }
            return *this;
        }
        FINGERA_FORCEINLINE data_wrap &op_shl(int cnt) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] <<= cnt;
            }
            return *this;
        }
    };
    using type = data_wrap<UsedType>;
    using used_type = UsedType;
public:

    static FINGERA_FORCEINLINE constexpr int way() {
        return WayCount;
    }

    static FINGERA_FORCEINLINE type op_broadcast(used_type value) {
        return type(value);
    }

    static FINGERA_FORCEINLINE type op_add(type x, type y) {
        return x.op_add(y);
    }
    static FINGERA_FORCEINLINE type op_xor(type x, type y) {
        return x.op_xor(y);
    }
    static FINGERA_FORCEINLINE type op_or(type x, type y) {
        return x.op_or(y);
    }
    static FINGERA_FORCEINLINE type op_and(type x, type y) {
        return x.op_and(y);
    }
    static FINGERA_FORCEINLINE type op_andnot(type x, type y) {
        return x.op_not().op_and(y);
    }

    template<int N>
    static FINGERA_FORCEINLINE type op_shr(type x) {
        return x.op_shr(N);
    }
    template<int N>
    static FINGERA_FORCEINLINE type op_shl(type x) {
        return x.op_shl(N);
    }
    template<int N>
    static FINGERA_FORCEINLINE type op_rol(type x) {
        type right = op_shr<sizeof(used_type) * 8 - N>(x);
        type left = op_shl<N>(x);
        return left.op_or(right);
    }

    template<bool ReadLittleEndian = true>
    static FINGERA_FORCEINLINE type load(const void *mem, size_t blk_size, size_t offset) {
        const char *ptr = static_cast<const char *>(mem) + offset;
        type r;
        for (int i = 0; i < way(); i++) {
            if (ReadLittleEndian) {
                r.stub[i] = read_little<used_type>(ptr + blk_size * i);
            } else {
                r.stub[i] = read_big<used_type>(ptr + blk_size * i);
            }
        }
        return r;
    }
    template<bool WriteLittleEndian = true>
    static FINGERA_FORCEINLINE void save(type value, void *out, size_t blk_size, size_t offset) {
        char *ptr = static_cast<char *>(out) + offset;
        for (int i = 0; i < way(); i++) {
            if (WriteLittleEndian) {
                write_little<used_type>(ptr + blk_size * i, value.stub[i]);
            } else {
                write_big<used_type>(ptr + blk_size * i, value.stub[i]);
            }
        }
    }
};

} // namespace fingera