#include "json.hpp"
#include "form_blocks.hpp"
#include <iostream>
#include <set>
#include <vector>

using json = nlohmann::json;

std::pair<json, bool> run_dce_pass(json& blk){
  json new_json;
  bool changed = false;
  std::set<std::string> total_args;
  for(auto instr: blk){
    if(instr.contains("args")){
      for(auto arg: instr["args"]) total_args.insert(arg);
    }
  }
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
  return std::make_pair(new_json, changed);
}

int main(){

  // pipes in json as input
  json prog;
  std::cin >> prog;

  for(auto func : prog["functions"]){
    bool changed = false;
    std::vector<json> blocks = form_blocks(func["instrs"]);
    json new_lst(json::value_t::array);
    for(auto blk: blocks){
      json new_blk = blk;
      while(true){
        auto result = run_dce_pass(new_blk);
        if(!result.second) break;
        new_blk = result.first;
      }
      new_lst.insert(new_lst.end(), new_blk.begin(), new_blk.end());
    }
    func["instrs"] = new_lst;
  }

  std::cout << prog << std::endl;
}