﻿// -----------------------------------------------------------------------------------------
// QSVEnc/NVEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2011-2016 rigaya
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

#pragma once
#ifndef __RGY_OSDEP_H__
#define __RGY_OSDEP_H__

#if defined(_MSC_VER)
#ifndef RGY_FORCEINLINE
#define RGY_FORCEINLINE __forceinline
#endif
#ifndef RGY_NOINLINE
#define RGY_NOINLINE __declspec(noinline)
#endif
#else
#ifndef RGY_FORCEINLINE
#define RGY_FORCEINLINE inline
#endif
#ifndef RGY_NOINLINE
#define RGY_NOINLINE __attribute__ ((noinline))
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <process.h>
#include <io.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <shellapi.h>
#define RGY_LOAD_LIBRARY(x) LoadLibrary(x)
#define RGY_GET_PROC_ADDRESS GetProcAddress
#define RGY_FREE_LIBRARY FreeLibrary

#else //#if defined(_WIN32) || defined(_WIN64)
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <pthread.h>
#include <sched.h>
#include <dlfcn.h>

static inline void *_aligned_malloc(size_t size, size_t alignment) {
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}
#define _aligned_free free

typedef wchar_t WCHAR;
typedef int BOOL;
typedef void* HANDLE;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef int errno_t;

#define RGY_LOAD_LIBRARY(x) dlopen((x), RTLD_LAZY)
#define RGY_GET_PROC_ADDRESS dlsym
#define RGY_FREE_LIBRARY dlclose

static uint32_t CP_THREAD_ACP = 0;
static uint32_t CP_UTF8 = 0;

#define __stdcall
#define __fastcall

template <typename _CountofType, size_t _SizeOfArray>
char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) (int)sizeof(*__countof_helper(_Array))

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

struct LUID {
  int LowPart;
  int HighPart;
};

static inline char *strtok_s(char *strToken, const char *strDelimit, char **context) {
    return strtok(strToken, strDelimit);
}
static inline char *strcpy_s(char *dst, size_t size, const char *src) {
    return strcpy(dst, src);
}
static inline char *strcpy_s(char *dst, const char *src) {
    return strcpy(dst, src);
}
static inline char *strcat_s(char *dst, size_t size, const char *src) {
    return strcat(dst, src);
}
static inline int _vsprintf_s(char *buffer, size_t size, const char *format, va_list argptr) {
    return vsprintf(buffer, format, argptr);
}
#define sscanf_s sscanf
#define swscanf_s swscanf
#define vsprintf_s(buf, size, fmt, va)  vsprintf(buf, fmt, va)
#define vswprintf_s vswprintf
#define _strnicmp strncasecmp
#define stricmp strcasecmp
#define _stricmp stricmp

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

#if NO_XGETBV_INTRIN && defined(__AVX__)
static inline unsigned long long _xgetbv(unsigned int index) {
  unsigned int eax, edx;
  __asm__ __volatile__("xgetbv" : "=a"(eax), "=d"(edx) : "c"(index));
  return ((unsigned long long)edx << 32) | eax;
}
#endif

#if NO_RDTSCP_INTRIN
static inline uint64_t __rdtscp(uint32_t *Aux) {
    uint32_t aux;
    uint64_t rax,rdx;
    asm volatile ( "rdtscp\n" : "=a" (rax), "=d" (rdx), "=c" (aux) : : );
    *Aux = aux;
    return (rdx << 32) + rax;
}
#endif //#if NO_RDTSCP_INTRIN

//uint64_t __rdtsc() {
//    unsigned int eax, edx;
//    __asm__ volatile("rdtsc" : "=a"(eax), "=d"(edx));
//    return ((uint64_t)edx << 32) | eax;
//}

static short _InterlockedIncrement16(volatile short *pVariable) {
    return __sync_add_and_fetch((volatile short*)pVariable, 1);
}

static short _InterlockedDecrement16(volatile short *pVariable) {
    return __sync_sub_and_fetch((volatile short*)pVariable, 1);
}

static int32_t _InterlockedIncrement(volatile int32_t *pVariable) {
    return __sync_add_and_fetch((volatile int32_t*)pVariable, 1);
}

static int32_t _InterlockedDecrement(volatile int32_t *pVariable) {
    return __sync_sub_and_fetch((volatile int32_t*)pVariable, 1);
}

static inline int _vscprintf(const char * format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}

static inline int _vscwprintf(const WCHAR * format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vswprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}

static inline int sprintf_s(char *dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(dst, format, args);
    va_end(args);
    return ret;
}
static inline int sprintf_s(char *dst, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(dst, format, args);
    va_end(args);
    return ret;
}

static inline char *_fullpath(char *dst, const char *path, size_t size) {
    return realpath(path, dst);
}

static inline const char *PathFindExtension(const char *path) {
    return strrchr(basename(path), '.');
}

static inline const char *PathFindFileName(const char *path) {
    const int path_len = strlen(path) + 1;
    char *const buffer = (char *)calloc(path_len, sizeof(buffer[0]));
    if (buffer == nullptr) {
        return nullptr;
    }
    memcpy(buffer, path, path_len);
    char *ptr_basename = basename(buffer);
    const char *ptr_ret = nullptr;
    if (!(ptr_basename == nullptr || *ptr_basename == '.' || ptr_basename < buffer || buffer + path_len <= ptr_basename)) {
        ptr_ret = path + (ptr_basename - buffer);
    }
    free(buffer);
    return ptr_ret;
}
#define PathFindFileNameA PathFindFileName

static inline int PathFileExists(const char *path) {
    struct stat st;
    return 0 == stat(path, &st) && !S_ISDIR(st.st_mode);
}
static inline int PathIsDirectory(const char *dir) {
    struct stat st;
    return 0 == stat(dir, &st) && S_ISDIR(st.st_mode);
}
static inline BOOL CreateDirectory(const char *dir, void *dummy) {
    return mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO) == 0 ? 1 : 0;
}
#define PathFileExistsA PathFileExists
#define PathIsDirectoryA PathIsDirectory
#define CreateDirectoryA CreateDirectory
#define PathFindExtensionA PathFindExtension

static inline int PathIsUNC(const char *path) {
    return 0;
}

static inline int fopen_s(FILE **pfp, const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    *pfp = fp;
    return (fp == NULL) ? 1 : 0;
}

static uint32_t GetCurrentProcessId() {
    pid_t pid = getpid();
    return (uint32_t)pid;
}

static pthread_t GetCurrentThread() {
    return pthread_self();
}

static void SetThreadAffinityMask(pthread_t thread, size_t mask) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (uint32_t j = 0; j < sizeof(mask) * 8; j++) {
        if (mask & (1 << j)) {
            CPU_SET(j, &cpuset);
        }
    }
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}

enum {
    THREAD_PRIORITY_NORMAL,
    THREAD_PRIORITY_HIGHEST,
    THREAD_PRIORITY_ABOVE_NORMAL,
    THREAD_PRIORITY_BELOW_NORMAL,
    THREAD_PRIORITY_LOWEST,
    THREAD_PRIORITY_IDLE,
};

static void SetThreadPriority(pthread_t thread, int priority) {
    return; //何もしない
}

#define _fread_nolock fread
#define _fwrite_nolock fwrite
#define _fgetc_nolock fgetc
#define _fseeki64 fseek
#define _ftelli64 ftell

#endif //#if defined(_WIN32) || defined(_WIN64)

#endif //__RGY_OSDEP_H__
