//
// Created by sb866 on 10/3/23.
//

#ifndef CS6120_ASSIGNMENTS_RTLIB_HPP
#define CS6120_ASSIGNMENTS_RTLIB_HPP

#include <unordered_map>

extern std::unordered_map<int, std::unordered_map<int, int>> log_data;

extern "C" void start_logging();

extern "C" void logop(int fun_counter, int loop_counter);

extern "C" void end_logging();

#endif //CS6120_ASSIGNMENTS_RTLIB_HPP
