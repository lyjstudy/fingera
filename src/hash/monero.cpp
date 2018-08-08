#include <cassert>
#include <cstdint>
#include <iostream>
#include <fingera/hex.hpp>
#include <fingera/hash/monero.hpp>
#include <fingera/config.hpp>
extern "C" {
#include "monero/hash-ops.h"
#include "monero/keccak.h"
}
#include <immintrin.h>

static void (*const extra_hashes[4])(const void *, size_t, char *) = {
    hash_extra_blake, hash_extra_groestl, hash_extra_jh, hash_extra_skein
};

namespace fingera {
namespace hash {
    
void monero_standard(const void *data, size_t length, void *result) {
    assert(length != 0);
    uint8_t major_version = *(const uint8_t *)data;
    assert(major_version <= 127); // now 7

    const int cn_variant = major_version >= 7 ? major_version - 6 : 0;
    cn_slow_hash(data, length, (char *)result, cn_variant, 0);
}

// This will shift and xor tmp1 into itself as 4 32-bit vals such as
// sl_xor(a1 a2 a3 a4) = a1 (a2^a1) (a3^a2^a1) (a4^a3^a2^a1)
inline __m128i sl_xor(__m128i tmp1) {
    __m128i tmp4;
    tmp4 = _mm_slli_si128(tmp1, 0x04);
    tmp1 = _mm_xor_si128(tmp1, tmp4);
    tmp4 = _mm_slli_si128(tmp4, 0x04);
    tmp1 = _mm_xor_si128(tmp1, tmp4);
    tmp4 = _mm_slli_si128(tmp4, 0x04);
    tmp1 = _mm_xor_si128(tmp1, tmp4);
    return tmp1;
}

template<uint8_t rcon>
inline void aes_genkey_sub(__m128i* xout0, __m128i* xout2) {
    __m128i xout1 = _mm_aeskeygenassist_si128(*xout2, rcon);
    xout1  = _mm_shuffle_epi32(xout1, 0xFF); // see PSHUFD, set all elems to 4th elem
    *xout0 = sl_xor(*xout0);
    *xout0 = _mm_xor_si128(*xout0, xout1);
    xout1  = _mm_aeskeygenassist_si128(*xout0, 0x00);
    xout1  = _mm_shuffle_epi32(xout1, 0xAA); // see PSHUFD, set all elems to 3rd elem
    *xout2 = sl_xor(*xout2);
    *xout2 = _mm_xor_si128(*xout2, xout1);
}

inline void aes_expand_key(const __m128i* memory, __m128i* k0, __m128i* k1, __m128i* k2, __m128i* k3, __m128i* k4, __m128i* k5, __m128i* k6, __m128i* k7, __m128i* k8, __m128i* k9) {
    __m128i xout0 = _mm_load_si128(memory);
    __m128i xout2 = _mm_load_si128(memory + 1);
    *k0 = xout0;
    *k1 = xout2;

    aes_genkey_sub<0x01>(&xout0, &xout2);
    *k2 = xout0;
    *k3 = xout2;

    aes_genkey_sub<0x02>(&xout0, &xout2);
    *k4 = xout0;
    *k5 = xout2;

    aes_genkey_sub<0x04>(&xout0, &xout2);
    *k6 = xout0;
    *k7 = xout2;

    aes_genkey_sub<0x08>(&xout0, &xout2);
    *k8 = xout0;
    *k9 = xout2;
}

inline void aes_round(__m128i key, __m128i* x0, __m128i* x1, __m128i* x2, __m128i* x3, __m128i* x4, __m128i* x5, __m128i* x6, __m128i* x7) {
    *x0 = _mm_aesenc_si128(*x0, key);
    *x1 = _mm_aesenc_si128(*x1, key);
    *x2 = _mm_aesenc_si128(*x2, key);
    *x3 = _mm_aesenc_si128(*x3, key);
    *x4 = _mm_aesenc_si128(*x4, key);
    *x5 = _mm_aesenc_si128(*x5, key);
    *x6 = _mm_aesenc_si128(*x6, key);
    *x7 = _mm_aesenc_si128(*x7, key);
}

#define MEMORY         (1 << 21) // 2MB scratchpad
#define ITERATIONS      (1 << 19)
inline void cn_explode_scratchpad(const __m128i *input, __m128i *output) {
    __m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;
    aes_expand_key(input, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

    __m128i xin0, xin1, xin2, xin3, xin4, xin5, xin6, xin7;
    xin0 = _mm_load_si128(input + 4);
    xin1 = _mm_load_si128(input + 5);
    xin2 = _mm_load_si128(input + 6);
    xin3 = _mm_load_si128(input + 7);
    xin4 = _mm_load_si128(input + 8);
    xin5 = _mm_load_si128(input + 9);
    xin6 = _mm_load_si128(input + 10);
    xin7 = _mm_load_si128(input + 11);

    for (size_t i = 0; i < MEMORY / 16; i += 8) {
        aes_round(k0, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k1, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k2, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k3, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k4, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k5, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k6, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k7, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k8, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
        aes_round(k9, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);

        _mm_store_si128(output + i + 0, xin0);
        _mm_store_si128(output + i + 1, xin1);
        _mm_store_si128(output + i + 2, xin2);
        _mm_store_si128(output + i + 3, xin3);
        _mm_store_si128(output + i + 4, xin4);
        _mm_store_si128(output + i + 5, xin5);
        _mm_store_si128(output + i + 6, xin6);
        _mm_store_si128(output + i + 7, xin7);
    }
}

static inline void cn_implode_scratchpad(const __m128i *input, __m128i *output)
{
    __m128i xout0, xout1, xout2, xout3, xout4, xout5, xout6, xout7;
    __m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;

    aes_expand_key(output + 2, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

    xout0 = _mm_load_si128(output + 4);
    xout1 = _mm_load_si128(output + 5);
    xout2 = _mm_load_si128(output + 6);
    xout3 = _mm_load_si128(output + 7);
    xout4 = _mm_load_si128(output + 8);
    xout5 = _mm_load_si128(output + 9);
    xout6 = _mm_load_si128(output + 10);
    xout7 = _mm_load_si128(output + 11);

    for (size_t i = 0; i < MEMORY / 16; i += 8)
    {
        xout0 = _mm_xor_si128(_mm_load_si128(input + i + 0), xout0);
        xout1 = _mm_xor_si128(_mm_load_si128(input + i + 1), xout1);
        xout2 = _mm_xor_si128(_mm_load_si128(input + i + 2), xout2);
        xout3 = _mm_xor_si128(_mm_load_si128(input + i + 3), xout3);
        xout4 = _mm_xor_si128(_mm_load_si128(input + i + 4), xout4);
        xout5 = _mm_xor_si128(_mm_load_si128(input + i + 5), xout5);
        xout6 = _mm_xor_si128(_mm_load_si128(input + i + 6), xout6);
        xout7 = _mm_xor_si128(_mm_load_si128(input + i + 7), xout7);

        aes_round(k0, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k1, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k2, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k3, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k4, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k5, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k6, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k7, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k8, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
        aes_round(k9, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6, &xout7);
    }

    _mm_store_si128(output + 4, xout0);
    _mm_store_si128(output + 5, xout1);
    _mm_store_si128(output + 6, xout2);
    _mm_store_si128(output + 7, xout3);
    _mm_store_si128(output + 8, xout4);
    _mm_store_si128(output + 9, xout5);
    _mm_store_si128(output + 10, xout6);
    _mm_store_si128(output + 11, xout7);
}

void monero_cpu_fast(const void *block_blob, size_t length, void *result) {
    // major_version 1-2 current 1
    // minor_version 1-2 current 1
    // timestamp 1-10 current min 5()
    // prev_id 32
    // nonce 4
    // tree_root_hash 32
    // (tx_hashes.size()+1) 1-10
    // 0-126 tx: 76
    // 127-254 tx: 77
    // accept: 76->80
    assert(length >= 76 && length <= 80);
    assert(*(const uint8_t *)block_blob == 7); // accept: major_version = 7

    alignas(16) uint8_t keccak_state[200];
    alignas(256) uint8_t memory[MEMORY];

    /* CryptoNight Step 1:  Use Keccak1600 to initialize the 'state' (and 'text') buffers from the data. */
    keccak1600((const uint8_t *)block_blob, length, keccak_state);

    uint64_t tweak1_2_0 = ((uint64_t *)keccak_state)[24] ^ *((uint64_t *)((char *)block_blob + 35));

    cn_explode_scratchpad((__m128i*)keccak_state, (__m128i*)memory);

    const uint8_t* l0 = memory;
    uint64_t* h0 = reinterpret_cast<uint64_t*>(keccak_state);

    uint64_t al0 = h0[0] ^ h0[4];
    uint64_t ah0 = h0[1] ^ h0[5];
    __m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);

/*
    FILE *fp = fopen("/tmp/mmmmmm_cpu_fast", "wb");
    fwrite(memory, MEMORY, 1, fp);
    fclose(fp);
*/
    // std::cout << al0 << ", " << ah0 << ", " << (h0[2] ^ h0[6]) << ", " << (h0[3] ^ h0[7]) << ", " << tweak1_2_0 << std::endl;

    uint64_t idx0 = al0;
    for (size_t i = 0; i < ITERATIONS; i++) {
        const void *m = &l0[idx0 & 0x1FFFF0];
        // _mm_prefetch(m, _MM_HINT_NTA);
        __m128i cx = _mm_load_si128((__m128i *) m);
        cx = _mm_aesenc_si128(cx, _mm_set_epi64x(ah0, al0));

        __m128i tmp = _mm_xor_si128(bx0, cx);
        ((uint64_t *)m)[0] = _mm_cvtsi128_si64(tmp);

        tmp = _mm_castps_si128(_mm_movehl_ps(_mm_castsi128_ps(tmp), _mm_castsi128_ps(tmp)));
        uint64_t vh = _mm_cvtsi128_si64(tmp);
        uint8_t x = vh >> 24;
        static const uint16_t table = 0x7531;
        const uint8_t index = (((x >> 3) & 6) | (x & 1)) << 1;
        vh ^= ((table >> index) & 0x3) << 28;
        ((uint64_t *)m)[1] = vh;

        idx0 = _mm_cvtsi128_si64(cx);
        m = &l0[idx0 & 0x1FFFF0];
        // _mm_prefetch(m, _MM_HINT_NTA);
        bx0 = cx;

        uint64_t hi, lo, cl, ch;
        cl = ((uint64_t*) m)[0];
        ch = ((uint64_t*) m)[1];
        lo = _mulx_u64(idx0, cl, (unsigned long long *)&hi);

        al0 += hi;
        ah0 += lo;

        ((uint64_t*) m)[0] = al0;
        ((uint64_t*) m)[1] = ah0 ^ tweak1_2_0;

        al0 ^= cl;
        ah0 ^= ch;
        idx0 = al0;
/*
        if (i == 0) {
    FILE *fp = fopen("/tmp/mmmmmm_cpu_fast", "wb");
    fwrite(memory, MEMORY, 1, fp);
    fclose(fp);
            std::cout << al0 << ", " << ah0 << std::endl;
        }
*/
    }

    cn_implode_scratchpad((__m128i*) memory, (__m128i*) h0);

/*
    for (size_t i = 0; i < 25; i++) {
        std::cout << h0[i] << ", ";
        if (i && (i % 5) == 0) std::cout << std::endl;
    }
    std::cout << std::endl;
*/
    keccakf(h0, 24);
    extra_hashes[h0[0] & 3](h0, 200, (char *)result);
}

} // namespace hash
} // namespace fingera