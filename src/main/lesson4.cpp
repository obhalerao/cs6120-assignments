#include "../lib/dataflow_analysis.hpp"
#include "../lib/analyses/constant_prop.cpp"
#include "../lib/analyses/defined_vars.cpp"
#include <cstdio>
#include <iostream>

int main(int argc, char* argv[]) {

    std::string analyzeMode = "naive";
    bool profileMode = false;
    bool graphMode = false;
    std::string analysis = "none";

    // this loop was basically written by ChatGPT
    // Start from 1 to skip the program name (argv[0])
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s") {
            analyzeMode = "smart";
        } else if (arg == "-p") {
            profileMode = true;
        } else if (arg == "-w") {
            analyzeMode = "worklist";
        } else if (arg == "-g") {
            graphMode = true;
        } else if (arg == "-a" && i + 1 < argc) {
            analysis = argv[i + 1];
        }
    }

    json prog;
    std::cin >> prog;
    auto func = prog["functions"][0];

    if (graphMode) {
        printf("digraph {\n");
    }

    for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        
        // DataFlowAnalysisBase analyzer;
        bool forwards;
        if (analysis == "cprop") {
            auto analyzer = DataFlowAnalysis<ConstantPropAnalysis, std::unordered_map<std::string, std::pair<std::optional<ll>, type_t>>>(&cfg);
            forwards = true;
            if (analyzeMode == "smart") {
                analyzer.smartAnalyze(forwards);
            } else if (analyzeMode == "worklist") {
                analyzer.worklistAnalyze(forwards);
            } else {
                analyzer.naiveAnalyze(forwards);
            }

            if (graphMode) {
                printf("%s\n", analyzer.prettifyBlock().c_str());
            } else {
                printf("%s\n", analyzer.report().c_str());
            }

            if (profileMode) {
                fprintf(stderr, "Total times meet called for function %s: %d\n", func["name"].get<std::string>().c_str(), analyzer.count);
            }

        } else {
            auto analyzer = DataFlowAnalysis<DefinedVarsAnalysis, std::unordered_set<std::string>>(&cfg);
            forwards = true;
            if (analyzeMode == "smart") {
                analyzer.smartAnalyze(forwards);
            } else if (analyzeMode == "worklist") {
                analyzer.worklistAnalyze(forwards);
            } else {
                analyzer.naiveAnalyze(forwards);
            }

            if (graphMode) {
                printf("%s\n", analyzer.prettifyBlock().c_str());
            } else {
                printf("%s\n", analyzer.report().c_str());
            }

            if (profileMode) {
                fprintf(stderr, "Total times meet called for function %s: %d\n", func["name"].get<std::string>().c_str(), analyzer.count);
            }
        
        }        
    }

    if (graphMode) {
        printf("}\n");
    }
}
