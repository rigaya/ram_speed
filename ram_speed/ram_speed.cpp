// -----------------------------------------------------------------------------------------
// ram_speed by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2020 rigaya
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

#include "rgy_osdep.h"
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <numeric>
#include <map>
#include <algorithm>
#include <thread>
#include <atomic>
#include <random>
#include <cmath>
#include <array>

#include "cpu_info.h"
#include "rgy_simd.h"
#include "rgy_util.h"
#include "rgy_thread_affinity.h"
#include "rgy_env.h"
#include "ram_speed.h"

static inline size_t align_size(size_t i) {
    return (i + 511) & ~511;
}

struct RamSpeedParam {
    RamSpeedMode test_mode;
    RGYThreadAffinityMode thread_affinity_mode;

    RamSpeedParam() : test_mode(RamSpeedMode::RW), thread_affinity_mode(RGYThreadAffinityMode::ALL) {};
};

struct RAM_SPEED_THREAD {
    RamSpeedMode test_mode;
    int thread_id;
    size_t check_size_bytes;
    RGYThreadAffinity thread_affinity;
    uint32_t physical_cores;
    double megabytes_per_sec;
};

static int get_target_core_count(const cpu_info_t *cpuinfo, RGYThreadAffinityMode thread_affinity_mode) {
    switch (thread_affinity_mode) {
    case RGYThreadAffinityMode::PCORE: return cpuinfo->physical_cores_p;
    case RGYThreadAffinityMode::ECORE: return cpuinfo->physical_cores_e;
#if defined(_WIN32) || defined(_WIN64)
    case RGYThreadAffinityMode::LOGICAL: return cpuinfo->logical_cores;
    case RGYThreadAffinityMode::PHYSICAL: return cpuinfo->physical_cores;
#endif //#if defined(_WIN32) || defined(_WIN64)
    default: return cpuinfo->physical_cores;
    }
}

struct RAM_SPEED_THREAD_WAKE {
    std::atomic<size_t> check_bit_start;
    std::atomic<size_t> check_bit_fin;
    size_t check_bit_all;
};


typedef uint32_t index_t;

#if ENABLE_ASM
#ifdef __cplusplus
extern "C" {
#endif
        extern void read_sse(uint8_t *src, uint32_t size, uint32_t count_n);
        extern void read_avx(uint8_t *src, uint32_t size, uint32_t count_n);
        extern void read_avx512(uint8_t *src, uint32_t size, uint32_t count_n);
        extern void write_sse(uint8_t *dst, uint32_t size, uint32_t count_n);
        extern void write_avx(uint8_t *dst, uint32_t size, uint32_t count_n);
        extern void write_avx512(uint8_t *dst, uint32_t size, uint32_t count_n);
        extern void ram_latency_test(index_t *src);
#ifdef __cplusplus
}
#endif

#else //#if ENABLE_ASM

#include <cstdint>
#if defined(__x86__) || defined(__x86_64__) || defined(_M_X86) || defined(_M_X64)
#include <emmintrin.h>
using data128 = __m128i;
#elif defined(__aarch64__) || defined(_M_ARM)
#include "arm_neon.h"
using data128 = int32x4_t;
#endif

RGY_NOINLINE
void read_c(uint8_t *src, uint32_t size, uint32_t count_n) {
    data128 dat0, dat1, dat2, dat3;
    do {
        int inner_count = size >> 8;
        volatile uint8_t *ptr = src;
        do {
            dat0 = *(volatile data128 *)(ptr +   0);
            dat1 = *(volatile data128 *)(ptr +  16);
            dat2 = *(volatile data128 *)(ptr +  32);
            dat3 = *(volatile data128 *)(ptr +  48);
            dat0 = *(volatile data128 *)(ptr +  64);
            dat1 = *(volatile data128 *)(ptr +  80);
            dat2 = *(volatile data128 *)(ptr +  96);
            dat3 = *(volatile data128 *)(ptr + 112);
            ptr += 128;
            dat0 = *(volatile data128 *)(ptr +   0);
            dat1 = *(volatile data128 *)(ptr +  16);
            dat2 = *(volatile data128 *)(ptr +  32);
            dat3 = *(volatile data128 *)(ptr +  48);
            dat0 = *(volatile data128 *)(ptr +  64);
            dat1 = *(volatile data128 *)(ptr +  80);
            dat2 = *(volatile data128 *)(ptr +  96);
            dat3 = *(volatile data128 *)(ptr + 112);
            ptr += 128;
            inner_count--;
        } while (inner_count > 0);
        count_n--;
    } while (count_n > 0);
}

