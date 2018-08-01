cmake_minimum_required(VERSION 3.5)

set( CMAKE_CXX_STANDARD 14 )

option(FINGERA_USE_MMX "Enable MMX" OFF)
option(FINGERA_USE_SSE2 "Enable SSE2" OFF)
option(FINGERA_USE_AVX2 "Enable AVX2" OFF)
option(FINGERA_USE_AVX512F "Enable AVX512F" OFF)
set(FINGERA_AUTO_DETECT true)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (${FINGERA_AUTO_DETECT})
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/fingera/config.hpp.in 
    ${CMAKE_CURRENT_BINARY_DIR}/include/fingera/config.hpp)

find_package(Boost REQUIRED COMPONENTS
    thread
    unit_test_framework
)

add_library(fingera 
    src/cpu_features.cpp
)

target_link_libraries(fingera pthread)
target_include_directories(fingera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(tests)
add_subdirectory(benchmark)