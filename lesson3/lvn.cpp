#include "json.hpp"
#include <iostream>
#include <chrono>
#include <unordered_map>

enum opcode_t 
{
  b_add, 
  b_mul, 
  b_sub, 
  b_div,
  b_eq, 
  b_lt, 
  b_le, 
  b_ge, 
  b_not, 
  b_and, 
  b_or, 
  b_id
};

using json = nlohmann::json;

struct Instr{
  opcode_t opcode;
  std::string arg1;
  std::string arg2;
  std::string dest;

  bool operator==(const Instr &other) const {
    return (opcode == other.opcode
            && arg1 == other.arg1
            && arg2 == other.arg2
            && dest == other.dest);
  }
};

/*  Custom hash function for unordered_map 
    Taken from https://codeforces.com/blog/entry/62393
*/

struct custom_hash {
    static uint64_t splitmix64(uint64_t x) {
        // http://xorshift.di.unimi.it/splitmix64.c
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }

    template <typename T>
    size_t operator()(T &x) const {
        uint64_t xx = std::hash<T>(x);
        static const uint64_t FIXED_RANDOM = 
          std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(xx + FIXED_RANDOM);
    }
};

// Overriding std::hash for our new struct

template <>
struct std::hash<Instr>{
  std::size_t operator()(const Instr& instr) const {
    auto h1 = std::hash<int>()((int)instr.opcode);
    auto h2 = std::hash<std::string>()(instr.arg1);
    auto h3 = std::hash<std::string>()(instr.arg2);
    auto h4 = std::hash<std::string>()(instr.dest);
    return ((((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1)) >> 1) ^ (h4 << 1);
  }
};

int main(){

  // pipes in json as input
  json prog;
  // std::cin >> prog;
  // std::cout << prog;
  Instr x {b_add, "a", "b", "c"};
  std::unordered_map<Instr, int> instr2int;
  instr2int[x] = 5;
  std::cout << instr2int[x] << std::endl;

}