#include <cassert>
#include <fingera/cpu_features.hpp>

namespace fingera {

static bool x86_cpuid(unsigned v, unsigned *ax, unsigned *bx, unsigned *cx, unsigned *dx) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__x86_64__)
        __asm__("movq\t%%rbx, %%rsi\n\t"
                "cpuid\n\t"
                "xchgq\t%%rbx, %%rsi\n\t"
                : "=a"(*ax), "=S"(*bx), "=c"(*cx), "=d"(*dx)
                : "a"(v));
        return true;
    #elif defined(__i386__)
        __asm__("movl\t%%bx, %%esi\n\t"
                "cpuid\n\t"
                "xchgl\t%%bx, %%esi\n\t"
                : "=a"(*ax), "=S"(*bx), "=c"(*cx), "=d"(*dx)
                : "a"(v));
    #else
        assert(false && "Unknown Platform(x86_cpuid)");
    #endif
#elif defined(_MSC_VER)
    int regs[4];
    __cpuid(regs, v);
    *ax = regs[0];
    *bx = regs[1];
    *cx = regs[2];
    *dx = regs[3];
    return true;
#else
    assert(false && "Unknown Compiler(x86_cpuid)");
    return false;
#endif
}
static bool x86_cpuid_ex(unsigned v, unsigned subleaf, unsigned *ax, unsigned *bx, unsigned *cx, unsigned *dx) {
#if defined(__x86_64__)
    #if defined(__GNUC__) || defined(__clang__)
        __asm__("movq\t%%rbx, %%rsi\n\t"
                "cpuid\n\t"
                "xchgq\t%%rbx, %%rsi\n\t"
                : "=a"(*ax), "=S"(*bx), "=c"(*cx), "=d"(*dx)
                : "a"(v), "c"(subleaf));
        return true;
    #elif defined(_MSC_VER)
        int regs[4];
        __cpuidex(regs, v, subleaf);
        *ax = regs[0];
        *bx = regs[1];
        *cx = regs[2];
        *dx = regs[3];
        return true;
    #else
        assert(false && "Unknown Compiler(x86_cpuid_ex)");
        return false;
    #endif
#elif defined(__i386__)
    #if defined(__GNUC__) || defined(__clang__)
        __asm__("movl\t%%bx, %%esi\n\t"
                "cpuid\n\t"
                "xchgl\t%%bx, %%esi\n\t"
                : "=a"(*ax), "=S"(*bx), "=c"(*cx), "=d"(*dx)
                : "a"(v), "c"(subleaf));
        return true;
    #elif defined(_MSC_VER)
        __asm {
            mov   ax,v
            mov   cx,subleaf
            cpuid
            mov   esi,ax
            mov   dword ptr [esi],ax
            mov   esi,bx
            mov   dword ptr [esi],bx
            mov   esi,cx
            mov   dword ptr [esi],cx
            mov   esi,dx
            mov   dword ptr [esi],dx
        }
        return true;
    #else
        assert(false && "Unknown Compiler(x86_cpuid_ex)");
        return false;
    #endif
#else
    assert(false && "Unknown Platform(x86_cpuid_ex)");
    return false;
#endif
}
static bool x86_xgetbv(unsigned *ax, unsigned *dx) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__(".byte 0x0f, 0x01, 0xd0" : "=a"(*ax), "=d"(*dx) : "c"(0));
    return true;
#elif defined(_MSC_FULL_VER) && defined(_XCR_XFEATURE_ENABLED_MASK)
    unsigned long long Result = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
    *ax = Result;
    *dx = Result >> 32;
    return true;
#else
    return false;
#endif
}

