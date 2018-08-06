

file(GLOB UNIT_TESTS "*.cpp" "**/*.cpp")

add_executable( unit_test ${UNIT_TESTS} )
target_link_libraries( unit_test fingera )