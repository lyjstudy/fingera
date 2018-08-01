

add_executable( bench_multiway_sha256 bench_multiway_sha256.cpp )
target_link_libraries( bench_multiway_sha256 fingera benchmark )
target_compile_options( bench_multiway_sha256 PRIVATE "-O3" )
