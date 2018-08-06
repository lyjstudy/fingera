#include <benchmark/benchmark.h>

#include <boost/compute/buffer.hpp>
#include <boost/compute/platform.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/utility/source.hpp>
#include <boost/compute/utility/dim.hpp>

const char *scaler_func = BOOST_COMPUTE_STRINGIZE_SOURCE(
    inline uint Sigma0(uint x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
    __kernel void test_entry(__global uint *value, __global uint *output) {
        uint gid = get_global_id(0);
        output[gid] = Sigma0(value[gid]);
    }
);
const char *vector_func = BOOST_COMPUTE_STRINGIZE_SOURCE(
    inline uint16 Sigma0(uint16 x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
    __kernel void test_entry(__global uint *value, __global uint *output) {
        uint gid = get_global_id(0);
        vstore16(Sigma0(vload16(gid * 16, value)), gid * 16, output);
    }
);

static void TEST_OCL_VECTOR(benchmark::State& state) {
    namespace bc = boost::compute;
    using bcc = bc::device;
    using bcs = bc::system;

    auto queue = bcs::default_queue();
    auto context = bcs::default_context();
    auto kernel = bc::kernel::create_with_source(vector_func, "test_entry", context);

    constexpr size_t thread_count = 1024;
    bc::buffer chunk(context, thread_count * 16 * 4, bc::memory_object::mem_flags::read_only);
    bc::buffer digest(context, thread_count * 16 * 4, bc::memory_object::mem_flags::write_only);
    kernel.set_arg(0, chunk);
    kernel.set_arg(1, digest);
    uint8_t blocks[thread_count * 16 * 4];

    for (auto _ : state) {
        queue.enqueue_write_buffer(chunk, 0, sizeof(blocks), blocks);
        queue.enqueue_1d_range_kernel(kernel, (size_t)0, thread_count, (size_t)1);
        queue.finish();
    }
}
BENCHMARK(TEST_OCL_VECTOR);

static void TEST_OCL_SCALER(benchmark::State& state) {
    namespace bc = boost::compute;
    using bcc = bc::device;
    using bcs = bc::system;

    auto queue = bcs::default_queue();
    auto context = bcs::default_context();
    auto kernel = bc::kernel::create_with_source(scaler_func, "test_entry", context);

    constexpr size_t thread_count = 1024;
    bc::buffer chunk(context, thread_count * 4, bc::memory_object::mem_flags::read_only);
    bc::buffer digest(context, thread_count * 4, bc::memory_object::mem_flags::write_only);
    kernel.set_arg(0, chunk);
    kernel.set_arg(1, digest);
    uint8_t blocks[thread_count * 4];

    for (auto _ : state) {
        queue.enqueue_write_buffer(chunk, 0, sizeof(blocks), blocks);
        queue.enqueue_1d_range_kernel(kernel, (size_t)0, thread_count, (size_t)1);
        queue.finish();
    }
}
BENCHMARK(TEST_OCL_SCALER);


BENCHMARK_MAIN();