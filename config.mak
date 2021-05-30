SRCS =  ram_speed/ram_speed.cpp ram_speed/cpu_info.cpp ram_speed/rgy_simd.cpp ram_speed/rgy_env.cpp ram_speed/rgy_util.cpp ram_speed/rgy_codepage.cpp
SRCASMS =  ram_speed/ram_speed_x64.asm
SRCDIR = .
CXX = g++
AS = nasm
LD = g++
PROGRAM = ram_speed64
ENABLE_DEBUG = 0
ENABLE_ASM = 1
CXXFLAGS = -std=c++17 -Wall -Wno-unknown-pragmas -Wno-unused -Wno-missing-braces -I./ram_speed -pthread -DNO_XGETBV_INTRIN=1 -fPIE -O3 -DNDEBUG=1 -mfpmath=sse -ffast-math -fomit-frame-pointer -DLINUX -DUNIX -D_FILE_OFFSET_BITS=64 -D__USE_LARGEFILE64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -DLINUX64 -m64
ASFLAGS = -I./ram_speed -DLINUX=1 -f elf64 -DARCH_X86_64=1
LDFLAGS = -L. -ldl -lstdc++ -lstdc++fs -m64 -pthread
PREFIX = /usr/local
