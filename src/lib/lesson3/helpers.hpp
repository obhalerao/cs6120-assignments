#ifndef L3_HELPERS_
#define L3_HELPERS_

#include "json.hpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <string>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <queue>

using json = nlohmann::json;
enum operation_t 
{
  b_add, 
  b_mul, 
  b_sub, 
  b_div,
  b_eq, 
  b_lt, 
  b_le,
  b_gt, 
  b_ge, 
  b_not, 
  b_and, 
  b_or, 
  b_id,
  b_const,
  b_fadd,
  b_fsub,
  b_fmul,
  b_fdiv,
  b_feq,
  b_flt,
  b_fle,
  b_fgt,
  b_fge,
  b_call,
  b_other
};

enum type_t
{
  b_int,
  b_bool,
  b_float,
  b_unk,
};

extern std::unordered_map<std::string, operation_t> str2op;

extern std::unordered_map<std::string, type_t> str2type;

// apparently floating point ops are commutative but not associative
extern std::unordered_set<operation_t> commutatives;

// If op represents a 1-arg operation, val2 is -1.
// If op is const, val1 refers to the value stored in the const.
struct Value{
  operation_t op;
  type_t b_type;
  int val1;
  int val2;


  // We conservatively define unknown types to be different from all other types.
  // TODO: maybe change to allow for more optimizations?
  bool operator==(const Value &other) const;
};

// Overriding std::hash for our new struct

namespace std{
  template <>
  struct hash<Value>{
    size_t operator()(const Value& value) const {
      auto h1 = std::hash<int>()((int)value.op);
      auto h2 = std::hash<int>()(value.val1);
      auto h3 = std::hash<int>()(value.val2);
      auto h4 = std::hash<int>()((int)value.b_type);
      return (((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1) >> 1) ^ (h4 << 1);
    }
  };
}

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

// Decomposes a Bril instruction list into its basic blocks
// Represented as a vector of JSON objects.

std::vector<json> form_blocks(json& instrs);

std::pair<std::vector<json>, bool> global_dce_pass(std::vector<json>& blocks);

std::pair<json, bool> local_dce_pass(json& blk);

json lvn_pass(json& blk, std::string var_suffix);

std::string find_suffix(json instrs);

#endif