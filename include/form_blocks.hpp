#include "json.hpp"
#include <vector>
#include <iostream>

using json = nlohmann::json;

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