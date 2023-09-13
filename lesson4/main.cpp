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

    std::string stringify_old(vars_t t) {
        // need to escape the braces to display properly in dot
        return joinToString<typename std::unordered_set<std::string>::iterator>(t.begin(), t.end(), "\\{", ", ", "\\}");
    }

    std::string stringify(vars_t t) override {
        // need to escape the braces to display properly in dot
        std::vector<std::string> help;
        for (auto i : t) {
            help.push_back(i);
        }
        std::sort(help.begin(), help.end());
        return joinToString<typename std::vector<std::string>::iterator>(help.begin(), help.end(), "\\{", ", ", "\\}");
    }
};

int main() {
    json prog;
    std::cin >> prog;
    auto func = prog["functions"][0];
    for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DataFlowAnalysis<DefinedVarsAnalysis, std::unordered_set<std::string>>(&cfg);
        analyzer.smartAnalyze(true);

        printf(
            "%s\n",
            cfg.prettifyNodes(
                [](int i, CFG* cfg){
                    std::string instr = instr_to_string(cfg->nodes[i].instr);
                    return string_format("label=\"%s\"", instr.c_str());
                }
            ).c_str()
            );
        printf(
            "%s\n",
            analyzer.prettifyBlock().c_str()
        );
    }
    printf("\n");
}
