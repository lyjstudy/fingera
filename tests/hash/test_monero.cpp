
#include <fingera/hash/monero.hpp>
#include <boost/test/unit_test.hpp>

#include <cstring>
#include <vector>
#include <iostream>
#include <fingera/hex.hpp>


BOOST_AUTO_TEST_SUITE(monero_tests)

BOOST_AUTO_TEST_CASE(base) {
    // 0707c3d4a9db055ced477105ab5607d19fa12cf3f538f0e4e724f3bde40ddc05d16a9a068001885b038000b9ba16ee6a563456fc9ec93af68675a295f592992645a54175b375e66e32429e01
    std::vector<uint8_t> data;
    char hash[32];

    const char *hex_data = "0707c3d4a9db055ced477105ab5607d19fa12cf3f538f0e4e724f3bde40ddc05d16a9a068001885b038000b9ba16ee6a563456fc9ec93af68675a295f592992645a54175b375e66e32429e01";
    const char *hex_hash = "b1bce924940f6118736b208af75221012cd7fb4602912cc6771938740ebc0400";

    BOOST_CHECK(fingera::from_hex(hex_data, data));
    fingera::hash::monero_standard(&data[0], data.size(), hash);
    // BOOST_CHECK_EQUAL(fingera::to_hex(hash, 32), hex_hash);
    std::cout << fingera::to_hex(hash, 32) << std::endl;

    fingera::hash::monero_cpu_fast(&data[0], data.size(), hash);
    std::cout << fingera::to_hex(hash, 32) << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
