#pragma once

#include <cstdint>
#include <fingera/config.hpp>

namespace fingera {
namespace hash {

template<typename Instr>
class multiway_sha256 {
public:
    using type = typename Instr::type;
protected:
    static FINGERA_FORCEINLINE type _broadcast(uint32_t x) {
        return Instr::op_broadcast(x);
    }

    static FINGERA_FORCEINLINE type _add(type x) {
        return x;
    }
    template<typename ...Args>
    static FINGERA_FORCEINLINE type _add(type x, Args... rest) {
        return Instr::op_add(x, _add(rest...));
    }
    template<typename ...Args>
    static FINGERA_FORCEINLINE type _inc(type &x, Args... rest) {
        x = _add(x, rest...);
        return x;
    }

    static FINGERA_FORCEINLINE type _xor(type x, type y) {
        return Instr::op_xor(x, y);
    }
    static FINGERA_FORCEINLINE type _xor(type x, type y, type z) {
        return _xor(_xor(x, y), z);
    }

    static FINGERA_FORCEINLINE type _or(type x, type y) {
        return Instr::op_or(x, y);
    }
    static FINGERA_FORCEINLINE type _and(type x, type y) {
        return Instr::op_and(x, y);
    }

    template<int N>
    static FINGERA_FORCEINLINE type _shr(type x) {
        return Instr::template op_shr<N>(x);
    }
    template<int N>
    static FINGERA_FORCEINLINE type _shl(type x) {
        return Instr::template op_shl<N>(x);
    }
    template<int N>
    static FINGERA_FORCEINLINE type _rol(type x) {
        return Instr::template op_rol<N>(x);
    }
protected:
    static FINGERA_FORCEINLINE type Ch(type x, type y, type z) {
        // z ^ (x & (y ^ z))
        return _xor(z, _and(x, _xor(y, z)));
    }
    static FINGERA_FORCEINLINE type Maj(type x, type y, type z) {
        // (x & y) | (z & (x | y))
        return _or(_and(x, y), _and(z, _or(x, y)));
    }
    static FINGERA_FORCEINLINE type Sigma0(type x) {
        // (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10)
        return _xor(_rol<30>(x), _rol<19>(x), _rol<10>(x));
    }
    static FINGERA_FORCEINLINE type Sigma1(type x) {
        // (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7);
        return _xor(_rol<26>(x), _rol<21>(x), _rol<7>(x));
    }
    static FINGERA_FORCEINLINE type sigma0(type x) {
        // (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3)
        return _xor(_rol<25>(x), _rol<14>(x), _shr<3>(x));
    }
    static FINGERA_FORCEINLINE type sigma1(type x) {
        // (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10)
        return _xor(_rol<15>(x), _rol<13>(x), _shr<10>(x));
    }