RGY_NOINLINE
void write_c(uint8_t *src, uint32_t size, uint32_t count_n) {
    data128 dat0 = {0}, dat1 = {0}, dat2 = {0}, dat3 = {0};
    do {
        int inner_count = size >> 8;
        volatile uint8_t *ptr = src;
        do {
            *(volatile data128 *)(ptr +   0) = dat0;
            *(volatile data128 *)(ptr +  16) = dat1;
            *(volatile data128 *)(ptr +  32) = dat2;
            *(volatile data128 *)(ptr +  48) = dat3;
            *(volatile data128 *)(ptr +  64) = dat0;
            *(volatile data128 *)(ptr +  80) = dat1;
            *(volatile data128 *)(ptr +  96) = dat2;
            *(volatile data128 *)(ptr + 112) = dat3;
            ptr += 128;
            *(volatile data128 *)(ptr +   0) = dat0;
            *(volatile data128 *)(ptr +  16) = dat1;
            *(volatile data128 *)(ptr +  32) = dat2;
            *(volatile data128 *)(ptr +  48) = dat3;
            *(volatile data128 *)(ptr +  64) = dat0;
            *(volatile data128 *)(ptr +  80) = dat1;
            *(volatile data128 *)(ptr +  96) = dat2;
            *(volatile data128 *)(ptr + 112) = dat3;
            ptr += 128;
            inner_count--;
        } while (inner_count > 0);
        count_n--;
    } while (count_n > 0);
}

RGY_NOINLINE
void ram_latency_test(volatile index_t *src) {
    uint32_t idx = src[0];
    while (idx)
        idx = src[idx];
}
#endif //#if ENABLE_ASM

typedef void(*func_ram_test)(uint8_t *dst, uint32_t size, uint32_t count_n);

void ram_speed_func(RAM_SPEED_THREAD *thread_prm, RAM_SPEED_THREAD_WAKE *thread_wk) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST); ////高優先度で実行
    SetThreadAffinityMask(GetCurrentThread(), thread_prm->thread_affinity.getMask(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(0));

    const int TEST_COUNT = 31;
    size_t check_size_bytes = align_size(thread_prm->check_size_bytes);
    const size_t test_bytes   = std::max<size_t>(((thread_prm->test_mode == RamSpeedMode::READ) ? 4096 : 2048) * 1024 * thread_prm->physical_cores, check_size_bytes);
    const size_t warmup_bytes = test_bytes * 2;
    uint8_t *ptr = (uint8_t *)_aligned_malloc(check_size_bytes, 64);
    for (size_t i = 0; i < check_size_bytes; i++)
        ptr[i] = 0;
    uint32_t count_n = std::max(1u, (uint32_t)(test_bytes / check_size_bytes));
    const int avx = (0 != (get_availableSIMD() & AVX)) + (0 != (get_availableSIMD() & AVX512F));
    int64_t result[TEST_COUNT];
    static const func_ram_test RAM_TEST_LIST[][3] = {
    #if ENABLE_ASM
        { read_sse, write_sse },
        { read_avx, write_avx },
        { read_avx512, write_avx512 }
    #else
        { read_c, write_c },
        { read_c, write_c },
        { read_c, write_c }
    #endif
    };

    const func_ram_test ram_test = RAM_TEST_LIST[avx][thread_prm->test_mode];
    ram_test(ptr, check_size_bytes, std::max(1u, (uint32_t)(std::min<size_t>(64 * warmup_bytes, 16 * 1024 * 1024) / check_size_bytes)));

    thread_wk->check_bit_start |= (size_t)1 << thread_prm->thread_id;
    auto check_bit_expected = thread_wk->check_bit_all;
    while (!thread_wk->check_bit_start.compare_exchange_strong(check_bit_expected, thread_wk->check_bit_all)) {
        ram_test(ptr, check_size_bytes, std::max(1u, (uint32_t)(warmup_bytes / check_size_bytes)));
        thread_wk->check_bit_start |= (size_t)1 << thread_prm->thread_id;
        check_bit_expected = thread_wk->check_bit_all;
    }

    for (int i = 0; i < TEST_COUNT; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        ram_test(ptr, check_size_bytes, count_n);
        auto fin = std::chrono::high_resolution_clock::now();
        result[i] = std::chrono::duration_cast<std::chrono::microseconds>(fin - start).count();
    }
    thread_wk->check_bit_fin |= (size_t)1 << thread_prm->thread_id;
    check_bit_expected = thread_wk->check_bit_all;
    while (!thread_wk->check_bit_fin.compare_exchange_strong(check_bit_expected, thread_wk->check_bit_all)) {
        ram_test(ptr, check_size_bytes, std::max(1u, (uint32_t)(warmup_bytes / check_size_bytes)));
        thread_wk->check_bit_fin |= (size_t)1 << thread_prm->thread_id;
        check_bit_expected = thread_wk->check_bit_all;
    }
    _aligned_free(ptr);

    //中央値を採用
    std::sort(result, result + TEST_COUNT);
    const auto time = result[TEST_COUNT / 2];

    thread_prm->megabytes_per_sec = (check_size_bytes * (double)count_n / (1024.0 * 1024.0)) / (time * 0.000001);
}

