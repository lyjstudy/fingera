#include <benchmark/benchmark.h>

#include <fingera/hash/monero.hpp>

static uint8_t block_unknow[76] = {
    0x07
};

static void TEST_STANDARD(benchmark::State& state) {
    char out[32];
    for (auto _ : state) {
        fingera::hash::monero_standard(block_unknow, sizeof(block_unknow), out);
    }
}
BENCHMARK(TEST_STANDARD);

static void TEST_CPU_FAST(benchmark::State& state) {
    char out[32];
    for (auto _ : state) {
        fingera::hash::monero_cpu_fast(block_unknow, sizeof(block_unknow), out);
    }
}
BENCHMARK(TEST_CPU_FAST);

BENCHMARK_MAIN();