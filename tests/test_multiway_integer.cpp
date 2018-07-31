
#include <fingera/multiway_integer.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fingera/cpu_features.hpp>

BOOST_AUTO_TEST_SUITE(multiway_integer_tests)

BOOST_AUTO_TEST_CASE(base) {
    using namespace fingera;
    using u64_u8_integer = multiway_integer<uint8_t, uint64_t>;
    BOOST_CHECK_EQUAL(u64_u8_integer::op_broadcast(0x1F), 0x1F1F1F1F1F1F1F1Full);
    BOOST_CHECK_EQUAL(u64_u8_integer::op_add(0x1F1F1F1F1F1F1F1Full, 0x0101010101010101ull), 0x2020202020202020ull);
    BOOST_CHECK_EQUAL(u64_u8_integer::op_add(0x1F1F1F1F1F1F1F1Full, 0xF1F1F1F1F1F1F1F1ull), 0x1010101010101010ull);
    BOOST_CHECK_EQUAL(u64_u8_integer::op_xor(0x1F1F1F1F1F1F1F1Full, 0x0101010101010101ull), 0x1E1E1E1E1E1E1E1Eull);

    std::unordered_map<std::string, bool> features;
    get_cpu_features(features);
    for (auto i = features.begin(); i != features.end(); ++i) {
        if (i->second) {
            std::cout << i->first << std::endl;
        }
    }

    uint8_t left[] = {
        1, 2, 3, 4, 5, 6, 7, 8,
    };
    uint8_t right[] = {
        0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    };
    uint8_t result[8];
    u64_u8_integer::save(
        u64_u8_integer::op_add(u64_u8_integer::load(left, 1, 0), u64_u8_integer::load(right, 1, 0)),
        result, 1, 0
    );
    for (size_t i = 0; i < 8; i++) {
        BOOST_CHECK_EQUAL(result[i], left[i] + right[i]);
    }
}

BOOST_AUTO_TEST_SUITE_END()
