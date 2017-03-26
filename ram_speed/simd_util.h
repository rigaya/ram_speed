#ifndef __SIMD_UTIL_H__
#define __SIMD_UTIL_H__

#include <stdint.h>

enum {
    NONE   = 0x0000,
    SSE2   = 0x0001,
    SSE3   = 0x0002,
    SSSE3  = 0x0004,
    SSE41  = 0x0008,
    SSE42  = 0x0010,
    POPCNT = 0x0020,
    AVX    = 0x0040,
    AVX2   = 0x0080,
};

uint32_t get_availableSIMD();

#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h>
#else

#include <stdlib.h>
#include <pthread.h>

static inline void *_aligned_malloc(size_t size, size_t alignment) {
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}
#define _aligned_free free

static inline void __cpuid(int cpuInfo[4], int param) {
    int eax = 0, ebx = 0, ecx = 0, edx = 0;
    __asm("xor %%ecx, %%ecx\n\t"
    "cpuid" : "=a"(eax), "=b" (ebx), "=c"(ecx), "=d"(edx)
        : "0"(param));
    cpuInfo[0] = eax;
    cpuInfo[1] = ebx;
    cpuInfo[2] = ecx;
    cpuInfo[3] = edx;
}

static inline unsigned long long _xgetbv(unsigned int index) {
    unsigned int eax, edx;
    __asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(index));
    return ((unsigned long long)edx << 32) | eax;
}

#if NO_RDTSCP_INTRIN
static inline uint64_t __rdtscp(uint32_t *Aux) {
    uint32_t aux;
    uint64_t rax, rdx;
    asm volatile ("rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : :);
    *Aux = aux;
    return (rdx << 32) + rax;
}
#endif //#if NO_RDTSCP_INTRIN

#endif //#if !(defined(_WIN32) || defined(_WIN64))

#endif //__SIMD_UTIL_H__
