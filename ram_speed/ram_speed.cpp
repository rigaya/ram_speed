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

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cmath>

#include "cpu_info.h"
#include "simd_util.h"
#include "ram_speed.h"
#include "ram_speed_osdep.h"

static inline size_t align_size(size_t i) {
    return (i + 511) & ~511;
}

typedef struct {
    int mode;
    size_t check_size_bytes;
    uint32_t thread_id;
    uint32_t physical_cores;
    double megabytes_per_sec;
} RAM_SPEED_THREAD;


typedef struct {
    std::atomic<size_t> check_bit_start;
    std::atomic<size_t> check_bit_fin;
    size_t check_bit_all;
} RAM_SPEED_THREAD_WAKE;

#ifdef __cplusplus
extern "C" {
#endif
    extern void read_sse(uint8_t *src, uint32_t size, uint32_t count_n);
    extern void read_avx(uint8_t *src, uint32_t size, uint32_t count_n);
    extern void read_avx512(uint8_t *src, uint32_t size, uint32_t count_n);
    extern void write_sse(uint8_t *dst, uint32_t size, uint32_t count_n);
    extern void write_avx(uint8_t *dst, uint32_t size, uint32_t count_n);
    extern void write_avx512(uint8_t *dst, uint32_t size, uint32_t count_n);
#ifdef __cplusplus
}
#endif


typedef void(*func_ram_test)(uint8_t *dst, uint32_t size, uint32_t count_n);

void ram_speed_func(RAM_SPEED_THREAD *thread_prm, RAM_SPEED_THREAD_WAKE *thread_wk) {
    const int TEST_COUNT = 31;
    size_t check_size_bytes = align_size(thread_prm->check_size_bytes);
    const size_t test_bytes   = std::max<size_t>(((thread_prm->mode == RAM_SPEED_MODE_READ) ? 4096 : 2048) * 1024 * thread_prm->physical_cores, check_size_bytes);
    const size_t warmup_bytes = test_bytes * 2;
    uint8_t *ptr = (uint8_t *)_aligned_malloc(check_size_bytes, 64);
    for (size_t i = 0; i < check_size_bytes; i++)
        ptr[i] = 0;
    uint32_t count_n = std::max(1u, (uint32_t)(test_bytes / check_size_bytes));
    const int avx = (0 != (get_availableSIMD() & AVX)) + (0 != (get_availableSIMD() & AVX512F));
    int64_t result[TEST_COUNT];
    static const func_ram_test RAM_TEST_LIST[][3] = {
        { read_sse, write_sse },
        { read_avx, write_avx },
        { read_avx512, write_avx512 }
    };

    const func_ram_test ram_test = RAM_TEST_LIST[avx][thread_prm->mode];
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

int ram_speed_thread_id(int thread_index, const cpu_info_t& cpu_info) {
    int thread_id = (thread_index % cpu_info.physical_cores) * (cpu_info.logical_cores / cpu_info.physical_cores) + (int)(thread_index / cpu_info.physical_cores);;
#if defined(_WIN32) || defined(_WIN64)
    return thread_id;
#else
    return cpu_info.proc_list[thread_id].processor_id;
#endif
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

double ram_speed_mt(size_t check_size, int mode, int thread_n) {
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
        thread_wake.check_bit_all |= (size_t)1 << ram_speed_thread_id(i, cpu_info);
    }
    for (int i = 0; i < thread_n; i++) {
        auto size_i0 = align_size((i + 0) * check_size / thread_n);
        auto size_i1 = align_size((i + 1) * check_size / thread_n);
        thread_prm[i].physical_cores = cpu_info.physical_cores;
        thread_prm[i].mode = (mode == RAM_SPEED_MODE_RW) ? (i & 1) : mode;
        thread_prm[i].check_size_bytes = size_i1 - size_i0;
        thread_prm[i].thread_id = ram_speed_thread_id(i, cpu_info);
        threads.push_back(std::thread(ram_speed_func, &thread_prm[i], &thread_wake));
        //渡されたスレッドIDからスレッドAffinityを決定
        //特定のコアにスレッドを縛り付ける
        SetThreadAffinityMask(threads[i].native_handle(), (uint64_t)1 << (int)thread_prm[i].thread_id);
        //高優先度で実行
        SetThreadPriority(threads[i].native_handle(), THREAD_PRIORITY_HIGHEST);
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

std::vector<double> ram_speed_mt_list(size_t check_size, int mode, bool logical_core) {
    cpu_info_t cpu_info;
    get_cpu_info(&cpu_info);

    std::vector<double> results;
    for (uint32_t ith = 1; ith <= cpu_info.physical_cores; ith++) {
        if (!check_size_and_thread(check_size, ith)) {
            return results;
        }
        results.push_back(ram_speed_mt(check_size, mode, ith));
    }
    if (logical_core && cpu_info.logical_cores != cpu_info.physical_cores) {
        int smt = cpu_info.logical_cores / cpu_info.physical_cores;
        for (uint32_t ith = cpu_info.physical_cores+1; ith <= cpu_info.logical_cores; ith++) {
            if (!check_size_and_thread(check_size, ith)) {
                return results;
            }
            results.push_back(ram_speed_mt(check_size, mode, ith));
        }
    }
    return results;
}

double step(double d) {
    if (d < 18.0) return 0.5; //256KB
    return 0.25;
}

int main(int argc, char **argv) {
    bool check_logical_cores = false;
    bool chek_ram_only = false;
    for (int i = 1; i < argc; i++) {
        check_logical_cores |= (0 == _stricmp(argv[1], "-l"));
        chek_ram_only |= (0 == _stricmp(argv[1], "-r"));
    }

    char mes[256];
    getCPUInfo(mes, 256);
    fprintf(stderr, "%s\n", mes);
    FILE *fp = fopen("result.csv", "w");
    if (fp == NULL) {
        fprintf(stderr, "failed to open output file.\n");
    } else {
        cpu_info_t cpu_info;
        get_cpu_info(&cpu_info);

        const double max_size = std::log2((double)(cpu_info.physical_cores * 32 * 1024 * 1024));
        fprintf(fp, "%s\n", mes);
        fprintf(fp, "read\n");
        for (double i_size = (chek_ram_only) ? max_size : 12; i_size <= max_size; i_size += step(i_size)) {
            if (i_size >= sizeof(size_t) * 8) {
                break;
            }
            const size_t check_size = align_size(size_t(std::pow(2.0, i_size) + 0.5));
            const bool overMB = false; // check_size >= 1024 * 1024 * 1024;
            fprintf(fp, "%6d,", check_size >> 10);
            std::vector<double> results = ram_speed_mt_list(check_size, RAM_SPEED_MODE_READ, check_logical_cores);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp, "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6d %s, %2d threads: %6.1f GB/s\n", check_size >> ((overMB) ? 20 : 10), (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
        fprintf(fp, "write\n");
        for (double i_size = (chek_ram_only) ? max_size : 12; i_size <= max_size; i_size += step(i_size)) {
            if (i_size >= sizeof(size_t) * 8) {
                break;
            }
            const size_t check_size = align_size(size_t(std::pow(2.0, i_size) + 0.5));
            const bool overMB = false; //check_size >= 1024 * 1024;
            fprintf(fp, "%6d,", check_size >> 10);
            std::vector<double> results = ram_speed_mt_list(check_size, RAM_SPEED_MODE_WRITE, check_logical_cores);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp, "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6d %s, %2d threads: %6.1f GB/s\n", check_size >> ((overMB) ? 20 : 10), (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
}

