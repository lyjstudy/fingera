#pragma once

namespace fingera {

////////////////////////////////////////////conversion/////////////////////////////////////////////
template<typename T> inline T to_little(T value) noexcept;
template<typename T> inline T to_big(T value) noexcept;
template<typename T> inline T from_little(T value) noexcept;
template<typename T> inline T from_big(T value) noexcept;

///////////////////////////////////////////memory endian///////////////////////////////////////////
template<typename T> inline T read_little(const void *buffer) noexcept;
template<typename T> inline void write_little(void *buffer, T data) noexcept;
template<typename T> inline T read_big(const void *buffer) noexcept;
template<typename T> inline void write_big(void *buffer, T data) noexcept;

} // namespace fingera

#include <fingera/impl/endian.ipp>