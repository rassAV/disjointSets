#include "disjoint_sets.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <memory>
#include <boost/container/vector.hpp>
#include <boost/container/map.hpp>

// Для измерения памяти
#ifdef _WIN32
#include <Windows.h>
#include <psapi.h> 
#elif defined(__linux__)
#include <sys/resource.h>
#endif

// Заглушка для памяти, подсчет в Windows
#ifdef _WIN32
size_t get_memory_usage() {
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return pmc.WorkingSetSize;
}
// Заглушка для памяти, подсчет в Linux
#elif defined(__linux__)
size_t get_memory_usage() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_maxrss * 1024;
}
// В других системах
#else
size_t get_memory_usage() {
    return 0;
}
#endif

// Измерение времени
template<typename Func>
void measure_execution(Func&& func, const std::string& name) {
    auto start = std::chrono::steady_clock::now();
    func();
    auto end = std::chrono::steady_clock::now();
    std::cout << name << " took "
        << std::chrono::duration<double, std::milli>(end - start).count()
        << " ms\n";

    size_t memory_usage = get_memory_usage();
    std::cout << "Memory usage: " << memory_usage / (1024 * 1024) << " MB\n";
    std::cout << "----------------------------------------------\n";
}

int main() {
    const std::string csv_file_name = "./files/src_dst_2.csv";
    const std::string json_file_name = "./files/dest_list_2.json";

    // STL контейнеры
    {
        std::vector<int> dst_list;
        std::map<int, std::vector<int>> src_to_dsts;
        std::map<int, std::vector<int>> group_to_srcs;
        std::map<int, std::vector<int>> group_to_dsts;

        measure_execution([&]() {
            load_json(json_file_name, dst_list);
            load_data_filtered(csv_file_name, dst_list, src_to_dsts);
            group_data(src_to_dsts, group_to_srcs, group_to_dsts);
            }, "STL Containers");
    }

    // Boost контейнеры
    {
        boost::container::vector<int> dst_list2;
        boost::container::map<int, boost::container::vector<int>> src_to_dsts2;
        boost::container::map<int, boost::container::vector<int>> group_to_srcs2;
        boost::container::map<int, boost::container::vector<int>> group_to_dsts2;

        measure_execution([&]() {
            load_json(json_file_name, dst_list2);
            load_data_filtered(csv_file_name, dst_list2, src_to_dsts2);
            group_data(src_to_dsts2, group_to_srcs2, group_to_dsts2);
            }, "Boost Containers");
    }

    return 0;
}
