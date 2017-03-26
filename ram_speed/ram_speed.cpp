//  -----------------------------------------------------------------------------------------
//    ram_speed by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <thread>
#include <atomic>

#include "cpu_info.h"
#include "simd_util.h"
#include "ram_speed.h"
#include "ram_speed_osdep.h"

typedef struct {
    int mode;
    uint32_t check_size_bytes;
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
    extern void write_sse(uint8_t *dst, uint32_t size, uint32_t count_n);
    extern void write_avx(uint8_t *dst, uint32_t size, uint32_t count_n);
#ifdef __cplusplus
}
#endif


typedef void(*func_ram_test)(uint8_t *dst, uint32_t size, uint32_t count_n);

void ram_speed_func(RAM_SPEED_THREAD *thread_prm, RAM_SPEED_THREAD_WAKE *thread_wk) {
    const int TEST_COUNT = 31;
    uint32_t check_size_bytes = (thread_prm->check_size_bytes + 255) & ~255;
    const uint32_t test_kilo_bytes   = (uint32_t)(((thread_prm->mode == RAM_SPEED_MODE_READ) ? 1 : 0.5) * thread_prm->physical_cores * 4 * 1024 / (std::max)(1.0, log2(check_size_bytes / 1024.0)) + 0.5);
    const uint32_t warmup_kilo_bytes = test_kilo_bytes * 2;
    uint8_t *ptr = (uint8_t *)_aligned_malloc(check_size_bytes, 64);
    for (uint32_t i = 0; i < check_size_bytes; i++)
        ptr[i] = 0;
    uint32_t count_n = std::max(1, (int)(test_kilo_bytes * 1024.0 / check_size_bytes + 0.5));
    int avx = 0 != (get_availableSIMD() & AVX);
    int64_t result[TEST_COUNT];
    static const func_ram_test RAM_TEST_LIST[][2] = {
        { read_sse, write_sse },
        { read_avx, write_avx },
    };

    const func_ram_test ram_test = RAM_TEST_LIST[avx][thread_prm->mode];
    ram_test(ptr, check_size_bytes, std::max(1, (int)(warmup_kilo_bytes * 64 * 1024.0 / check_size_bytes + 0.5)));

    thread_wk->check_bit_start |= (size_t)1 << thread_prm->thread_id;
    auto check_bit_expected = thread_wk->check_bit_all;
    while (!thread_wk->check_bit_start.compare_exchange_strong(check_bit_expected, thread_wk->check_bit_all)) {
        ram_test(ptr, check_size_bytes, std::max(1, (int)(warmup_kilo_bytes * 1024.0 / check_size_bytes + 0.5)));
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
        ram_test(ptr, check_size_bytes, std::max(1, (int)(warmup_kilo_bytes * 1024.0 / check_size_bytes + 0.5)));
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
#if defined(_WIN32) || defined(_WIN64)
    return (thread_index % cpu_info.physical_cores) * (cpu_info.logical_cores / cpu_info.physical_cores) + (int)(thread_index / cpu_info.physical_cores);
#else
    return thread_index;
#endif
}

double ram_speed_mt(int check_size_kilobytes, int mode, int thread_n) {
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
        thread_prm[i].physical_cores = cpu_info.physical_cores;
        thread_prm[i].mode = (mode == RAM_SPEED_MODE_RW) ? (i & 1) : mode;
        thread_prm[i].check_size_bytes = (check_size_kilobytes * 1024 / thread_n + 255) & ~255;
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

std::vector<double> ram_speed_mt_list(int check_size_kilobytes, int mode, bool logical_core) {
    cpu_info_t cpu_info;
    get_cpu_info(&cpu_info);

    std::vector<double> results;
    for (uint32_t ith = 1; ith <= cpu_info.physical_cores; ith++) {
        results.push_back(ram_speed_mt(check_size_kilobytes, mode, ith));
    }
    if (logical_core && cpu_info.logical_cores != cpu_info.physical_cores) {
        int smt = cpu_info.logical_cores / cpu_info.physical_cores;
        for (uint32_t ith = cpu_info.physical_cores+1; ith <= cpu_info.logical_cores; ith++) {
            results.push_back(ram_speed_mt(check_size_kilobytes, mode, ith));
        }
    }
    return results;
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
        fprintf(fp, "%s\n", mes);
        fprintf(fp, "read\n");
        for (int i_size = (chek_ram_only) ? 17 : 1; i_size <= 17; i_size++) {
            const int size_in_kilo_byte = 1 << i_size;
            const bool overMB = size_in_kilo_byte >= 1024;
            fprintf(fp, "%6d %s,", (overMB) ? size_in_kilo_byte >> 10 : size_in_kilo_byte, (overMB) ? "MB" : "KB");
            std::vector<double> results = ram_speed_mt_list(size_in_kilo_byte, RAM_SPEED_MODE_READ, check_logical_cores);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp, "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6d %s, %2d threads: %6.1f GB/s\n", (overMB) ? size_in_kilo_byte >> 10 : size_in_kilo_byte, (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n");
        fprintf(fp, "write\n");
        for (int i_size = (chek_ram_only) ? 17 : 1; i_size <= 17; i_size++) {
            const int size_in_kilo_byte = 1 << i_size;
            const bool overMB = size_in_kilo_byte >= 1024;
            fprintf(fp, "%6d %s,", (overMB) ? size_in_kilo_byte >> 10 : size_in_kilo_byte, (overMB) ? "MB" : "KB");
            std::vector<double> results = ram_speed_mt_list(size_in_kilo_byte, RAM_SPEED_MODE_WRITE, check_logical_cores);
            for (uint32_t i = 0; i < results.size(); i++) {
                fprintf(fp, "%6.1f,", results[i] / 1024.0);
                fprintf(stderr, "%6d %s, %2d threads: %6.1f GB/s\n", (overMB) ? size_in_kilo_byte >> 10 : size_in_kilo_byte, (overMB) ? "MB" : "KB", i+1, results[i] / 1024.0);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
}

