R"===(

inline uint Ch(uint x, uint y, uint z) { return z ^ (x & (y ^ z)); }
inline uint Maj(uint x, uint y, uint z) { return (x & y) | (z & (x | y)); }
inline uint Sigma0(uint x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
inline uint Sigma1(uint x) { return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); }
inline uint sigma0(uint x) { return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); }
inline uint sigma1(uint x) { return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); }

/** One round of SHA-256. */
inline void Round(uint a, uint b, uint c, uint* d, uint e, uint f, uint g, uint* h, uint k) {
    uint t1 = *h + Sigma1(e) + Ch(e, f, g) + k;
    uint t2 = Sigma0(a) + Maj(a, b, c);
    *d = *d + t1;
    *h = t1 + t2;
}

#define swap_byte(x) ( ((x) << 24) | (((x) << 8) & 0x00ff0000) | (((x) >> 8) & 0x0000ff00) | ((x) >> 24) )

__kernel void sha256_process_trunk(__global uint *chunk, __global uint *digest){
    uint a = 0x6a09e667;
    uint b = 0xbb67ae85;
    uint c = 0x3c6ef372;
    uint d = 0xa54ff53a;
    uint e = 0x510e527f;
    uint f = 0x9b05688c;
    uint g = 0x1f83d9ab;
    uint h = 0x5be0cd19;

    uint w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

    Round(a, b, c, &d, e, f, g, &h, 0x428a2f98 + (w0 = swap_byte(chunk[0])));
    Round(h, a, b, &c, d, e, f, &g, 0x71374491 + (w1 = swap_byte(chunk[1])));
    Round(g, h, a, &b, c, d, e, &f, 0xb5c0fbcf + (w2 = swap_byte(chunk[2])));
    Round(f, g, h, &a, b, c, d, &e, 0xe9b5dba5 + (w3 = swap_byte(chunk[3])));
    Round(e, f, g, &h, a, b, c, &d, 0x3956c25b + (w4 = swap_byte(chunk[4])));
    Round(d, e, f, &g, h, a, b, &c, 0x59f111f1 + (w5 = swap_byte(chunk[5])));
    Round(c, d, e, &f, g, h, a, &b, 0x923f82a4 + (w6 = swap_byte(chunk[6])));
    Round(b, c, d, &e, f, g, h, &a, 0xab1c5ed5 + (w7 = swap_byte(chunk[7])));
    Round(a, b, c, &d, e, f, g, &h, 0xd807aa98 + (w8 = swap_byte(chunk[8])));
    Round(h, a, b, &c, d, e, f, &g, 0x12835b01 + (w9 = swap_byte(chunk[9])));
    Round(g, h, a, &b, c, d, e, &f, 0x243185be + (w10 = swap_byte(chunk[10])));
    Round(f, g, h, &a, b, c, d, &e, 0x550c7dc3 + (w11 = swap_byte(chunk[11])));
    Round(e, f, g, &h, a, b, c, &d, 0x72be5d74 + (w12 = swap_byte(chunk[12])));
    Round(d, e, f, &g, h, a, b, &c, 0x80deb1fe + (w13 = swap_byte(chunk[13])));
    Round(c, d, e, &f, g, h, a, &b, 0x9bdc06a7 + (w14 = swap_byte(chunk[14])));
    Round(b, c, d, &e, f, g, h, &a, 0xc19bf174 + (w15 = swap_byte(chunk[15])));

    Round(a, b, c, &d, e, f, g, &h, 0xe49b69c1 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
    Round(h, a, b, &c, d, e, f, &g, 0xefbe4786 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
    Round(g, h, a, &b, c, d, e, &f, 0x0fc19dc6 + (w2 += sigma1(w0) + w11 + sigma0(w3)));
    Round(f, g, h, &a, b, c, d, &e, 0x240ca1cc + (w3 += sigma1(w1) + w12 + sigma0(w4)));
    Round(e, f, g, &h, a, b, c, &d, 0x2de92c6f + (w4 += sigma1(w2) + w13 + sigma0(w5)));
    Round(d, e, f, &g, h, a, b, &c, 0x4a7484aa + (w5 += sigma1(w3) + w14 + sigma0(w6)));
    Round(c, d, e, &f, g, h, a, &b, 0x5cb0a9dc + (w6 += sigma1(w4) + w15 + sigma0(w7)));
    Round(b, c, d, &e, f, g, h, &a, 0x76f988da + (w7 += sigma1(w5) + w0 + sigma0(w8)));
    Round(a, b, c, &d, e, f, g, &h, 0x983e5152 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
    Round(h, a, b, &c, d, e, f, &g, 0xa831c66d + (w9 += sigma1(w7) + w2 + sigma0(w10)));
    Round(g, h, a, &b, c, d, e, &f, 0xb00327c8 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
    Round(f, g, h, &a, b, c, d, &e, 0xbf597fc7 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
    Round(e, f, g, &h, a, b, c, &d, 0xc6e00bf3 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
    Round(d, e, f, &g, h, a, b, &c, 0xd5a79147 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
    Round(c, d, e, &f, g, h, a, &b, 0x06ca6351 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
    Round(b, c, d, &e, f, g, h, &a, 0x14292967 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

    Round(a, b, c, &d, e, f, g, &h, 0x27b70a85 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
    Round(h, a, b, &c, d, e, f, &g, 0x2e1b2138 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
    Round(g, h, a, &b, c, d, e, &f, 0x4d2c6dfc + (w2 += sigma1(w0) + w11 + sigma0(w3)));
    Round(f, g, h, &a, b, c, d, &e, 0x53380d13 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
    Round(e, f, g, &h, a, b, c, &d, 0x650a7354 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
    Round(d, e, f, &g, h, a, b, &c, 0x766a0abb + (w5 += sigma1(w3) + w14 + sigma0(w6)));
    Round(c, d, e, &f, g, h, a, &b, 0x81c2c92e + (w6 += sigma1(w4) + w15 + sigma0(w7)));
    Round(b, c, d, &e, f, g, h, &a, 0x92722c85 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
    Round(a, b, c, &d, e, f, g, &h, 0xa2bfe8a1 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
    Round(h, a, b, &c, d, e, f, &g, 0xa81a664b + (w9 += sigma1(w7) + w2 + sigma0(w10)));
    Round(g, h, a, &b, c, d, e, &f, 0xc24b8b70 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
    Round(f, g, h, &a, b, c, d, &e, 0xc76c51a3 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
    Round(e, f, g, &h, a, b, c, &d, 0xd192e819 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
    Round(d, e, f, &g, h, a, b, &c, 0xd6990624 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
    Round(c, d, e, &f, g, h, a, &b, 0xf40e3585 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
    Round(b, c, d, &e, f, g, h, &a, 0x106aa070 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

    Round(a, b, c, &d, e, f, g, &h, 0x19a4c116 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
    Round(h, a, b, &c, d, e, f, &g, 0x1e376c08 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
    Round(g, h, a, &b, c, d, e, &f, 0x2748774c + (w2 += sigma1(w0) + w11 + sigma0(w3)));
    Round(f, g, h, &a, b, c, d, &e, 0x34b0bcb5 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
    Round(e, f, g, &h, a, b, c, &d, 0x391c0cb3 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
    Round(d, e, f, &g, h, a, b, &c, 0x4ed8aa4a + (w5 += sigma1(w3) + w14 + sigma0(w6)));
    Round(c, d, e, &f, g, h, a, &b, 0x5b9cca4f + (w6 += sigma1(w4) + w15 + sigma0(w7)));
    Round(b, c, d, &e, f, g, h, &a, 0x682e6ff3 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
    Round(a, b, c, &d, e, f, g, &h, 0x748f82ee + (w8 += sigma1(w6) + w1 + sigma0(w9)));
    Round(h, a, b, &c, d, e, f, &g, 0x78a5636f + (w9 += sigma1(w7) + w2 + sigma0(w10)));
    Round(g, h, a, &b, c, d, e, &f, 0x84c87814 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
    Round(f, g, h, &a, b, c, d, &e, 0x8cc70208 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
    Round(e, f, g, &h, a, b, c, &d, 0x90befffa + (w12 += sigma1(w10) + w5 + sigma0(w13)));
    Round(d, e, f, &g, h, a, b, &c, 0xa4506ceb + (w13 += sigma1(w11) + w6 + sigma0(w14)));
    Round(c, d, e, &f, g, h, a, &b, 0xbef9a3f7 + (w14 + sigma1(w12) + w7 + sigma0(w15)));
    Round(b, c, d, &e, f, g, h, &a, 0xc67178f2 + (w15 + sigma1(w13) + w8 + sigma0(w0)));

    digest[0] = swap_byte(a + 0x6a09e667);
    digest[1] = swap_byte(b + 0xbb67ae85);
    digest[2] = swap_byte(c + 0x3c6ef372);
    digest[3] = swap_byte(d + 0xa54ff53a);
    digest[4] = swap_byte(e + 0x510e527f);
    digest[5] = swap_byte(f + 0x9b05688c);
    digest[6] = swap_byte(g + 0x1f83d9ab);
    digest[7] = swap_byte(h + 0x5be0cd19);
}

)==="