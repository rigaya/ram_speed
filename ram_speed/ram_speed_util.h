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
#ifndef __RGY_UTIL_H__
#define __RGY_UTIL_H__

#include "rgy_tchar.h"
#if defined(_WIN32) || defined(_WIN64)
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif
#include <vector>
#include <array>
#include <string>
#include <chrono>
#include <cassert>
#include <memory>
#include <algorithm>
#include <unordered_map>
#include <climits>
#include <map>
#include <list>
#include <sstream>
#include <atomic>
#include <functional>
#include <type_traits>
#include "rgy_osdep.h"

typedef std::basic_string<TCHAR> tstring;

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x)
#endif

#if defined(_MSC_VER)
#define DO_PRAGMA(x)
#define RGY_DISABLE_WARNING_PUSH
#define RGY_DISABLE_WARNING_STR(str)
#define RGY_DISABLE_WARNING_POP
#elif defined(__clang__)
#define DO_PRAGMA_(x) _Pragma (#x)
#define DO_PRAGMA(x)  DO_PRAGMA_(x)
#define RGY_DISABLE_WARNING_PUSH     DO_PRAGMA(clang diagnostic push)
#define RGY_DISABLE_WARNING_NUM(num)
#define RGY_DISABLE_WARNING_STR(str) DO_PRAGMA(clang diagnostic ignored str)
#define RGY_DISABLE_WARNING_POP      DO_PRAGMA(clang diagnostic pop)
#elif defined(__GNUC__)
#define DO_PRAGMA_(x) _Pragma (#x)
#define DO_PRAGMA(x)  DO_PRAGMA_(x)
#define RGY_DISABLE_WARNING_PUSH     DO_PRAGMA(GCC diagnostic push)
#define RGY_DISABLE_WARNING_NUM(num)
#define RGY_DISABLE_WARNING_STR(str) DO_PRAGMA(GCC diagnostic ignored str)
#define RGY_DISABLE_WARNING_POP      DO_PRAGMA(GCC diagnostic pop)
#endif

using std::vector;
using std::unique_ptr;
using std::shared_ptr;

#ifndef MIN3
#define MIN3(a,b,c) (min((a), min((b), (c))))
#endif
#ifndef MAX3
#define MAX3(a,b,c) (max((a), max((b), (c))))
#endif

#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

#define ALIGN(x,align) (((x)+((align)-1))&(~((align)-1)))
#define ALIGN16(x) (((x)+15)&(~15))
#define ALIGN32(x) (((x)+31)&(~31))

#define MAP_PAIR_0_1_PROTO(prefix, name0, type0, name1, type1) \
    type1 prefix ## _ ## name0 ## _to_ ## name1(type0 var0); \
    type0 prefix ## _ ## name1 ## _to_ ## name0(type1 var1);

#define MAP_PAIR_0_1(prefix, name0, type0, name1, type1, map_pair, default0, default1) \
    RGY_NOINLINE \
    type1 prefix ## _ ## name0 ## _to_ ## name1(type0 var0) {\
        auto ret = std::find_if(map_pair.begin(), map_pair.end(), [var0](std::pair<type0, type1> a) { \
            return a.first == var0; \
        }); \
        return (ret == map_pair.end()) ? (default1) : ret->second; \
    } \
    RGY_NOINLINE  \
    type0 prefix ## _ ## name1 ## _to_ ## name0(type1 var1) {\
        auto ret = std::find_if(map_pair.begin(), map_pair.end(), [var1](std::pair<type0, type1> a) { \
            return a.second == var1; \
        }); \
        return (ret == map_pair.end()) ? (default0) : ret->first; \
    }

typedef long long lls;
typedef unsigned long long llu;

#define RGY_MEMSET_ZERO(x) { memset(&(x), 0, sizeof(x)); }

