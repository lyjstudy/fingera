#pragma once

#include <unordered_map>

namespace fingera {

bool get_cpu_features(std::unordered_map<std::string, bool> &features);

} // namespace fingera