#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <iterator>

namespace fingera {

template<typename IteratorIn, typename IteratorOut, bool IsLower = true>
void to_hex(IteratorIn begin, IteratorIn end, IteratorOut out);

// Warnning: Did'nt append string terminator '\0'
template<bool IsLower = true>
inline void to_hex(const void *buf, char *str, size_t byte_size);

template<bool IsLower = true>
inline std::string to_hex(const void *buf, size_t byte_size);

template<typename Iterator, bool IsLower = true>
inline std::string to_hex(Iterator begin, Iterator end);

template<typename Container, bool IsLower = true>
inline std::string to_hex(const Container &container);


template<typename IteratorIn, typename IteratorOut>
bool from_hex(IteratorIn begin, IteratorIn end, IteratorOut out);

// Assume buf_size = str_size / 2
inline bool from_hex(const char *str, void *buf, size_t str_size);
inline bool from_hex(const std::string &str, void *buf);

template<typename Container>
inline bool from_hex(const std::string &str, Container &c);

} // namespace fingera

#include <fingera/impl/hex.ipp>
