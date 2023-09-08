#include "helpers.hpp"
#include <iostream>
#include <set>
#include <vector>

using json = nlohmann::json;

int main(){

  // pipes in json as input
  json prog;
  std::cin >> prog;

  json nprog;

  for(auto func : prog["functions"]){
    bool changed = false;
    json new_instrs = func["instrs"];
    std::vector<json> blocks = form_blocks(new_instrs);
    func["instrs"] = json::array();
    for(int i = 0; i < blocks.size(); i++){
      while(true){
        auto res = local_dce_pass(blocks[i]);
        blocks[i] = res.first;
        if(!res.second) break;
      }
    }
    func["instrs"] = json::array();
    for(auto blk: blocks){
      func["instrs"].insert(func["instrs"].end(), blk.begin(), blk.end());
    }
    nprog["functions"].push_back(func);
  }

  std::cout << nprog << std::endl;
}