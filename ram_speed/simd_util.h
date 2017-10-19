// -----------------------------------------------------------------------------------------
// ram_speed by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

#ifndef __SIMD_UTIL_H__
#define __SIMD_UTIL_H__

#include <stdint.h>

enum {
    NONE       = 0x0000,
    SSE2       = 0x0001,
    SSE3       = 0x0002,
    SSSE3      = 0x0004,
    SSE41      = 0x0008,
    SSE42      = 0x0010,
    POPCNT     = 0x0020,
    AVX        = 0x0040,
    AVX2       = 0x0080,
    AVX512F    = 0x0100,
    AVX512DQ   = 0x0200,
    AVX512IFMA = 0x0400,
    AVX512PF   = 0x0800,
    AVX512ER   = 0x1000,
    AVX512CD   = 0x2000,
    AVX512BW   = 0x4000,
    AVX512VL   = 0x8000,
    AVX512VBMI = 0x10000,
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
