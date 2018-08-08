#pragma once

#include <cstring>

namespace fingera {
namespace hash {

void monero_standard(const void *block_blob, size_t length, void *result);

void monero_cpu_fast(const void *block_blob, size_t length, void *result);

} // namespace hash
} // namespace fingera