
#include <fingera/hex.hpp>
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <fingera/span.hpp>

BOOST_AUTO_TEST_SUITE(hex_tests)

BOOST_AUTO_TEST_CASE(hex) {

    using namespace fingera;

    char test_string[] = {
        '0', '1', '2',
        'a', 'b', '2',
        'c', 'f', '5', '3',
    };

    char *test_ptr = test_string;

    std::vector<uint8_t> data = {
        1, 0x2a, 0xb2, 0xcf, 0x53,
    };

    span<int> def_constructor;
    // BOOST_CHECK_EQUAL(def_constructor.data(), nullptr);
    BOOST_CHECK_EQUAL(def_constructor.size(), 0);
    span<int> null_constructor(nullptr);
    // BOOST_CHECK_EQUAL(def_constructor.data(), nullptr);
    BOOST_CHECK_EQUAL(def_constructor.size(), 0);
    span<char> ptr_constructor(test_ptr, sizeof(test_string));
    span<char> arr_constructor(test_string);
    BOOST_CHECK_EQUAL(arr_constructor.data(), test_string);
    BOOST_CHECK_EQUAL(arr_constructor.size(), sizeof(test_string));
    span<uint8_t> con_constructor(data);
    BOOST_CHECK_EQUAL(con_constructor.data(), data.data());
    BOOST_CHECK_EQUAL(con_constructor.size(), data.size());
    span<uint8_t> copy_constructor(con_constructor);
    BOOST_CHECK_EQUAL(copy_constructor.data(), data.data());
    BOOST_CHECK_EQUAL(copy_constructor.size(), data.size());
    span<uint8_t> equal = copy_constructor;
    BOOST_CHECK_EQUAL(equal.data(), data.data());
    BOOST_CHECK_EQUAL(equal.size(), data.size());

    BOOST_CHECK(std::equal(equal.begin(), equal.end(), data.begin(), data.end()));
    BOOST_CHECK(std::equal(equal.rbegin(), equal.rend(), data.rbegin(), data.rend()));

    auto r = to_hex(equal);

    const span<char> ref_str(test_string);
    BOOST_CHECK(std::equal(r.rbegin(), r.rend(), ref_str.rbegin(), ref_str.rend()));

}

BOOST_AUTO_TEST_SUITE_END()
