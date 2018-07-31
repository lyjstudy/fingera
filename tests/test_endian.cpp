
#include <fingera/endian.hpp>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(endian_tests)

BOOST_AUTO_TEST_CASE(readwrite) {

    using namespace fingera;

    #define IMPL_TEST_READWRITE(type, value, big_value) \
        BOOST_CHECK_EQUAL(read_little<type>(little_buffer), static_cast<type>(value)); \
        BOOST_CHECK_EQUAL(read_big<type>(big_buffer), static_cast<type>(big_value)); \
        memset(buffer, 0, sizeof(buffer)); \
        write_little<type>(buffer, static_cast<type>(value)); \
        BOOST_CHECK_EQUAL(memcmp(buffer, little_buffer, sizeof(type)), 0); \

    uint8_t little_buffer[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t big_buffer[] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
    uint8_t buffer[8];

    IMPL_TEST_READWRITE(uint8_t, 0x01, 0x08);
    IMPL_TEST_READWRITE(int8_t, 0x01, 0x08);
    IMPL_TEST_READWRITE(uint16_t, 0x0201, 0x0807);
    IMPL_TEST_READWRITE(int16_t, 0x0201, 0x0807);
    IMPL_TEST_READWRITE(uint32_t, 0x04030201, 0x08070605);
    IMPL_TEST_READWRITE(int32_t, 0x04030201, 0x08070605);
    IMPL_TEST_READWRITE(uint64_t, 0x0807060504030201L, 0x0807060504030201L);
    IMPL_TEST_READWRITE(int64_t, 0x0807060504030201L, 0x0807060504030201L);
    IMPL_TEST_READWRITE(float, 1.53998961e-36f, 4.06321607e-34f);
    IMPL_TEST_READWRITE(double, 5.447603722011605e-270, 5.447603722011605e-270);
}

BOOST_AUTO_TEST_SUITE_END()
