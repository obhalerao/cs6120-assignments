#include "helpers.hpp"

int main(){

  // pipes in json as input
  json prog;
  std::cin >> prog;

  json nprog;

  for(auto func: prog["functions"]){
    json instrs = func["instrs"];
    std::vector<json> blocks = form_blocks(instrs);
    for(int i = 0; i < blocks.size(); i++){
      json new_block = lvn_pass(blocks[i]);
      blocks[i] = new_block;
    }
    while(true){
      auto res = global_dce_pass(blocks);
      blocks = res.first;
      if(!res.second) break;
    }
    for(int i = 0; i < blocks.size(); i++){
      while(true){
        auto res = local_dce_pass(blocks[i]);
        blocks[i] = res.first;
        if(!res.second) break;
      }
    }
    func["instrs"] = json::array();
    for(auto block: blocks){
      func["instrs"].insert(func["instrs"].end(), block.begin(), block.end());
    }
    nprog["functions"].push_back(func);
  }

  std::cout << nprog << std::endl;

}