bool check_size_and_thread(size_t check_size, int thread_n) {
    for (int i = 0; i < thread_n; i++) {
        auto size_i0 = align_size((i + 0) * check_size / thread_n);
        auto size_i1 = align_size((i + 1) * check_size / thread_n);
        if (size_i1 == size_i0) {
            return false;
        }
    }
    return true;
}

double ram_speed_mt(const size_t check_size, const RamSpeedMode test_mode, const RGYThreadAffinityMode thread_affinity_mode, int thread_n) {
    thread_n = std::min<int>(thread_n, sizeof(size_t) * 8);
    std::vector<std::thread> threads;
    std::vector<RAM_SPEED_THREAD> thread_prm(thread_n);
    RAM_SPEED_THREAD_WAKE thread_wake;
    cpu_info_t cpu_info;
    get_cpu_info(&cpu_info);

    thread_wake.check_bit_start = 0;
    thread_wake.check_bit_fin = 0;
    thread_wake.check_bit_all = 0;
    for (int i = 0; i < thread_n; i++) {
        thread_wake.check_bit_all |= (size_t)1 << i;
    }
    for (int i = 0; i < thread_n; i++) {
        auto size_i0 = align_size((i + 0) * check_size / thread_n);
        auto size_i1 = align_size((i + 1) * check_size / thread_n);
        thread_prm[i].physical_cores = cpu_info.physical_cores;
        thread_prm[i].test_mode = (test_mode == RamSpeedMode::RW) ? (RamSpeedMode)(i & 1) : test_mode;
        thread_prm[i].check_size_bytes = size_i1 - size_i0;
        thread_prm[i].thread_id = i;
        thread_prm[i].thread_affinity = RGYThreadAffinity(thread_affinity_mode, 1llu << i);
        threads.push_back(std::thread(ram_speed_func, &thread_prm[i], &thread_wake));
    }
    for (int i = 0; i < thread_n; i++) {
        threads[i].join();
    }

    double sum = 0.0;
    for (const auto& prm : thread_prm) {
        sum += prm.megabytes_per_sec;
    }
    return sum;
}

