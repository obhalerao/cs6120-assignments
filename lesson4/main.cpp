#include "dataflow_analysis.hpp"
#include <cstdio>

int main() {
    json prog;
    std::cin >> prog;
    for(auto func : prog["functions"]){
        CFG cfg(func["instrs"]);
    }
}