template<typename T, size_t size>
std::vector<T> make_vector(T(&ptr)[size]) {
    return std::vector<T>(ptr, ptr + size);
}
template<typename T, size_t size>
std::vector<T> make_vector(const T(&ptr)[size]) {
    return std::vector<T>(ptr, ptr + size);
}
template<typename T, typename... ArgTypes>
std::vector<T> make_vector(ArgTypes... args) {
    return std::vector<T>{ reinterpret_cast<T>(args)... };
}
template<typename T0, typename T1>
std::vector<T0> make_vector(const T0 *ptr, T1 size) {
    static_assert(std::is_integral<T1>::value == true, "T1 should be integral");
    return (ptr && size) ? std::vector<T0>(ptr, ptr + size) : std::vector<T0>();
}
template<typename T0, typename T1>
std::vector<T0> make_vector(T0 *ptr, T1 size) {
    static_assert(std::is_integral<T1>::value == true, "T1 should be integral");
    return (ptr && size) ? std::vector<T0>(ptr, ptr + size) : std::vector<T0>();
}
template<typename T, typename ...Args>
constexpr std::array<T, sizeof...(Args)> make_array(Args&&... args) {
    return std::array<T, sizeof...(Args)>{ static_cast<Args&&>(args)... };
}
template<typename T, std::size_t N>
constexpr std::size_t array_size(const std::array<T, N>&) {
    return N;
}
template<typename T, std::size_t N>
constexpr std::size_t array_size(T(&)[N]) {
    return N;
}
template<typename T>
void vector_cat(vector<T>& v1, const vector<T>& v2) {
    if (v2.size()) {
        v1.insert(v1.end(), v2.begin(), v2.end());
    }
}
template<typename T>
void vector_cat(std::vector<T>& v1, const T *ptr, size_t nCount) {
    if (nCount) {
        size_t currentSize = v1.size();
        v1.resize(currentSize + nCount);
        memcpy(v1.data() + currentSize, ptr, sizeof(T) * nCount);
    }
}
template<typename T>
static void rgy_free(T& ptr) {
    static_assert(std::is_pointer<T>::value == true, "T should be pointer");
    if (ptr) {
        free(ptr);
        ptr = nullptr;
    }
}

template<typename T>
using unique_ptr_custom = std::unique_ptr<T, std::function<void(T*)>>;

struct aligned_malloc_deleter {
    void operator()(void* ptr) const {
        _aligned_free(ptr);
    }
};

struct malloc_deleter {
    void operator()(void* ptr) const {
        free(ptr);
    }
};

struct fp_deleter {
    void operator()(FILE* fp) const {
        if (fp) {
            fflush(fp);
            fclose(fp);
        }
    }
};

struct handle_deleter {
    void operator()(HANDLE handle) const {
        if (handle) {
#if defined(_WIN32) || defined(_WIN64)
            CloseHandle(handle);
#endif //#if defined(_WIN32) || defined(_WIN64)
        }
    }
};

struct module_deleter {
    void operator()(void *hmodule) const {
        if (hmodule) {
#if defined(_WIN32) || defined(_WIN64)
            FreeLibrary((HMODULE)hmodule);
#endif //#if defined(_WIN32) || defined(_WIN64)
        }
    }
};

template<typename T>
static inline T rgy_gcd(T a, T b) {
    static_assert(std::is_integral<T>::value, "rgy_gcd is defined only for integer.");
    if (a == 0) return b;
    if (b == 0) return a;
    T c;
    while ((c = a % b) != 0)
        a = b, b = c;
    return b;
}

template<typename T>
static inline T rgy_gcd(std::pair<T, T> int2) {
    return rgy_gcd(int2.first, int2.second);
}

template<typename T>
static inline T rgy_lcm(T a, T b) {
    static_assert(std::is_integral<T>::value, "rgy_lcm is defined only for integer.");
    if (a == 0) return 0;
    if (b == 0) return 0;
    T gcd = rgy_gcd(a, b);
    a /= gcd;
    b /= gcd;
    return a * b * gcd;
}

template<typename T>
static inline T rgy_lcm(std::pair<T, T> int2) {
    return rgy_lcm(int2.first, int2.second);
}

template<typename T>
static inline void rgy_reduce(T& a, T& b) {
    static_assert(std::is_integral<T>::value, "rgy_reduce is defined only for integer.");
    if (a == 0 || b == 0) return;
    T gcd = rgy_gcd(a, b);
    a /= gcd;
    b /= gcd;
}

template<typename T>
static inline void rgy_reduce(std::pair<T, T>& int2) {
    rgy_reduce(int2.first, int2.second);
}

