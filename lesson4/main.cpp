#include "dataflow_analysis.hpp"
#include <cstdio>

class DefinedVarsAnalysis : DataflowBaseSingleton<std::unordered_set<std::string>> {
public:
    using vars_t = std::unordered_set<std::string>;
    vars_t makeTop() override {
        return {};
    }

    std::pair<vars_t, bool> meet(const CFGNode &node, const std::vector<vars_t> &influencers, const vars_t &old) override {
        vars_t unionSet(old);
        for (auto inDef : influencers) {
            unionSet.insert(inDef.begin(), inDef.end());
        }
        return {unionSet, unionSet == old};
    };

    std::pair<vars_t, bool> transfer(const CFGNode &node, const vars_t &influence, const vars_t &old) override {
        vars_t outSet(old);
        outSet.insert(influence.begin(), influence.end());

        if(node.instr.contains("dest") && outSet.find(node.instr["dest"]) == outSet.end()) {
            outSet.insert(node.instr["dest"]);
        }
        return {outSet, outSet == old};
    }
};

int main() {
    json prog;
    std::cin >> prog;
    auto func = prog["functions"][0];
    for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DataFlowAnalysis<DefinedVarsAnalysis, std::unordered_set<std::string>>(&cfg);
        analyzer.naiveAnalyze(true);

        printf("func %s\n", func["name"].dump().c_str());
        printf("%ld nodes\n", cfg.nodes.size());

        for (auto block : cfg.blocks) {
            auto &inSet = analyzer.nodeIn[block.nodes.front()];
            auto &outSet = analyzer.nodeOut[block.nodes.back()];
            std::string prettySetIn = joinToString(inSet.begin(), inSet.end(), std::string("{"), std::string("}"), std::string(", "));
            std::string prettySetOut = joinToString(outSet.begin(), outSet.end(), std::string("{"), std::string("}"), std::string(", "));
            
            printf("block %s\n", block.blockName.c_str());
            printf("defined in\n");
            printf("%s\n", prettySetIn.c_str());
            printf("defined out\n");
            printf("%s\n", prettySetOut.c_str());
        }
        printf("%s\n", cfg.prettify().c_str());
    }
    printf("\n");
}
