#pragma once

#include <cstdint>

namespace fingera {
namespace hash {

template<typename Instr>
class multiway_ripemd160 {
public:
    using type = typename Instr::impl_type;
protected:
};

} // namespace hash
} // namespace fingera