template<typename T>
class rgy_rational {
    static_assert(std::is_integral<T>::value, "rgy_rational is defined only for integer.");
private:
    T num, den;
public:
    rgy_rational() : num(0), den(1) {}
    rgy_rational(T _num, T _den) : num(_num), den(_den) { reduce(); }
    rgy_rational(const rgy_rational<T>& r) : num(r.num), den(r.den) { reduce(); }
    rgy_rational<T>& operator=(const rgy_rational<T> &r) { num = r.num; den = r.den; reduce(); return *this; }
    bool is_valid() const { return den != 0; };
    T n() const {
        return this->num;
    }
    T d() const {
        return this->den;
    }
    float qfloat() const {
        return (float)qdouble();
    }
    double qdouble() const {
        return (double)num / (double)den;
    }
    void reduce() {
        if (den == 0) {
            return;
        }
        rgy_reduce(num, den);
        if (den < 0) {
            num = -num;
            den = -den;
        }
    }
    rgy_rational<T> inv() const {
        rgy_rational<T> tmp(den, num);
        if (tmp.den == 0) {
            tmp.den = 0;
            tmp.num = 0;
        } else if (tmp.den < 0) {
            tmp.num = -tmp.num;
            tmp.den = -tmp.den;
        }
        return tmp;
    }

    rgy_rational<T> operator+ () {
        return *this;
    }
    rgy_rational<T> operator- () {
        return rgy_rational<T>(-1 * this->num, this->den);
    }

    rgy_rational<T>& operator+= (const rgy_rational<T>& r) {
        if (r.den == 0 || den == 0) {
            den = 0;
            num = 0;
            return *this;
        }

        T gcd0 = rgy_gcd(den, r.den);
        den /= gcd0;
        T tmp = r.den / gcd0;
        num = num * tmp + r.num * den;
        T gcd1 = rgy_gcd(num, gcd0);
        num /= gcd1;
        tmp = r.den / gcd1;
        den *= tmp;

        return *this;
    }
    rgy_rational<T>& operator-= (const rgy_rational<T>& r) {
        rgy_rational<T> tmp(r);
        tmp.num *= -1;
        *this += tmp;
        return *this;
    }
    rgy_rational<T>& operator*= (const rgy_rational<T>& r) {
        if (r.den == 0 || den == 0) {
            den = 0;
            num = 0;
            return *this;
        }
        T gcd0 = rgy_gcd(num, r.den);
        T gcd1 = rgy_gcd(den, r.num);
        T a0 = num / gcd0;
        T a1 = r.num / gcd1;
        T b0 = den / gcd1;
        T b1 = r.den / gcd0;
        num = a0 * a1;
        den = b0 * b1;

        if (den < 0) {
            num = -num;
            den = -den;
        }
        return *this;
    }
    rgy_rational<T>& operator/= (const rgy_rational<T>& r) {
        *this *= r.inv();
        return *this;
    }

    rgy_rational<T>& operator+= (const T& i) {
        num += i * den;
        return *this;
    }
    rgy_rational<T>& operator-= (const T& i) {
        num -= i * den;
        return *this;
    }
    rgy_rational<T>& operator*= (const T& i) {
        T gcd = rgy_gcd(i, den);
        num *= i / gcd;
        den /= gcd;
        return *this;
    }
    rgy_rational<T>& operator/= (const T& i) {
        if (i == 0) {
            num = 0;
            den = 0;
        } else if (num != 0) {
            T gcd = rgy_gcd(num, i);
            num /= gcd;
            den *= i / gcd;
            if (den < 0) {
                num = -num;
                den = -den;
            }
        }
        return *this;
    }

    template<typename Arg>
    rgy_rational<T> operator + (const Arg& a) {
        rgy_rational<T> t(*this);
        t += a;
        return t;
    }
    template<typename Arg>
    rgy_rational<T> operator - (const Arg& a) {
        rgy_rational<T> t(*this);
        t -= a;
        return t;
    }
    template<typename Arg>
    rgy_rational<T> operator * (const Arg& a) {
        rgy_rational<T> t(*this);
        t *= a;
        return t;
    }
    template<typename Arg>
    rgy_rational<T> operator / (const Arg& a) {
        rgy_rational<T> t(*this);
        t /= a;
        return t;
    }
    const rgy_rational<T>& operator++() { num += den; return *this; }
    const rgy_rational<T>& operator--() { num -= den; return *this; }

    bool operator== (const rgy_rational<T>& r) const {
        return ((num == r.num) && (den == r.den));
    }
    bool operator!= (const rgy_rational<T>& r) const {
        return ((num != r.num) || (den != r.den));
    }

    std::string print() const {
        std::stringstream ss;
        ss << num << "/" << den;
        return ss.str();
    }

    std::wstring printw() const {
        std::wstringstream ss;
        ss << num << "/" << den;
        return ss.str();
    }

    tstring printt() const {
#if _UNICODE
        return printw();
#else
        return print();
#endif
    }
};

