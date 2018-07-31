cmake_minimum_required(VERSION 3.5)

set( CMAKE_CXX_STANDARD 14 )

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(DetectCPUFeatures)

detect_cpu_features()

message("FINGERA_HAS_SSE2: " ${FINGERA_HAS_SSE2})
message("FINGERA_HAS_AVX2: " ${FINGERA_HAS_AVX2})
message("FINGERA_HAS_AVX512F: " ${FINGERA_HAS_AVX512F})

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS OFF)
set(BOOST_ALL_DYN_LINK ON)
add_definitions(-DBOOST_ALL_DYN_LINK=1)

find_package(Boost REQUIRED COMPONENTS
	thread
    unit_test_framework
)

add_compile_options("-mavx2")

add_library(fingera 
    src/cpu_features.cpp
)

target_link_libraries(fingera)
target_include_directories(fingera PUBLIC include)

add_subdirectory(tests)