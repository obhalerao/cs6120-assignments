#include "json.hpp"
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
      for(std::string arg: instr["args"]) total_args.insert(arg);
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

  json nprog;

  for(auto func : prog["functions"]){
    bool changed = false;
    json new_instrs = func["instrs"];
    while(true){
      auto result = run_dce_pass(new_instrs);
      if(!result.second) break;
      new_instrs = result.first;
    }
    func["instrs"] = new_instrs;
    nprog["functions"].push_back(func);
  }

  std::cout << nprog << std::endl;
}