std::vector<double> ram_speed_mt_list(const size_t check_size, const RamSpeedMode test_mode, const RGYThreadAffinityMode thread_affinity_mode) {
    cpu_info_t cpu_info;
    get_cpu_info(&cpu_info);

    std::vector<double> results;
    for (int ith = 1; ith <= cpu_info.physical_cores; ith++) {
        if (!check_size_and_thread(check_size, ith)) {
            return results;
        }
        results.push_back(ram_speed_mt(check_size, test_mode, thread_affinity_mode, ith));
    }
    return results;
}

enum RamLatencyTest {
    RL_TEST_SEQUENTIAL,
    RL_TEST_CL_FORWARD,
    RL_TEST_CL_FORWARD2,
    RL_TEST_RANDOM_PAGE,
    RL_TEST_RANDOM_FULL,
};

double ram_latency(RamLatencyTest test, size_t size, int loop_count, int test_count) {
    std::vector<double> result(test_count);
    const index_t elems = (index_t)((size + 3) / 4);
    std::unique_ptr<index_t, decltype(&_aligned_free)> buffer((index_t *)_aligned_malloc(elems * sizeof(index_t), 64), _aligned_free);
    auto buf_ptr = buffer.get();
    memset(buf_ptr, 0, elems * sizeof(index_t));

    if (test == RL_TEST_SEQUENTIAL) {
        index_t prev_idx = 0;
        for (index_t i = 1; i < elems; i++) {
            buf_ptr[prev_idx] = i;
            prev_idx = i;
        }
    } else if (test == RL_TEST_CL_FORWARD) {
        const index_t step_size = 128 / sizeof(index_t);
        const index_t step_elems = (elems + step_size - 1) / step_size;
        index_t prev_idx = 0;
        for (index_t j = 0; j < step_size; j++) {
            for (index_t i = 0; i < step_elems; i++) {
                index_t idx = i * step_size + j;
                if (idx != 0 && idx < elems) {
                    buf_ptr[prev_idx] = idx;
                    prev_idx = idx;
                }
            }
        }
    } else if (test == RL_TEST_CL_FORWARD2) {
        const index_t cl_size = 64 / sizeof(index_t);
        const index_t cl_block1 = 16;
        const index_t cl_block2 = 4096 / (sizeof(index_t) * cl_block1 * cl_size);
        const index_t page_count = (size + 4095) / 4096;
        std::random_device rd;
        std::mt19937 mt(rd());
        std::vector<index_t> temp(cl_block1);
        index_t prev_idx = 0;
        for (index_t k = 0; k < cl_size; k++) {
            for (index_t ipage = 0; ipage < page_count; ipage++) {
                for (index_t j = 0; j < cl_block2; j++) {
                    for (index_t i = 0; i < cl_block1; i++) {
                        #define BLOCK_IDX(i, j, k) ((((ipage) * cl_block2 + (j)) * cl_block1 + (i)) * cl_size + (k))
                        if ((i|j|k) != 0) {
                            int page_unused = 0;
                            for (index_t x = 0; x < cl_block2; x++) {
                                index_t idx = BLOCK_IDX(i, x, k);
                                if (idx < elems && buf_ptr[idx] == 0) {
                                    temp[page_unused] = idx;
                                    page_unused++;
                                }
                            }
                            if (page_unused > 0) {
                                std::uniform_int_distribution<index_t> dist(0, page_unused-1);
                                index_t idx = temp[dist(mt)];
                                buf_ptr[prev_idx] = idx;
                                prev_idx = idx;
                            }
                        }
                        #undef BLOCK_IDX
                    }
                }
            }
        }
        buf_ptr[prev_idx] = 0;
    } else if (test == RL_TEST_RANDOM_PAGE) {
        const index_t page_size = 4096 / sizeof(index_t);
        const index_t page_counts = (elems + page_size - 1) / page_size;
        const index_t block_size = 128 / sizeof(index_t);
        const index_t block_elems = (page_size + block_size - 1) / block_size;
        std::random_device rd;
        std::mt19937 mt(rd());
        std::vector<index_t> temp(page_counts);
        #define BLOCK_IDX(i, j, k) ((k)*page_size + (i)*block_size + (j))
        index_t prev_idx = 0;
        for (index_t j = 0; j < block_size; j++) {
            for (index_t k = 0; k < page_counts; k++) {
                for (index_t i = 0; i < block_elems; i++) {
                    if ((i|j|k) != 0) {
                        int page_unused = 0;
                        for (index_t x = 0; x < page_counts; x++) {
                            index_t idx = BLOCK_IDX(i, j, x);
                            if (idx < elems && buf_ptr[idx] == 0) {
                                temp[page_unused] = idx;
                                page_unused++;
                            }
                        }
                        if (page_unused > 0) {
                            std::uniform_int_distribution<index_t> dist(0, page_unused-1);
                            index_t idx = temp[dist(mt)];
                            buf_ptr[prev_idx] = idx;
                            prev_idx = idx;
                        }
                    }
                }
            }
        }
        buf_ptr[prev_idx] = 0;
        #undef BLOCK_IDX
    } else if (test == RL_TEST_RANDOM_FULL) {
        std::vector<index_t> indexes;
        for (index_t i = 1; i < elems; i++) {
            indexes.push_back(i);
        }
        std::random_device rd;
        std::mt19937 mt(rd());
        index_t prev_idx = 0;
        uint64_t used_count = 0;
        while ((int64_t)indexes.size() - (int64_t)used_count > 0) {
            std::uniform_int_distribution<index_t> dist(0, (index_t)indexes.size()-1);
            index_t idx = 0;
            for (int itry = 0; itry < 8; itry++) {
                idx = dist(mt);
                while (indexes[idx] == 0) {
                    idx = dist(mt);
                }
                if (std::abs(((int64_t)prev_idx - indexes[idx])) >= (int64_t)(128 / sizeof(index_t))) break;
            }
            buf_ptr[prev_idx] = indexes[idx];
            prev_idx = indexes[idx];
            indexes[idx] = 0;
            used_count++;
            if (used_count * 2 > indexes.size()) {
                std::vector<index_t> temp;
                for (auto i : indexes) {
                    if (i > 0) {
                        temp.push_back(i);
                    }
                }
                indexes = temp;
                used_count = 0;
            }
        }
    }

    for (int i = 0; i < test_count; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int iloop = 0; iloop < loop_count; iloop++) {
            ram_latency_test(buf_ptr);
        }
        auto fin = std::chrono::high_resolution_clock::now();
        result[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(fin - start).count() / (double)(elems * loop_count);
    }
    std::sort(result.begin(), result.end());
    const auto time = result[result.size() / 2];
    return time;
}

