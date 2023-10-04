#include <cstdio>
#include <unordered_map>

std::unordered_map<int, std::unordered_map<int, int>> log_data;

void _start_logging() {
    log_data = std::unordered_map<int, std::unordered_map<int, int>>();
}

void _logop(int fun_counter, int loop_counter) {
    if (log_data.find(fun_counter) == log_data.end()) {
        log_data[fun_counter] = std::unordered_map<int, int>();
    }
    auto &log = log_data[fun_counter];
    if (log.find(loop_counter) == log.end()) {
        log[loop_counter] = 0;
    }
    log[loop_counter]++;
}

void _end_logging() {
    for (auto &[fun_counter, log] : log_data) {
        for (auto &[loop_counter, num_iterations] : log) {
            printf("function %d had its %dth loop header execute %d times\n", fun_counter, loop_counter, num_iterations);
        }
    }
}

// https://isocpp.org/wiki/faq/mixing-c-and-cpp
extern "C" void start_logging() {
    _start_logging();
}

extern "C" void logop(int fun_counter, int loop_counter) {
    _logop(fun_counter, loop_counter);
}

extern "C" void end_logging() {
    _end_logging();
}
