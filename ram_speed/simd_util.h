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

#endif //__SIMD_UTIL_H__
