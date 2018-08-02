
#include <fingera/ocl/device.hpp>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(device_tests)

BOOST_AUTO_TEST_CASE(base) {
    using namespace fingera;

    ocl::dump_devices();
    ocl::dump_platforms();
}

BOOST_AUTO_TEST_SUITE_END()
