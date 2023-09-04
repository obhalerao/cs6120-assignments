#include "json.hpp"
#include <iostream>

using json = nlohmann::json;

int main(){

  // pipes in json as input
  json prog;
  std::cin >> prog;
  std::cout << prog;

}