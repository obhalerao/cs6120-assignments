#include "json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

// Counts the number of branches in the JSON representation of a bril program.

int main(int argc, char* argv[]){

  // pipes in json as input
  json data;
  std::cin >> data;

  int branches {0};

  for(auto function: data["functions"]){
    for(auto instr: function["instrs"]){
      if(instr.contains("op") && instr["op"] == "br") branches++;
    }
  }

  std::cout << "This program has " << branches << " branches." << std::endl;
}