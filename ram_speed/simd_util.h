#pragma once

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
