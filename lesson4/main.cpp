#include "dataflow_analysis.hpp"
#include <cstdio>

//class CPValue {
//public:
//    using forward_map_t = std::unordered_map<std::string, std::string>;
//    using backward_map_t = std::unordered_map<std::string, std::unordered_set<std::string>>;
//    forward_map_t mapForwards;
//    backward_map_t mapBackwards;
//
//    static CPValue Top() {
//        return {};
//    }
//
//
//private:
//    static std::pair<forward_map_t, backward_map_t> deepCopyMaps(const forward_map_t &forwards, const backward_map_t &backwards) {
//        forward_map_t retForwards;
//        backward_map_t retBackwards;
//
//        for (const auto& entry : forwards) {
//            retForwards[entry.first] = entry.second;
//        }
//
//        for (const auto& entry : backwards) {
//            auto toPut = std::unordered_set<std::string>();
//            for (const auto& string : entry.second) {
//                toPut.insert(string);
//            }
//            retBackwards[entry.first] = toPut;
//        }
//        return {retForwards, retBackwards};
//    }
//
//    static CPValue deepCopy(CPValue c) {
//        auto pair = CPValue::deepCopyMaps(c.mapForwards, c.mapBackwards);
//        return {pair.first, pair.second};
//    }
//
//
//};
//
//class CopyPropAnalysis : ForwardDataflowBaseSingleton<CPValue> {
//public:
//    static CPValue makeTop() override {
//        return CPValue();
//    }
//};


class DefinedVarsAnalysis : ForwardDataflowBaseSingleton<std::unordered_set<std::string>> {
public:
    using vars_t = std::unordered_set<std::string>;
    vars_t makeTop() override {
        return {};
    }

    std::pair<vars_t, bool> inCalc(const CFGNode &node, const std::vector<vars_t> &outs, const vars_t &oldIn) override {
        vars_t unionSet(oldIn);
        for (auto inDef : outs) {
            unionSet.insert(inDef.begin(), inDef.end());
        }
        return {unionSet, unionSet == oldIn};
    };

    std::pair<vars_t, bool> outCalc(const CFGNode &node, const vars_t &inNew, const vars_t &outOld) override {
        vars_t outSet(outOld);
        outSet.insert(inNew.begin(), inNew.end());

        if(node.instr.contains("dest") && outSet.find(node.instr["dest"]) == outSet.end()) {
            outSet.insert(node.instr["dest"]);
        }
        return {outSet, outSet == outOld};
    }
};

int main() {
    json prog;
    std::cin >> prog;
    auto func = prog["functions"][0];
    for(auto func : prog["functions"]){
        CFG cfg(func["instrs"]);
        auto analyzer = DataFlowAnalysis<DefinedVarsAnalysis, std::unordered_set<std::string>>(&cfg);
        analyzer.naiveForwardAnalysis();

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
    }
    printf("\n");
}