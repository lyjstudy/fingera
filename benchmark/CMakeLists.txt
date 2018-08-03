

add_executable( bench_multiway_sha256 bench_multiway_sha256.cpp )
target_link_libraries( bench_multiway_sha256 fingera benchmark )

add_executable( bench_ocl_demo bench_ocl_demo.cpp )
target_link_libraries( bench_ocl_demo fingera benchmark )
