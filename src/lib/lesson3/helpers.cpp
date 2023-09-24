#include "helpers.hpp"
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


std::unordered_map<std::string, operation_t> str2op = {
  {"add", b_add},
  {"mul", b_mul},
  {"sub", b_sub},
  {"div", b_div},
  {"eq", b_eq},
  {"lt", b_lt},
  {"gt", b_gt},
  {"le", b_le},
  {"ge", b_ge},
  {"not", b_not},
  {"and", b_and},
  {"or", b_or},
  {"id", b_id},
  {"fadd", b_fadd},
  {"fmul", b_fmul},
  {"fsub", b_fsub},
  {"fdiv", b_fdiv},
  {"feq", b_feq},
  {"flt", b_flt},
  {"fgt", b_fgt},
  {"fle", b_fle},
  {"fge", b_fge},
  {"call", b_call},
  {"const", b_const},
};

std::unordered_map<std::string, type_t> str2type = {
  {"int", b_int},
  {"bool", b_bool},
  {"float", b_float}
};

std::unordered_set<operation_t> commutatives( 
  {b_add, b_mul, b_eq, b_and, b_or, b_fadd, b_fmul, b_feq}
);

    // If op represents a 1-arg operation, val2 is -1.
// If op is const, val1 refers to the value stored in the const.
bool Value::operator==(const Value &other) const {
    return (op == other.op
            && b_type == other.b_type
            && val1 == other.val1
            && val2 == other.val2);
  }
// Decomposes a Bril instruction list into its basic blocks
// Represented as a vector of JSON objects.

std::vector<json> form_blocks(json& instrs){
  std::vector<json> blocks;
  json cur_block;
  for(auto instr: instrs){
    if(instr.contains("label")){
      if(cur_block.size() != 0){
        blocks.push_back(cur_block);
      }
      cur_block.clear();
    }
    cur_block.push_back(instr);
    if(instr.contains("op") && 
    (instr["op"] == "jmp" || (instr["op"] == "br"))){
      blocks.push_back(cur_block);
      cur_block.clear();
    }
  }
  if(cur_block.size() != 0){
    blocks.push_back(cur_block);
  }
  cur_block.clear();
  return blocks;
}

std::pair<std::vector<json>, bool> global_dce_pass(std::vector<json>& blocks){
  std::vector<json> new_json_arr;
  bool changed = false;
  std::set<std::string> total_args;
  for(auto blk: blocks){
    for(auto instr: blk){
      if(instr.contains("args")){
        for(std::string arg: instr["args"]) total_args.insert(arg);
      }
    }
  }
  for(auto blk: blocks){
    json new_json;
    for(auto instr: blk){
      if(instr.contains("dest")){
        std::string dest = instr["dest"];
        if(total_args.find(dest) == total_args.end()){
          changed = true;
          continue;
        }
      }
      new_json.push_back(instr);
    }
    new_json_arr.push_back(new_json);
  }
  return std::make_pair(new_json_arr, changed);
}

std::pair<json, bool> local_dce_pass(json& blk){
  bool changed = false;
  json new_json;
  std::unordered_map<std::string, int> instr_map;
  std::unordered_set<int> to_delete;
  int counter = 0;
  for(auto i = blk.begin(); i != blk.end(); i++, counter++){
    json cur = *i;
    if(cur.contains("args")){
      for(auto arg: cur["args"]){
        instr_map.erase(arg);
      }
    }

    if(cur.contains("dest") && instr_map.find(cur["dest"]) != instr_map.end()){
      to_delete.insert(instr_map[cur["dest"]]);
    }

    if(cur.contains("dest") && cur["op"] != "call"){
      instr_map[cur["dest"]] = counter;
    }
  }
  counter = 0;
  changed = !to_delete.empty();
  for(auto i = blk.begin(); i != blk.end(); i++, counter++){
    if(to_delete.find(counter) == to_delete.end()){
      new_json.push_back(*i);
    }
  }
  return std::make_pair(new_json, changed);
}

