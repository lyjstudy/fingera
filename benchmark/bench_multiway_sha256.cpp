#include <benchmark/benchmark.h>
#include <fingera/config.hpp>
#include <fingera/hash/multiway_sha256.hpp>
#include <fingera/multiway_integer.hpp>
#include <fingera/instrinsic/mi_sse2.hpp>
#include <fingera/instrinsic/mi_avx2.hpp>
#include <fingera/instrinsic/mi_mmx.hpp>

using namespace fingera;

static void SHA256_1_way_generic(benchmark::State& state) {
    uint8_t blocks[64];
    uint8_t result[32];
    for (auto _ : state) {
        hash::multiway_sha256<multiway_integer<uint32_t, uint32_t>>::process_trunk(result, blocks);
    }
}
BENCHMARK(SHA256_1_way_generic);

static void SHA256_2_way_generic(benchmark::State& state) {
    uint8_t blocks[64 * 2];
    uint8_t result[32 * 2];
    for (auto _ : state) {
        hash::multiway_sha256<multiway_integer<uint32_t, uint64_t>>::process_trunk(result, blocks);
    }
}
BENCHMARK(SHA256_2_way_generic);

#if defined(FINGERA_USE_MMX)
static void SHA256_2_way_mmx(benchmark::State& state) {
    uint8_t blocks[64 * 2];
    uint8_t result[32 * 2];
    for (auto _ : state) {
        hash::multiway_sha256<instrinsic::mi_mmx>::process_trunk(result, blocks);
    }
}
BENCHMARK(SHA256_2_way_mmx);
#endif

#if defined(FINGERA_USE_SSE2)
static void SHA256_4_way_sse2(benchmark::State& state) {
    uint8_t blocks[64 * 4];
    uint8_t result[32 * 4];
    for (auto _ : state) {
        hash::multiway_sha256<instrinsic::mi_sse2>::process_trunk(result, blocks);
    }
}
BENCHMARK(SHA256_4_way_sse2);
#endif

#if defined(FINGERA_USE_AVX2)
static void SHA256_8_way_avx2(benchmark::State& state) {
    uint8_t blocks[64 * 8];
    uint8_t result[32 * 8];
    for (auto _ : state) {
        hash::multiway_sha256<instrinsic::mi_avx2>::process_trunk(result, blocks);
    }
}
BENCHMARK(SHA256_8_way_avx2);
#endif

BENCHMARK_MAIN();