const int INTER_CORE_ITER = 100000;

struct inter_core_data {
    alignas(64) std::atomic<int> s1;
    alignas(64) std::atomic<int> s2;
    alignas(64) bool abort;
    alignas(64) std::atomic<int> start;
    double result;
};

void func_inter_core_latency1(inter_core_data *data, uint64_t threadMask1) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST); //高優先度で実行
    SetThreadAffinityMask(GetCurrentThread(), threadMask1);
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    data->start++;
    while (data->start < 2) {
        // busy spin
    }
    auto value = data->s1.load();
    auto start = std::chrono::high_resolution_clock::now();
    while (data->s1 < INTER_CORE_ITER) {
        int new_val;
        while ((new_val = data->s2) != value) {
            // busy spin
        }
        value = new_val + 1;
        data->s1.exchange(value);
    }
    auto fin = std::chrono::high_resolution_clock::now();
    data->abort = true;
    data->result = std::chrono::duration_cast<std::chrono::nanoseconds>(fin - start).count() / (double)(INTER_CORE_ITER * 2);
}

void func_inter_core_latency2(inter_core_data *data, uint64_t threadMask2) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    SetThreadAffinityMask(GetCurrentThread(), threadMask2);
    std::this_thread::sleep_for(std::chrono::milliseconds(0));
    data->start++;
    while (data->start < 2) {
        // busy spin
    }
    auto value = data->s2.load();
    while (data->s1 < INTER_CORE_ITER) {
        while (value == data->s1) {
            // busy spin
        }
        value++;
        data->s2.exchange(value);
    }
}