json lvn_pass(json& blk, std::string var_suffix){
  // Two unordered maps: one from instrs to numbers, and one from numbers to pair<instr, queue>
  // We also want one from variable names to numbers.
  // replace with custom hash once i figure out what's going on
  std::unordered_map<Value, int> value2num;
  std::unordered_map<std::string, int> var2num;
  std::vector<std::pair<Value, std::queue<std::string>>> table;

  json new_instrs;

  int counter = 0;

  for(auto ins: blk){
    if(ins.contains("label")){
      new_instrs.push_back(ins);
      continue;
    }
    operation_t op = str2op.find(ins["op"]) != str2op.end() ? str2op[ins["op"]] : b_other;

    // First, add all unseen args to the table.
    if(ins.contains("args")){
      for(auto arg: ins["args"]){
        if(var2num.find(arg) == var2num.end()){
          int num = table.size();
          var2num[arg] = num;
          Value tmp {b_id, b_unk, num, -1};
          value2num[tmp] = num;
          std::queue<std::string> q;
          q.push(arg);
          table.push_back(std::make_pair(tmp, q));
        }
      }
    }

    Value current_value {b_other, b_unk, -1, -1};

    // update lvn table if op is recognized
    if(op != b_other && op != b_const && op != b_call){

      current_value.op = op;
      if(ins["type"].type_name() != "string"){
        current_value.b_type = b_unk;
      }else{
        current_value.b_type = str2type[ins["type"]];
      }
      current_value.val1 = var2num[ins["args"][0]];

      // 1-arg operations
      if(op == b_id || op == b_not){
        current_value.val2 = -1;
      }else{
        //2-arg operations
        current_value.val2 = var2num[ins["args"][1]];
      }

      if(commutatives.find(op) != commutatives.end()){
        if(current_value.val1 > current_value.val2){
          std::swap(current_value.val1, current_value.val2);
        }
      }
      
      if(value2num.find(current_value) != value2num.end()){
        int val = value2num[current_value];
        var2num[ins["dest"]] = val;
        table[val].second.push(ins["dest"]);
      }else{
        int num = table.size();
        var2num[ins["dest"]] = num;
        value2num[current_value] = num;
        std::queue<std::string> q;
        q.push(ins["dest"]);
        table.push_back(std::make_pair(current_value, q));
      }

    }else if(op == b_const){
      // deal with consts separately
      current_value.op = b_const;
      if(ins["type"].type_name() != "string"){
        current_value.b_type = b_unk;
      }else{
        current_value.b_type = str2type[ins["type"]];
      }

      if(current_value.b_type == b_int
        || current_value.b_type == b_bool){
        current_value.val1 = ins["value"];
        if(value2num.find(current_value) != value2num.end()){
          int val = value2num[current_value];
          var2num[ins["dest"]] = val;
          table[val].second.push(ins["dest"]);
        }else{
          int num = table.size();
          var2num[ins["dest"]] = num;
          value2num[current_value] = num;
          std::queue<std::string> q;
          q.push(ins["dest"]);
          table.push_back(std::make_pair(current_value, q));
        }
      }
    }else if(op == b_call && ins.contains("dest")){
      std::string dest = ins["dest"];
      if(var2num.find(dest) != var2num.end()){
        json tmp_instr;
        std::string tmp_name = dest + var_suffix + std::to_string(counter);
        counter++;
        tmp_instr["op"] = "id";
        tmp_instr["args"].push_back(dest);
        tmp_instr["dest"] = tmp_name;
        tmp_instr["type"] = ins["type"];
        new_instrs.push_back(tmp_instr);
        var2num[tmp_name] = var2num[dest];
        table[var2num["dest"]].second.push(tmp_name);
        var2num.erase(dest);
      }
    }

    // now that we have updated the table, we now need to update the instruction
    json new_instr;

    //don't want to deal with the cases where we don't know the type
    if(current_value.b_type == b_unk){
      new_instr = ins;
      new_instrs.push_back(new_instr);
      continue;
    }
    
    // first, check that the canonical home of the value is not dest.
    // TODO: check for consts and propagate them forward.
    if(value2num.find(current_value) != value2num.end()){
      int idx = value2num[current_value];
      std::queue<std::string>& q = table[idx].second;
      while(!q.empty() && (var2num.find(q.front()) == var2num.end()
      || var2num[q.front()] != idx)) q.pop();
      if(!q.empty() && q.front() != ins["dest"]){
        new_instr["op"] = "id";
        new_instr["args"].push_back(q.front());
        new_instr["dest"] = ins["dest"];
        new_instr["type"] = ins["type"];
        new_instrs.push_back(new_instr);
        continue;
      }
    }

    //now, go through the args.

    new_instr = ins;
    if(ins.contains("args")){
      new_instr["args"] = json::array();
      for(auto arg: ins["args"]){
        int idx = var2num[arg];
        std::queue<std::string>& q = table[idx].second;
        while(!q.empty() && (var2num.find(q.front()) == var2num.end()
        || var2num[q.front()] != idx)) q.pop();
        if(!q.empty() && q.front() != arg){
          new_instr["args"].push_back(q.front());
        }else{
          new_instr["args"].push_back(arg);
        }
      }
    }

    new_instrs.push_back(new_instr);
  }
  return new_instrs;
}

std::string find_suffix(json instrs) {
  size_t max_len = 1;
  for (auto instr : instrs) {
    if(instr.contains("dest")){
      std::string dest = instr["dest"];
      max_len = std::max(dest.length(), max_len);
    }
  }
  return "_unclobber" + std::string("_", max_len + 1);
}
