#pragma once

#include <cstdint>
#include <functional>
#include <fingera/endian.hpp>

namespace fingera {

template<typename TargetType, typename ImplType>
class multiway_integer {
    static_assert((sizeof(ImplType) % sizeof(TargetType)) == 0 && sizeof(ImplType) >= sizeof(TargetType), "not allowed");

    typedef union {
        ImplType value;
        TargetType stub[sizeof(ImplType) / sizeof(TargetType)];

        inline void set(TargetType val, int &idx) {
            stub[idx++] = val;
        }
    } access_stub;
    static_assert(sizeof(access_stub) == sizeof(ImplType), "add pack(1)");

    static inline ImplType _for_each(std::function<void (TargetType &)> Op) {
        access_stub r;
        for (int i = 0; i < way(); i++) {
            Op(r.stub[i]);
        }
        return r.value;
    }
    static inline ImplType _for_each(std::function<void (TargetType &, TargetType)> Op,
                            ImplType x) {
        access_stub r;
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        for (int i = 0; i < way(); i++) {
            Op(r.stub[i], px->stub[i]);
        }
        return r.value;
    }
    static inline ImplType _for_each(std::function<void (TargetType &, TargetType, TargetType)> Op,
                            ImplType x, ImplType y) {
        access_stub r;
        access_stub *px = reinterpret_cast<access_stub *>(&x);
        access_stub *py = reinterpret_cast<access_stub *>(&y);
        for (int i = 0; i < way(); i++) {
            Op(r.stub[i], px->stub[i], py->stub[i]);
        }
        return r.value;
    }
public:
    using type = ImplType;
    using impl_type = ImplType;
    using target_type = TargetType;

    static inline constexpr int way() {
        return sizeof(ImplType) / sizeof(TargetType);
    }

    static inline impl_type op_broadcast(target_type value) {
        return _for_each([&](target_type &val) {
            val = value;
        });
    }

    static inline impl_type op_add(impl_type x, impl_type y) {
        return _for_each([](target_type &val, target_type x, target_type y) {
            val = x + y;
        }, x, y);
    }
    static inline impl_type op_xor(impl_type x, impl_type y) {
        return x ^ y;
    }
    static inline impl_type op_or(impl_type x, impl_type y) {
        return x | y;
    }
    static inline impl_type op_and(impl_type x, impl_type y) {
        return x & y;
    }
    static inline impl_type op_andnot(impl_type x, impl_type y) {
        return ~x & y;
    }

    template<int N>
    static inline impl_type op_shr(impl_type x) {
        return _for_each([&](target_type &val, target_type x) {
            val = x >> N;
        }, x);
    }
    template<int N>
    static inline impl_type op_shl(impl_type x) {
        return _for_each([&](target_type &val, target_type x) {
            val = x << N;
        }, x);
    }
    template<int N>
    static inline impl_type op_rol(impl_type x) {
        return _for_each([&](target_type &val, target_type x) {
            val = (x << N) | (x >> (sizeof(target_type) * 8 - N));
        }, x);
    }

    template<bool ReadLittleEndian = true>
    static inline impl_type load(const void *mem, size_t blk_size, size_t offset) {
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
    static inline void save(impl_type value, void *out, size_t blk_size, size_t offset) {
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
        inline data_wrap() {
        }
        inline data_wrap(DataType value) {
            for (int i = 0; i < WayCount; i++)
                stub[i] = value;
        }
        inline data_wrap(const data_wrap &other) {
            memcpy(stub, other.stub, sizeof(stub));
        }
        inline data_wrap &operator=(const data_wrap &other) {
            memcpy(stub, other.stub, sizeof(stub));
            return *this;
        }
        inline data_wrap &op_add(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] += other.stub[i];
            }
            return *this;
        }
        inline data_wrap &op_xor(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] ^= other.stub[i];
            }
            return *this;
        }
        inline data_wrap &op_or(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] |= other.stub[i];
            }
            return *this;
        }
        inline data_wrap &op_and(const data_wrap &other) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] &= other.stub[i];
            }
            return *this;
        }
        inline data_wrap &op_not() {
            for (int i = 0; i < WayCount; i++) {
                stub[i] = ~stub[i];
            }
            return *this;
        }
        inline data_wrap &op_shr(int cnt) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] >>= cnt;
            }
            return *this;
        }
        inline data_wrap &op_shl(int cnt) {
            for (int i = 0; i < WayCount; i++) {
                stub[i] <<= cnt;
            }
            return *this;
        }
    };
    using type = data_wrap<UsedType>;
    using used_type = UsedType;
public:

    static inline constexpr int way() {
        return WayCount;
    }

    static inline type op_broadcast(used_type value) {
        return type(value);
    }

    static inline type op_add(type x, type y) {
        return x.op_add(y);
    }
    static inline type op_xor(type x, type y) {
        return x.op_xor(y);
    }
    static inline type op_or(type x, type y) {
        return x.op_or(y);
    }
    static inline type op_and(type x, type y) {
        return x.op_and(y);
    }
    static inline type op_andnot(type x, type y) {
        return x.op_not().op_and(y);
    }

    template<int N>
    static inline type op_shr(type x) {
        return x.op_shr(N);
    }
    template<int N>
    static inline type op_shl(type x) {
        return x.op_shl(N);
    }
    template<int N>
    static inline type op_rol(type x) {
        type right = op_shr<sizeof(used_type) * 8 - N>(x);
        type left = op_shl<N>(x);
        return left.op_or(right);
    }

    template<bool ReadLittleEndian = true>
    static inline type load(const void *mem, size_t blk_size, size_t offset) {
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
    static inline void save(type value, void *out, size_t blk_size, size_t offset) {
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