double inter_core_latency(uint64_t threadMask1, uint64_t threadMask2) {
    inter_core_data shared_data;
    shared_data.s1 = 0;
    shared_data.s2 = 0;
    shared_data.abort = false;
    shared_data.start = 0;
    std::thread th1(func_inter_core_latency1, &shared_data, threadMask1);
    std::thread th2(func_inter_core_latency2, &shared_data, threadMask2);
    th1.join();
    th2.join();
    return shared_data.result;
}

double step(double d) {
    //if (d < 18.0) return 0.5; //256KB
    return 0.125;
}

void print(FILE *fp, const char *format, ...) {
    va_list args;
    va_start(args, format);

    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    std::vector<char> buffer(len, 0);
    if (buffer.data() != nullptr) {
        vsprintf_s(buffer.data(), len, format, args); // C4996
        fprintf(stderr, "%s", buffer.data());
        fprintf(fp, "%s", buffer.data());
    }
    va_end(args);
}

std::string getOutFilename() {
    char mes[256];
    getCPUName(mes, sizeof(mes));

    std::string outfilename = "result_" + str_replace(trim(mes), "@", " ");
    {
        auto outfilename_org = outfilename;
        outfilename = str_replace(outfilename, "  ", " ");
        while (outfilename != outfilename_org) {
            outfilename_org = outfilename;
            outfilename = str_replace(outfilename, "  ", " ");
        }
    }
    outfilename = str_replace(outfilename, " ", "_") + ".csv";
    return outfilename;
}

#define SSTRING(str) STRING(str)
#define STRING(str) #str

std::string printVersion() {
    return std::string("ram_speed " RAM_SPEED_VERSION
#if defined(_MSC_VER)
        " (VC " SSTRING(_MSC_VER)
#elif defined(__clang__)
        " (clang " SSTRING(__clang_major__) "." SSTRING(__clang_minor__) "." SSTRING(__clang_patchlevel__)
#elif defined(__GNUC__)
        " (gcc " SSTRING(__GNUC__) "." SSTRING(__GNUC_MINOR__) "." SSTRING(__GNUC_PATCHLEVEL__)
#else
        " (unknown"
#endif
        "/"
#ifdef _WIN32
        "Win"
#elif  __linux
        "Linux"
#else
        "unknown"
#endif
        "/"
#if ENABLE_ASM
     "asm:on"
#else
     "asm:off"
#endif
     ")");
}

std::string printHelp() {
    std::string str = printVersion() + "\n\n";
    str += "-o, --output     output file name\n";
    str += "                   default: result_<CPU name>.csv\n";
    str += "-v, --version    print version\n";
    str += "-h, --help       print help\n";
    str += "\n";
    str += "    --target <string>      target to run test on\n";
    str += "      physical(default), pcore, ecore\n";
    str += "\n";
    str += "    --mem-only             check ram only\n";
    str += "\n";
    str += "    --no-latency           disable latency tests\n";
    str += "    --no-latency-intercore disable intercore latency test\n";
    str += "    --no-latency-mem       disable memory latency test\n";
    str += "\n";
    str += "    --no-bandwidth         disable bandwidth tests\n";
    str += "    --no-bandwidth-read    disable bandwidth read test\n";
    str += "    --no-bandwidth-write   disable bandwidth write test\n";
    return str;
}