static int64_t rgy_change_scale(int64_t t, const rgy_rational<int>& scale_in, const rgy_rational<int>& scale_out) {
    rgy_rational<int64_t> a = rgy_rational<int64_t>(scale_in.n(), scale_in.d());
    rgy_rational<int64_t> b = rgy_rational<int64_t>(scale_out.n(), scale_out.d());
    a *= t;
    a /= b;
    int64_t n = ((a.n() + a.d() / 2) / a.d());
    return n;
}

template<typename T>
void atomic_max(std::atomic<T> &maximum_value, T const &value) noexcept {
    T prev_value = maximum_value;
    while (prev_value < value &&
        !maximum_value.compare_exchange_weak(prev_value, value));
}

#if UNICODE
#define to_tstring to_wstring
#else
#define to_tstring to_string
#endif

typedef std::basic_stringstream<TCHAR> TStringStream;

#pragma warning (push)
#pragma warning (disable: 4244)
#pragma warning (disable: 4996)
static inline std::string tolowercase(const std::string& str) {
    std::string str_copy = str;
    std::transform(str_copy.cbegin(), str_copy.cend(), str_copy.begin(), tolower);
    return str_copy;
}
static inline std::string touppercase(const std::string &str) {
    std::string str_copy = str;
    std::transform(str_copy.cbegin(), str_copy.cend(), str_copy.begin(), toupper);
    return str_copy;
}
#if defined(_WIN32) || defined(_WIN64)
static inline std::wstring tolowercase(const std::wstring &str) {
    auto temp = wcsdup(str.data());
    _wcslwr(temp);
    std::wstring str_lo = temp;
    free(temp);
    return str_lo;
}
static inline std::wstring touppercase(const std::wstring &str) {
    auto temp = wcsdup(str.data());
    _wcsupr(temp);
    std::wstring str_lo = temp;
    free(temp);
    return str_lo;
}
#endif //#if defined(_WIN32) || defined(_WIN64)
#pragma warning (pop)

unsigned int wstring_to_string(const wchar_t *wstr, std::string& str, uint32_t codepage = CP_THREAD_ACP);
std::string wstring_to_string(const wchar_t *wstr, uint32_t codepage = CP_THREAD_ACP);
std::string wstring_to_string(const std::wstring& wstr, uint32_t codepage = CP_THREAD_ACP);
unsigned int char_to_wstring(std::wstring& wstr, const char *str, uint32_t codepage = CP_THREAD_ACP);
std::wstring char_to_wstring(const char *str, uint32_t = CP_THREAD_ACP);
std::wstring char_to_wstring(const std::string& str, uint32_t codepage = CP_THREAD_ACP);
#if defined(_WIN32) || defined(_WIN64)
std::wstring strsprintf(const WCHAR* format, ...);

std::wstring str_replace(std::wstring str, const std::wstring& from, const std::wstring& to);
std::wstring GetFullPath(const WCHAR *path);
bool rgy_get_filesize(const WCHAR *filepath, uint64_t *filesize);
std::pair<int, std::wstring> PathRemoveFileSpecFixed(const std::wstring& path);
std::wstring PathRemoveExtensionS(const std::wstring& path);
std::wstring PathCombineS(const std::wstring& dir, const std::wstring& filename);
std::string PathCombineS(const std::string& dir, const std::string& filename);
bool CreateDirectoryRecursive(const WCHAR *dir);
std::vector<tstring> get_file_list(const tstring& pattern, const tstring& dir);
#endif //#if defined(_WIN32) || defined(_WIN64)
tstring getExeDir();

std::wstring tchar_to_wstring(const tstring& tstr, uint32_t codepage = CP_THREAD_ACP);
std::wstring tchar_to_wstring(const TCHAR *tstr, uint32_t codepage = CP_THREAD_ACP);
unsigned int tchar_to_string(const TCHAR *tstr, std::string& str, uint32_t codepage = CP_THREAD_ACP);
std::string tchar_to_string(const TCHAR *tstr, uint32_t codepage = CP_THREAD_ACP);
std::string tchar_to_string(const tstring& tstr, uint32_t codepage = CP_THREAD_ACP);
unsigned int char_to_tstring(tstring& tstr, const char *str, uint32_t codepage = CP_THREAD_ACP);
tstring char_to_tstring(const char *str, uint32_t codepage = CP_THREAD_ACP);
tstring char_to_tstring(const std::string& str, uint32_t codepage = CP_THREAD_ACP);
unsigned int wstring_to_tstring(const WCHAR *wstr, tstring& tstr, uint32_t codepage = CP_THREAD_ACP);
tstring wstring_to_tstring(const WCHAR *wstr, uint32_t codepage = CP_THREAD_ACP);
tstring wstring_to_tstring(const std::wstring& wstr, uint32_t codepage = CP_THREAD_ACP);
unsigned int char_to_string(std::string& dst, uint32_t codepage_to, const char *src, uint32_t codepage_from = CP_THREAD_ACP);
std::string char_to_string(uint32_t codepage_to, const char *src, uint32_t codepage_from = CP_THREAD_ACP);