bool get_cpu_features(std::unordered_map<std::string, bool> &features) {
    unsigned ax = 0, bx = 0, cx = 0, dx = 0;
    unsigned max_level;
    if (!x86_cpuid(0, &max_level, &bx, &cx, &dx) || max_level < 1) {
        return false;
    }
    x86_cpuid(1, &ax, &bx, &cx, &dx);

    features["cmov"] = (dx >> 15) & 1;
    features["mmx"] = (dx >> 23) & 1;
    features["sse"] = (dx >> 25) & 1;
    features["sse2"] = (dx >> 26) & 1;
    features["sse3"] = (cx >> 0) & 1;
    features["ssse3"] = (cx >> 9) & 1;
    features["sse4.1"] = (cx >> 19) & 1;
    features["sse4.2"] = (cx >> 20) & 1;

    features["pclmul"] = (cx >> 1) & 1;
    features["cx16"] = (cx >> 13) & 1;
    features["movbe"] = (cx >> 22) & 1;
    features["popcnt"] = (cx >> 23) & 1;
    features["aes"] = (cx >> 25) & 1;
    features["rdrnd"] = (cx >> 30) & 1;

    // If CPUID indicates support for XSAVE, XRESTORE and AVX, and XGETBV
    // indicates that the AVX registers will be saved and restored on context
    // switch, then we have full AVX support.
    bool has_avx_save = ((cx >> 27) & 1) && ((cx >> 28) & 1) &&
                    x86_xgetbv(&ax, &dx) && ((ax & 0x6) == 0x6);
    features["avx"] = has_avx_save;
    features["fma"] = has_avx_save && (cx >> 12) & 1;
    features["f16c"] = has_avx_save && (cx >> 29) & 1;

    // Only enable XSAVE if OS has enabled support for saving YMM state.
    features["xsave"] = has_avx_save && (cx >> 26) & 1;

    // AVX512 requires additional context to be saved by the OS.
    bool has_avx512_save = has_avx_save && ((ax & 0xe0) == 0xe0);

    unsigned max_ext_level;
    x86_cpuid(0x80000000, &max_ext_level, &bx, &cx, &dx);

    bool hax_ext_leaf1 = max_ext_level >= 0x80000001 && x86_cpuid(0x80000001, &ax, &bx, &cx, &dx);
    features["lzcnt"] = hax_ext_leaf1 && ((cx >> 5) & 1);
    features["sse4a"] = hax_ext_leaf1 && ((cx >> 6) & 1);
    features["prfchw"] = hax_ext_leaf1 && ((cx >> 8) & 1);
    features["xop"] = hax_ext_leaf1 && ((cx >> 11) & 1) && has_avx_save;
    features["fma4"] = hax_ext_leaf1 && ((cx >> 16) & 1) && has_avx_save;
    features["tbm"] = hax_ext_leaf1 && ((cx >> 21) & 1);
    features["mwaitx"] = hax_ext_leaf1 && ((cx >> 29) & 1);

    bool has_leaf7 =
        max_level >= 7 && x86_cpuid_ex(0x7, 0x0, &ax, &bx, &cx, &dx);

    // AVX2 is only supported if we have the OS save support from AVX.
    features["avx2"] = has_avx_save && has_leaf7 && ((bx >> 5) & 1);

    features["fsgsbase"] = has_leaf7 && ((bx >> 0) & 1);
    features["sgx"] = has_leaf7 && ((bx >> 2) & 1);
    features["bmi"] = has_leaf7 && ((bx >> 3) & 1);
    features["hle"] = has_leaf7 && ((bx >> 4) & 1);
    features["bmi2"] = has_leaf7 && ((bx >> 8) & 1);
    features["invpcid"] = has_leaf7 && ((bx >> 10) & 1);
    features["rtm"] = has_leaf7 && ((bx >> 11) & 1);
    features["rdseed"] = has_leaf7 && ((bx >> 18) & 1);
    features["adx"] = has_leaf7 && ((bx >> 19) & 1);
    features["smap"] = has_leaf7 && ((bx >> 20) & 1);
    features["pcommit"] = has_leaf7 && ((bx >> 22) & 1);
    features["clflushopt"] = has_leaf7 && ((bx >> 23) & 1);
    features["clwb"] = has_leaf7 && ((bx >> 24) & 1);
    features["sha"] = has_leaf7 && ((bx >> 29) & 1);

    // AVX512 is only supported if the OS supports the context save for it.
    features["avx512f"] = has_leaf7 && ((bx >> 16) & 1) && has_avx512_save;
    features["avx512dq"] = has_leaf7 && ((bx >> 17) & 1) && has_avx512_save;
    features["avx512ifma"] = has_leaf7 && ((bx >> 21) & 1) && has_avx512_save;
    features["avx512pf"] = has_leaf7 && ((bx >> 26) & 1) && has_avx512_save;
    features["avx512er"] = has_leaf7 && ((bx >> 27) & 1) && has_avx512_save;
    features["avx512cd"] = has_leaf7 && ((bx >> 28) & 1) && has_avx512_save;
    features["avx512bw"] = has_leaf7 && ((bx >> 30) & 1) && has_avx512_save;
    features["avx512vl"] = has_leaf7 && ((bx >> 31) & 1) && has_avx512_save;

    features["prefetchwt1"] = has_leaf7 && (cx & 1);
    features["avx512vbmi"] = has_leaf7 && ((cx >> 1) & 1) && has_avx512_save;
    // Enable protection keys
    features["pku"] = has_leaf7 && ((cx >> 4) & 1);

    bool has_leafd = max_level >= 0xd &&
                    x86_cpuid_ex(0xd, 0x1, &ax, &bx, &cx, &dx);

    // Only enable XSAVE if OS has enabled support for saving YMM state.
    features["xsaveopt"] = has_avx_save && has_leafd && ((ax >> 0) & 1);
    features["xsavec"] = has_avx_save && has_leafd && ((ax >> 1) & 1);
    features["xsaves"] = has_avx_save && has_leafd && ((ax >> 3) & 1);

    return true;
}

} // namespace fingera

#if defined(CPU_FEATURES_BUILD_MAIN)

#include <iostream>

int main(int argc, const char *argv[]) {
    std::unordered_map<std::string, bool> features;
    if (!fingera::get_cpu_features(features)) {
        return 1;
    }
    for (auto i = features.begin(); i != features.end(); ++i) {
        if (i->second) {
            std::cout << i->first << ";";
        }
    }
    return 0;
}

#endif
