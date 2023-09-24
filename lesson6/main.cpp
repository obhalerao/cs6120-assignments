#include <iostream>

#include "dominator_analysis_temp.hpp"

#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"

int main(int argc, char* argv[]){
  json prog;
  std::cin >> prog;

  bool profileMode = false;
  bool naiveMode = false;

  for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-p") {
          profileMode = true;
      } else if (arg == "-n") {
          naiveMode = true;
      }
  }

  for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DominatorAnalysis(&cfg);
        if(naiveMode){
            analyzer.naiveAnalyze();
        }else{
            analyzer.smartAnalyze();
        }
        printf("%s\n", analyzer.report().c_str());
        printf("%s\n", analyzer.report_natural_loops().c_str());
        if (profileMode) {
            fprintf(stderr, "Total function calls for function %s when computing dominators: %d\n", func["name"].get<std::string>().c_str(), analyzer.count);
        }
    }
}