std::string strsprintf(const char* format, ...);
std::vector<std::wstring> split(const std::wstring &str, const std::wstring &delim, bool bTrim = false);
std::vector<std::string> split(const std::string &str, const std::string &delim, bool bTrim = false);
std::string lstrip(const std::string& string, const char* trim = " \t\v\r\n");
std::string rstrip(const std::string& string, const char* trim = " \t\v\r\n");
std::string trim(const std::string& string, const char* trim = " \t\v\r\n");
std::wstring lstrip(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");
std::wstring rstrip(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");
std::wstring trim(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");

#if defined(_WIN32) || defined(_WIN64)
std::vector<std::wstring> sep_cmd(const std::wstring &cmd);
std::vector<std::string> sep_cmd(const std::string &cmd);
#endif //#if defined(_WIN32) || defined(_WIN64)

std::string str_replace(std::string str, const std::string& from, const std::string& to);
std::string GetFullPath(const char *path);
bool rgy_get_filesize(const char *filepath, uint64_t *filesize);
std::pair<int, std::string> PathRemoveFileSpecFixed(const std::string& path);
std::string PathRemoveExtensionS(const std::string& path);
bool CreateDirectoryRecursive(const char *dir);

tstring print_time(double time);

static inline uint16_t readUB16(const void *ptr) {
    uint16_t i = *(uint16_t *)ptr;
    return (i >> 8) | (i << 8);
}

static inline uint32_t readUB32(const void *ptr) {
    uint32_t i = *(uint32_t *)ptr;
    return (i >> 24) | ((i & 0xff0000) >> 8) | ((i & 0xff00) << 8) | ((i & 0xff) << 24);
}

static inline uint32_t check_range_unsigned(uint32_t value, uint32_t min, uint32_t max) {
    return (value - min) <= (max - min);
}

static inline uint32_t popcnt32(uint32_t bits) {
    bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
    bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
    bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
    bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
    return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
}

static inline uint32_t popcnt64(uint64_t bits) {
    bits = (bits & 0x5555555555555555) + (bits >> 1 & 0x5555555555555555);
    bits = (bits & 0x3333333333333333) + (bits >> 2 & 0x3333333333333333);
    bits = (bits & 0x0f0f0f0f0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f0f0f0f0f);
    bits = (bits & 0x00ff00ff00ff00ff) + (bits >> 8 & 0x00ff00ff00ff00ff);
    bits = (bits & 0x0000ffff0000ffff) + (bits >>16 & 0x0000ffff0000ffff);
    bits = (bits & 0x00000000ffffffff) + (bits >>32 & 0x00000000ffffffff);
    return (uint32_t)bits;
}

template<typename type>
static std::basic_string<type> repeatStr(std::basic_string<type> str, int count) {
    std::basic_string<type> ret;
    for (int i = 0; i < count; i++) {
        ret += str;
    }
    return ret;
}

static tstring fourccToStr(uint32_t nFourCC) {
    tstring fcc;
    for (int i = 0; i < 4; i++) {
        fcc.push_back((TCHAR)*(i + (char*)&nFourCC));
    }
    return fcc;
}

bool check_ext(const TCHAR *filename, const std::vector<const char*>& ext_list);
bool check_ext(const tstring& filename, const std::vector<const char*>& ext_list);

//拡張子が一致するか確認する
static BOOL _tcheck_ext(const TCHAR *filename, const TCHAR *ext) {
    return (_tcsicmp(PathFindExtension(filename), ext) == 0) ? TRUE : FALSE;
}

int rgy_print_stderr(int log_level, const TCHAR *mes, HANDLE handle = NULL);

#if defined(_WIN32) || defined(_WIN64)
tstring getOSVersion(OSVERSIONINFOEXW *osinfo);
tstring getOSVersion();
#else
tstring getOSVersion();
#endif
BOOL rgy_is_64bit_os();
uint64_t getPhysicalRamSize(uint64_t *ramUsed);

tstring getEnviromentInfo();

struct rgy_time {
    int h, m, s, ms, us, ns;

    rgy_time() : h(0), m(0), s(0), ms(0), us(0), ns(0) {};
    rgy_time(double time_sec) : h(0), m(0), s(0), ms(0), us(0), ns(0) {
        s = (int)time_sec;
        time_sec -= s;
        ns = (int)(time_sec * 1e9 + 0.5);
        us = ns / 1000;
        ns -= us * 1000;
        ms = us / 1000;
        us -= ms * 1000;

        m = (int)(s / 60);
        s -= m * 60;
        h = m / 60;
        m -= h * 60;
    };
    rgy_time(uint32_t millisec) : h(0), m(0), s(0), ms(0), us(0), ns(0) {
        s = (int)(millisec / 1000);
        ms = (int)(millisec - s * 1000);
        m = s / 60;
        s -= m * 60;
        h = m / 60;
        m -= h * 60;
    };
    rgy_time(int64_t millisec) : h(0), m(0), s(0), ms(0), us(0), ns(0) {
        int64_t sec = millisec / 1000;
        ms = (int)(millisec - sec * 1000);

        int64_t min = sec / 60;
        s = (int)(sec - min * 60);

        h = (int)(min / 60);
        m = (int)(min - h * 60);
    }
    int64_t in_sec() {
        return (((int64_t)h * 60 + m) * 60 + s + (((ms + ((us >= 500) ? 1 : 0)) >= 500) ? 1 : 0));
    };
    int64_t in_ms() {
        return (((int64_t)h * 60 + m) * 60 + s) * 1000 + ms + ((us >= 500) ? 1 : 0);
    };
    tstring print() {
        auto str = strsprintf(_T("%d:%02d:%02d.%3d"), h, m, s, ms);
        if (us) {
#if _UNICODE
            str += std::to_wstring(us);
#else
            str += std::to_string(us);
#endif
        }
        if (ns) {
#if _UNICODE
            str += std::to_wstring(ns);
#else
            str += std::to_string(ns);
#endif
        }
        return str;
    };
};

class CombinationGenerator {
public:
    CombinationGenerator(int i) : m_nCombination(i) {

    }
    void create(vector<int> used) {
        if ((int)used.size() == m_nCombination) {
            m_nCombinationList.push_back(used);
        }
        for (int i = 0; i < m_nCombination; i++) {
            if (std::find(used.begin(), used.end(), i) == used.end()) {
                vector<int> u = used;
                u.push_back(i);
                create(u);
            }
        }
    }
    vector<vector<int>> generate() {
        vector<int> used;
        create(used);
        return m_nCombinationList;
    };
    int m_nCombination;
    vector<vector<int>> m_nCombinationList;
};

template<typename T>
class RGYListRef {
private:
    std::vector<std::unique_ptr<T>> m_objs;
    std::unordered_map<T *, std::atomic<int>> m_refCounts;
public:
    RGYListRef() : m_objs(), m_refCounts() {};
    ~RGYListRef() {
        m_refCounts.clear();
        m_objs.clear();
    }
    std::shared_ptr<T> get(T *ptr) {
        if (ptr == nullptr || m_refCounts.count(ptr) == 0) {
            return std::shared_ptr<T>();
        }
        m_refCounts[ptr]++;
        return std::shared_ptr<T>(ptr, [this](T *ptr) {
            m_refCounts[ptr]--;
        });
    }
    std::shared_ptr<T> get(std::function<int(T*)> initFunc = nullptr) {
        for (auto &count : m_refCounts) {
            if (count.second == 0) {
                m_refCounts[count.first]++;
                return std::shared_ptr<T>(count.first, [this](T *ptr) {
                    m_refCounts[ptr]--;
                });
            }
        }

        auto obj = std::make_unique<T>();
        auto ptr = obj.get();
        if (initFunc && initFunc(ptr)) {
            return std::shared_ptr<T>();
        }
        m_refCounts[ptr] = 1;
        m_objs.push_back(std::move(obj));
        return std::shared_ptr<T>(ptr, [this](T *ptr) {
            m_refCounts[ptr]--;
        });
    }
};

int rgy_avx_dummy_if_avail(int bAVXAvail);

unsigned short float2half(float value);

#endif //__RGY_UTIL_H__