    static FINGERA_FORCEINLINE void round(type a, type b, type c, type& d, type e, type f, type g, type& h, type k) {
        /*
            uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + k;
            uint32_t t2 = Sigma0(a) + Maj(a, b, c);
            d += t1;
            h = t1 + t2;
        */
        type t1 = _add(h, Sigma1(e), Ch(e, f, g), k);
        type t2 = _add(Sigma0(a), Maj(a, b, c));
        d = _add(d, t1);
        h = _add(t1, t2);
    }
public:
    static FINGERA_FORCEINLINE void process_block(
            type &a, type &b, type &c, type &d, 
            type &e, type &f, type &g, type &h,
            const void *block) {

        type w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

        type oa = a;
        type ob = b;
        type oc = c;
        type od = d;
        type oe = e;
        type of = f;
        type og = g;
        type oh = h;

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0x428a2f98ul), w0 = Instr::template load<false>(block, 64, 0)));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0x71374491ul), w1 = Instr::template load<false>(block, 64, 4)));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0xb5c0fbcful), w2 = Instr::template load<false>(block, 64, 8)));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0xe9b5dba5ul), w3 = Instr::template load<false>(block, 64, 12)));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x3956c25bul), w4 = Instr::template load<false>(block, 64, 16)));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0x59f111f1ul), w5 = Instr::template load<false>(block, 64, 20)));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x923f82a4ul), w6 = Instr::template load<false>(block, 64, 24)));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0xab1c5ed5ul), w7 = Instr::template load<false>(block, 64, 28)));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0xd807aa98ul), w8 = Instr::template load<false>(block, 64, 32)));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0x12835b01ul), w9 = Instr::template load<false>(block, 64, 36)));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0x243185beul), w10 = Instr::template load<false>(block, 64, 40)));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0x550c7dc3ul), w11 = Instr::template load<false>(block, 64, 44)));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x72be5d74ul), w12 = Instr::template load<false>(block, 64, 48)));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0x80deb1feul), w13 = Instr::template load<false>(block, 64, 52)));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x9bdc06a7ul), w14 = Instr::template load<false>(block, 64, 56)));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0xc19bf174ul), w15 = Instr::template load<false>(block, 64, 60)));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0xe49b69c1ul), _inc(w0, sigma1(w14), w9, sigma0(w1))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0xefbe4786ul), _inc(w1, sigma1(w15), w10, sigma0(w2))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0x0fc19dc6ul), _inc(w2, sigma1(w0), w11, sigma0(w3))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0x240ca1ccul), _inc(w3, sigma1(w1), w12, sigma0(w4))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x2de92c6ful), _inc(w4, sigma1(w2), w13, sigma0(w5))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0x4a7484aaul), _inc(w5, sigma1(w3), w14, sigma0(w6))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x5cb0a9dcul), _inc(w6, sigma1(w4), w15, sigma0(w7))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0x76f988daul), _inc(w7, sigma1(w5), w0, sigma0(w8))));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0x983e5152ul), _inc(w8, sigma1(w6), w1, sigma0(w9))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0xa831c66dul), _inc(w9, sigma1(w7), w2, sigma0(w10))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0xb00327c8ul), _inc(w10, sigma1(w8), w3, sigma0(w11))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0xbf597fc7ul), _inc(w11, sigma1(w9), w4, sigma0(w12))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0xc6e00bf3ul), _inc(w12, sigma1(w10), w5, sigma0(w13))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0xd5a79147ul), _inc(w13, sigma1(w11), w6, sigma0(w14))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x06ca6351ul), _inc(w14, sigma1(w12), w7, sigma0(w15))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0x14292967ul), _inc(w15, sigma1(w13), w8, sigma0(w0))));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0x27b70a85ul), _inc(w0, sigma1(w14), w9, sigma0(w1))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0x2e1b2138ul), _inc(w1, sigma1(w15), w10, sigma0(w2))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0x4d2c6dfcul), _inc(w2, sigma1(w0), w11, sigma0(w3))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0x53380d13ul), _inc(w3, sigma1(w1), w12, sigma0(w4))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x650a7354ul), _inc(w4, sigma1(w2), w13, sigma0(w5))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0x766a0abbul), _inc(w5, sigma1(w3), w14, sigma0(w6))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x81c2c92eul), _inc(w6, sigma1(w4), w15, sigma0(w7))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0x92722c85ul), _inc(w7, sigma1(w5), w0, sigma0(w8))));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0xa2bfe8a1ul), _inc(w8, sigma1(w6), w1, sigma0(w9))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0xa81a664bul), _inc(w9, sigma1(w7), w2, sigma0(w10))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0xc24b8b70ul), _inc(w10, sigma1(w8), w3, sigma0(w11))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0xc76c51a3ul), _inc(w11, sigma1(w9), w4, sigma0(w12))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0xd192e819ul), _inc(w12, sigma1(w10), w5, sigma0(w13))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0xd6990624ul), _inc(w13, sigma1(w11), w6, sigma0(w14))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0xf40e3585ul), _inc(w14, sigma1(w12), w7, sigma0(w15))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0x106aa070ul), _inc(w15, sigma1(w13), w8, sigma0(w0))));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0x19a4c116ul), _inc(w0, sigma1(w14), w9, sigma0(w1))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0x1e376c08ul), _inc(w1, sigma1(w15), w10, sigma0(w2))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0x2748774cul), _inc(w2, sigma1(w0), w11, sigma0(w3))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0x34b0bcb5ul), _inc(w3, sigma1(w1), w12, sigma0(w4))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x391c0cb3ul), _inc(w4, sigma1(w2), w13, sigma0(w5))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0x4ed8aa4aul), _inc(w5, sigma1(w3), w14, sigma0(w6))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0x5b9cca4ful), _inc(w6, sigma1(w4), w15, sigma0(w7))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0x682e6ff3ul), _inc(w7, sigma1(w5), w0, sigma0(w8))));

        round(a, b, c, d, e, f, g, h, _add(_broadcast(0x748f82eeul), _inc(w8, sigma1(w6), w1, sigma0(w9))));
        round(h, a, b, c, d, e, f, g, _add(_broadcast(0x78a5636ful), _inc(w9, sigma1(w7), w2, sigma0(w10))));
        round(g, h, a, b, c, d, e, f, _add(_broadcast(0x84c87814ul), _inc(w10, sigma1(w8), w3, sigma0(w11))));
        round(f, g, h, a, b, c, d, e, _add(_broadcast(0x8cc70208ul), _inc(w11, sigma1(w9), w4, sigma0(w12))));

        round(e, f, g, h, a, b, c, d, _add(_broadcast(0x90befffaul), _inc(w12, sigma1(w10), w5, sigma0(w13))));
        round(d, e, f, g, h, a, b, c, _add(_broadcast(0xa4506cebul), _inc(w13, sigma1(w11), w6, sigma0(w14))));
        round(c, d, e, f, g, h, a, b, _add(_broadcast(0xbef9a3f7ul), _inc(w14, sigma1(w12), w7, sigma0(w15))));
        round(b, c, d, e, f, g, h, a, _add(_broadcast(0xc67178f2ul), _inc(w15, sigma1(w13), w8, sigma0(w0))));

        a = _add(a, oa);
        b = _add(b, ob);
        c = _add(c, oc);
        d = _add(d, od);
        e = _add(e, oe);
        f = _add(f, of);
        g = _add(g, og);
        h = _add(h, oh);
    }

    static FINGERA_NOINLINE void process_trunk(void *out, const void *blocks, int count = 1) {
        type a = _broadcast(0x6a09e667ul);
        type b = _broadcast(0xbb67ae85ul);
        type c = _broadcast(0x3c6ef372ul);
        type d = _broadcast(0xa54ff53aul);
        type e = _broadcast(0x510e527ful);
        type f = _broadcast(0x9b05688cul);
        type g = _broadcast(0x1f83d9abul);
        type h = _broadcast(0x5be0cd19ul);

        char *cur_block = (char *)blocks;
        while (count--) {
            process_block(a, b, c, d, e, f, g, h, cur_block);
            cur_block += 64 * Instr::way();
        }

        Instr::template save<false>(a, out, 32,  0);
        Instr::template save<false>(b, out, 32,  4);
        Instr::template save<false>(c, out, 32,  8);
        Instr::template save<false>(d, out, 32, 12);
        Instr::template save<false>(e, out, 32, 16);
        Instr::template save<false>(f, out, 32, 20);
        Instr::template save<false>(g, out, 32, 24);
        Instr::template save<false>(h, out, 32, 28);
    }
};

} // namespace hash
} // namespace fingera
