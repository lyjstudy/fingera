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

void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks);
static void SHA256_1_way_cpp(benchmark::State& state) {
    uint8_t blocks[64];
    uint8_t s[64];
    for (auto _ : state) {
        Transform((uint32_t *)s, blocks, 1);
    }
}
BENCHMARK(SHA256_1_way_cpp);


/////////////////////////////////////
uint32_t inline Ch(uint32_t x, uint32_t y, uint32_t z) { return z ^ (x & (y ^ z)); }
uint32_t inline Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (z & (x | y)); }
uint32_t inline Sigma0(uint32_t x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
uint32_t inline Sigma1(uint32_t x) { return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); }
uint32_t inline sigma0(uint32_t x) { return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); }
uint32_t inline sigma1(uint32_t x) { return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); }

/** One round of SHA-256. */
void inline Round(uint32_t a, uint32_t b, uint32_t c, uint32_t& d, uint32_t e, uint32_t f, uint32_t g, uint32_t& h, uint32_t k)
{
    uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + k;
    uint32_t t2 = Sigma0(a) + Maj(a, b, c);
    d += t1;
    h = t1 + t2;
}

/** Perform a number of SHA-256 transformations, processing 64-byte chunks. */
void Transform(uint32_t* s, const unsigned char* chunk, size_t blocks)
{
    while (blocks--) {
        uint32_t a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
        uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

        Round(a, b, c, d, e, f, g, h, 0x428a2f98 + (w0 = read_big<uint32_t>(chunk + 0)));
        Round(h, a, b, c, d, e, f, g, 0x71374491 + (w1 = read_big<uint32_t>(chunk + 4)));
        Round(g, h, a, b, c, d, e, f, 0xb5c0fbcf + (w2 = read_big<uint32_t>(chunk + 8)));
        Round(f, g, h, a, b, c, d, e, 0xe9b5dba5 + (w3 = read_big<uint32_t>(chunk + 12)));
        Round(e, f, g, h, a, b, c, d, 0x3956c25b + (w4 = read_big<uint32_t>(chunk + 16)));
        Round(d, e, f, g, h, a, b, c, 0x59f111f1 + (w5 = read_big<uint32_t>(chunk + 20)));
        Round(c, d, e, f, g, h, a, b, 0x923f82a4 + (w6 = read_big<uint32_t>(chunk + 24)));
        Round(b, c, d, e, f, g, h, a, 0xab1c5ed5 + (w7 = read_big<uint32_t>(chunk + 28)));
        Round(a, b, c, d, e, f, g, h, 0xd807aa98 + (w8 = read_big<uint32_t>(chunk + 32)));
        Round(h, a, b, c, d, e, f, g, 0x12835b01 + (w9 = read_big<uint32_t>(chunk + 36)));
        Round(g, h, a, b, c, d, e, f, 0x243185be + (w10 = read_big<uint32_t>(chunk + 40)));
        Round(f, g, h, a, b, c, d, e, 0x550c7dc3 + (w11 = read_big<uint32_t>(chunk + 44)));
        Round(e, f, g, h, a, b, c, d, 0x72be5d74 + (w12 = read_big<uint32_t>(chunk + 48)));
        Round(d, e, f, g, h, a, b, c, 0x80deb1fe + (w13 = read_big<uint32_t>(chunk + 52)));
        Round(c, d, e, f, g, h, a, b, 0x9bdc06a7 + (w14 = read_big<uint32_t>(chunk + 56)));
        Round(b, c, d, e, f, g, h, a, 0xc19bf174 + (w15 = read_big<uint32_t>(chunk + 60)));

        Round(a, b, c, d, e, f, g, h, 0xe49b69c1 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, 0xefbe4786 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, 0x0fc19dc6 + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, 0x240ca1cc + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, 0x2de92c6f + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, 0x4a7484aa + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, 0x5cb0a9dc + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, 0x76f988da + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, 0x983e5152 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, 0xa831c66d + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, 0xb00327c8 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, 0xbf597fc7 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, 0xc6e00bf3 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, 0xd5a79147 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, 0x06ca6351 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, 0x14292967 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, d, e, f, g, h, 0x27b70a85 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, 0x2e1b2138 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, 0x4d2c6dfc + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, 0x53380d13 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, 0x650a7354 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, 0x766a0abb + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, 0x81c2c92e + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, 0x92722c85 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, 0xa2bfe8a1 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, 0xa81a664b + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, 0xc24b8b70 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, 0xc76c51a3 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, 0xd192e819 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, 0xd6990624 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, 0xf40e3585 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, 0x106aa070 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, d, e, f, g, h, 0x19a4c116 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, c, d, e, f, g, 0x1e376c08 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, b, c, d, e, f, 0x2748774c + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, a, b, c, d, e, 0x34b0bcb5 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, h, a, b, c, d, 0x391c0cb3 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, g, h, a, b, c, 0x4ed8aa4a + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, f, g, h, a, b, 0x5b9cca4f + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, e, f, g, h, a, 0x682e6ff3 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, d, e, f, g, h, 0x748f82ee + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, c, d, e, f, g, 0x78a5636f + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, b, c, d, e, f, 0x84c87814 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, a, b, c, d, e, 0x8cc70208 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, h, a, b, c, d, 0x90befffa + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, g, h, a, b, c, 0xa4506ceb + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, f, g, h, a, b, 0xbef9a3f7 + (w14 + sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, e, f, g, h, a, 0xc67178f2 + (w15 + sigma1(w13) + w8 + sigma0(w0)));

        s[0] += a;
        s[1] += b;
        s[2] += c;
        s[3] += d;
        s[4] += e;
        s[5] += f;
        s[6] += g;
        s[7] += h;
        chunk += 64;
    }
}

BENCHMARK_MAIN();