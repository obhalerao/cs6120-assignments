#include <iostream>

#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"

int main(int argc, char* argv[]){
  CFGNode node;

  json prog;
  std::cin >> prog;

  std::cout << prog;
}