int main(int argc, char **argv) {
    bool chek_ram_only = false;
    bool check_latency_mem = true;
    bool check_latency_intercore = true;
    bool check_bandwidth_read = true;
    bool check_bandwidth_write = true;
    int interval_sleep = 0;
    int min_latency_tests = 5;
    RamSpeedParam prm;
    prm.thread_affinity_mode = RGYThreadAffinityMode::PHYSICAL;
    std::string outfilename;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--output" || std::string(argv[i]) == "-o") {
            outfilename = argv[i+1];
            i++;
            continue;
        }
        if (std::string(argv[i]) == "--version" || std::string(argv[i]) == "-v") {
            printf("%s\n", printVersion().c_str());
            exit(0);
        }
        if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
            printf("%s\n", printHelp().c_str());
            exit(0);
        }
        if (std::string(argv[i]) == "--cpuinfo" || std::string(argv[i]) == "-h") {
            cpu_info_t cpuinfo = get_cpu_info();
            printf("%s\n", print_cpu_info(&cpuinfo).c_str());
            exit(0);
        }
        if (std::string(argv[i]) == "--target") {
            if (i + 1 < argc) {
                prm.thread_affinity_mode = rgy_str_to_thread_affnity_mode(argv[i+1]);
                if (prm.thread_affinity_mode == RGYThreadAffinityMode::END) {
                    fprintf(stderr, "Invalid param for --target.");
                    exit(1);
                }
            } else {
                fprintf(stderr, "Invalid param for --target.");
                exit(1);
            }
            i++;
            continue;
        }
        if (std::string(argv[i]) == "--mem-only") {
            chek_ram_only = true;
            check_latency_intercore = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-latency") {
            check_latency_intercore = false;
            check_latency_mem = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-latency-intercore") {
            check_latency_intercore = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-latency-mem") {
            check_latency_mem = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-bandwidth") {
            check_bandwidth_read = false;
            check_bandwidth_write = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-bandwidth-read") {
            check_bandwidth_read = false;
            continue;
        }
        if (std::string(argv[i]) == "--no-bandwidth-write") {
            check_bandwidth_write = false;
            continue;
        }
        if (std::string(argv[i]) == "--interval-sleep") {
            try {
                interval_sleep = std::stoi(argv[i + 1]);
            } catch (...) {
                fprintf(stderr, "Invalid param for --interval-sleep.");
                exit(1);
            }
            i++;
            continue;
        }
        if (std::string(argv[i]) == "--min-latency-tests") {
            try {
                min_latency_tests = std::stoi(argv[i + 1]);
            } catch (...) {
                fprintf(stderr, "Invalid param for --min-latency-tests.");
                exit(1);
            }
            i++;
            continue;
        }
    }
    if (outfilename.length() == 0) {
        outfilename = getOutFilename();
    }

    std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(outfilename.c_str(), "w"), fclose);
    if (fp == NULL) {
        fprintf(stderr, "failed to open output file.\n");
        return 1;
    }
    print(fp.get(), "%s\n\n", printVersion().c_str());
    print(fp.get(), "%s\n", getEnviromentInfo().c_str());

    const cpu_info_t cpu_info = get_cpu_info();
    if (check_latency_intercore) {
        const int target_core_count = get_target_core_count(&cpu_info, prm.thread_affinity_mode);
        print(fp.get(), "inter core latency (ns)\n");
        for (int j = 0; j < target_core_count; j++) {
            for (int i = 0; i < target_core_count; i++) {
                const auto& proc_info = cpu_info.proc_list[i];
                if (i == j && proc_info.logical_cores <= 1) {
                    print(fp.get(), ",       ");
                } else {
                    const auto ithread = RGYThreadAffinity(prm.thread_affinity_mode, 1llu<<i);
                    const auto jthread = RGYThreadAffinity(prm.thread_affinity_mode, 1llu<<j);
                    const double result = inter_core_latency(ithread.getMask(0), jthread.getMask((i==j) ? 1 : 0));
                    print(fp.get(), "%s %7.2f", (i) ? "," : "", result);
                }
            }
            print(fp.get(), "\n");
        }
        fflush(fp.get());
        if (interval_sleep > 0) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_sleep));
        }
    }

    const double max_size = std::log2((double)(std::max(cpu_info.physical_cores, 8) * 32 * 1024 * 1024));
    if (check_latency_mem) {
        const auto maskPrev = SetThreadAffinityMask(GetCurrentThread(), RGYThreadAffinity(prm.thread_affinity_mode).getMask(0));
        std::this_thread::sleep_for(std::chrono::milliseconds(0));

        print(fp.get(), "\nram latency (ns)\n");
        const std::array<RamLatencyTest, 3> latency_tests = { RL_TEST_SEQUENTIAL, RL_TEST_CL_FORWARD2, RL_TEST_RANDOM_FULL };
        const std::map<RamLatencyTest, std::string> latency_tests_name = {
            { RL_TEST_SEQUENTIAL, "sequantial" },
            { RL_TEST_CL_FORWARD, "cacheline forward" },
            { RL_TEST_CL_FORWARD2, "cacheline forward2" },
            { RL_TEST_RANDOM_PAGE, "page random" },
            { RL_TEST_RANDOM_FULL, "full random" }
        };
        for (auto test : latency_tests) {
            print(fp.get(), ", %s", latency_tests_name.at(test).c_str());
        }
        print(fp.get(), "\n");
        fflush(fp.get());

        for (double i_size = (chek_ram_only) ? max_size : 12; i_size <= max_size; i_size += step(i_size)) {
            const size_t check_size = align_size(size_t(std::pow(2.0, i_size) + 0.5));
            print(fp.get(), "%6zd", check_size >> 10);
            int test_count = 5;
            if      (check_size <       256 * 1024) test_count = 61;
            else if (check_size <  1 * 1024 * 1024) test_count = 31;
            else if (check_size <  2 * 1024 * 1024) test_count = 21;
            else if (check_size <  4 * 1024 * 1024) test_count = 15;
            else if (check_size < 16 * 1024 * 1024) test_count = 11;
            test_count = std::min(test_count, min_latency_tests);
            for (auto test : latency_tests) {
                double latency = ram_latency(test, check_size, std::max(1, (int)(2 * 1024 * 1024 / check_size)), test_count);
                print(fp.get(), ", %.2f", latency);
            }
            print(fp.get(), "\n");
            if (interval_sleep > 0 && check_size >= 1 * 1024 * 1024) {
                fflush(fp.get());
                std::this_thread::sleep_for(std::chrono::seconds(interval_sleep));
            }
        }
        fflush(fp.get());
        SetThreadAffinityMask(GetCurrentThread(), maskPrev);
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    }

    if (check_bandwidth_read) {
        print(fp.get(), "\nread bandwidth (GB/s)\n");
        for (double i_size = (chek_ram_only) ? max_size : 12; i_size <= max_size; i_size += step(i_size)) {
            if (i_size >= sizeof(size_t) * 8) {
                break;
            }
            const size_t check_size = align_size(size_t(std::pow(2.0, i_size) + 0.5));
            const bool overMB = false; // check_size >= 1024 * 1024 * 1024;
            fprintf(fp.get(), "%6zd,", check_size >> 10);
            std::vector<double> results = ram_speed_mt_list(check_size, RamSpeedMode::READ, prm.thread_affinity_mode);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp.get(), "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6zd %s, %2d threads: %6.1f GB/s\n", check_size >> ((overMB) ? 20 : 10), (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp.get(), "\n");
            if (interval_sleep > 0 && check_size >= 1 * 1024 * 1024) {
                fflush(fp.get());
                std::this_thread::sleep_for(std::chrono::seconds(interval_sleep));
            }
        }
        fflush(fp.get());
    }

    if (check_bandwidth_write) {
        print(fp.get(), "\nwrite bandwidth (GB/s)\n");
        for (double i_size = (chek_ram_only) ? max_size : 12; i_size <= max_size; i_size += step(i_size)) {
            if (i_size >= sizeof(size_t) * 8) {
                break;
            }
            const size_t check_size = align_size(size_t(std::pow(2.0, i_size) + 0.5));
            const bool overMB = false; //check_size >= 1024 * 1024;
            fprintf(fp.get(), "%6zd,", check_size >> 10);
            std::vector<double> results = ram_speed_mt_list(check_size, RamSpeedMode::WRITE, prm.thread_affinity_mode);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp.get(), "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6zd %s, %2d threads: %6.1f GB/s\n", check_size >> ((overMB) ? 20 : 10), (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp.get(), "\n");
            if (interval_sleep > 0 && check_size >= 1 * 1024 * 1024) {
                fflush(fp.get());
                std::this_thread::sleep_for(std::chrono::seconds(interval_sleep));
            }
        }
        fclose(fp.get());
    }
    return 0;
}

