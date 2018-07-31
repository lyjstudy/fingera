#pragma once

#include <cstdint>
#include <boost/endian/conversion.hpp>

namespace fingera {

namespace detail {
template<typename T> struct accessor {
    using type = T;
};

template<> struct accessor<float> {
    using type = uint32_t;
};
template<> struct accessor<double> {
    using type = uint64_t;
};
} // detail

////////////////////////////////////////////conversion/////////////////////////////////////////////
template<typename T> inline T to_little(T value) noexcept {
    return boost::endian::native_to_little<T>(value);
}
template<typename T> inline T to_big(T value) noexcept {
    return boost::endian::native_to_big<T>(value);
}
template<typename T> inline T from_little(T value) noexcept {
    return boost::endian::little_to_native<T>(value);
}
template<typename T> inline T from_big(T value) noexcept {
    return boost::endian::big_to_native<T>(value);
}

///////////////////////////////////////////memory endian///////////////////////////////////////////
template<typename T> inline T read_little(const void *buffer) noexcept {
    using type = typename detail::accessor<T>::type;
    static_assert(sizeof(type) == sizeof(T), "bad accessor");
    type value = from_little(*(const type *)buffer);
    return *(T *)&value;
}
template<typename T> inline void write_little(void *buffer, T data) noexcept {
    using type = typename detail::accessor<T>::type;
    static_assert(sizeof(type) == sizeof(T), "bad accessor");
    *(type *)buffer = to_little(*(const type *)&data);
}
template<typename T> inline T read_big(const void *buffer) noexcept {
    using type = typename detail::accessor<T>::type;
    static_assert(sizeof(type) == sizeof(T), "bad accessor");
    type value = from_big(*(const type *)buffer);
    return *(T *)&value;
}
template<typename T> inline void write_big(void *buffer, T data) noexcept {
    using type = typename detail::accessor<T>::type;
    static_assert(sizeof(type) == sizeof(T), "bad accessor");
    *(type *)buffer = to_big(*(const type *)&data);
}

} // namespace fingera