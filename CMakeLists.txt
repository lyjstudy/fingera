cmake_minimum_required(VERSION 3.5)

set( CMAKE_CXX_STANDARD 14 )

option(FINGERA_AUTO_INSTRINSIC "Auto enable cpu feature" ON)
option(FINGERA_USE_MMX "Enable MMX" OFF)
option(FINGERA_USE_SSE2 "Enable SSE2" OFF)
option(FINGERA_USE_AVX2 "Enable AVX2" OFF)
option(FINGERA_USE_AVX512F "Enable AVX512F" OFF)

option(FINGERA_ENABLE_BENCHMARK "Benchmark" ON)
option(FINGERA_ENABLE_UNIT_TESTS "Unit tests" ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(cotire)

if (${FINGERA_AUTO_INSTRINSIC} STREQUAL "ON")
    message("Detecting cpu features ...")
    include(DetectCPUFeatures)
    detect_cpu_features()
endif()

if (${FINGERA_USE_AVX2} STREQUAL "ON")
    add_compile_options("-mavx2")
endif()
if (${FINGERA_USE_AVX512F} STREQUAL "ON")
    add_compile_options("-mavx512f")
endif()
if (${FINGERA_ENABLE_BENCHMARK} STREQUAL "ON")
    add_compile_options("-Ofast" "-DNDEBUG")
endif()

if (${FINGERA_USE_AES} STREQUAL "ON")
    add_compile_options("-maes")
endif()

add_compile_options("-mbmi2")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/fingera/config.hpp.in 
    ${CMAKE_CURRENT_BINARY_DIR}/include/fingera/config.hpp)

set(BOOST_ROOT ~/opt/boost)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS ON)
find_package(Boost 1.67 REQUIRED COMPONENTS
    thread
    system
    regex
    date_time
    serialization
    unit_test_framework
)
include_directories(${Boost_INCLUDE_DIRS})

add_library(fingera 
    src/cpu_features.cpp
    src/stratum/client.cpp
    
    src/hash/monero.cpp
# monero
    src/hash/monero/blake256.c
    src/hash/monero/groestl.c
    src/hash/monero/jh.c
    src/hash/monero/keccak.c
    src/hash/monero/skein.c
    src/hash/monero/oaes_lib.c
    src/hash/monero/slow-hash.c
    src/hash/monero/aesb.c
    src/hash/monero/hash.c
    src/hash/monero/hash-extra-blake.c
    src/hash/monero/hash-extra-groestl.c
    src/hash/monero/hash-extra-jh.c
    src/hash/monero/hash-extra-skein.c
    
#    src/ocl/device.cpp
)

target_link_libraries(fingera pthread OpenCL ${Boost_LIBRARIES})
target_include_directories(fingera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)
cotire(fingera)

if (${FINGERA_ENABLE_UNIT_TESTS} STREQUAL "ON")
    add_subdirectory(tests)
endif()

if (${FINGERA_ENABLE_BENCHMARK} STREQUAL "ON")
    add_subdirectory(benchmark)
endif()
