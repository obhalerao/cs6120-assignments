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

    std::string stringify(vars_t t) override {
        return joinToString<typename std::unordered_set<std::string>::iterator>(t.begin(), t.end(), "\\{", ", ", "\\}");
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

        // printf("func %s\n", func["name"].dump().c_str());
        // printf("%ld nodes\n", cfg.nodes.size());

        printf(
            "%s\n",
            cfg.prettifyNodes(
                [](int i, CFG* cfg){
                    std::string instr = instr_to_string(cfg->nodes[i].instr);
                    return string_format("label=\"%s\"", instr.c_str());
                }
            ).c_str()
            );
        // printf(
        //     "%s\n",
        //     cfg.prettifyBlocks<int>(
        //         [](int i, CFG* cfg, int dumbass) {
        //             auto &blockNodes = cfg->blocks[i].nodes;
        //             auto &nodes = cfg->nodes;

        //             std::vector<std::string> nodeStrings;
        //             for (auto it = blockNodes.begin(); it != blockNodes.end(); it++) {
        //                 nodeStrings.push_back(string_format("{%s}", instr_to_string(nodes[*it].instr).c_str()));
        //             }
                    
        //             return string_format(
        //                 "shape=record, label=\"%s\"", 
        //                 joinToString(nodeStrings.begin(), nodeStrings.end(), "{", "|", "}").c_str()
        //             );
        //         },
        //         0
        //     ).c_str()
        // );
        printf(
            "%s\n",
            analyzer.prettifyBlockIn().c_str()
        );
    }
    printf("\n");
}
