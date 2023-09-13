#include "dataflow_analysis.hpp"
#include <cstdio>

typedef long long ll;

enum class type_t {
    b_int,
    b_bool
};

std::unordered_map<type_t, std::string> type2str = {
    {type_t::b_int, "int"},
    {type_t::b_bool, "bool"}
};

std::unordered_map<std::string, type_t> str2type = {
    {"int", type_t::b_int},
    {"bool", type_t::b_bool}
};

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
        // need to escape the braces to display properly in dot
        return joinToString<typename std::unordered_set<std::string>::iterator>(t.begin(), t.end(), "\\{", ", ", "\\}");
    }
};

class ConstantPropAnalysis: DataflowBaseSingleton<std::unordered_map<std::string, std::pair<ll, type_t>>>{
public:
    using varmap_t = std::unordered_map<std::string, std::pair<ll, type_t>>;

    varmap_t makeTop() override {
        return {};
    }

    std::pair<varmap_t, bool> meet(const CFGNode &node, const std::vector<varmap_t> &influencers, const varmap_t &old) override {
        varmap_t unionMap(old);
        for(varmap_t inConsts: influencers){
            for(auto it: inConsts){
                if(unionMap.find(it.first) != unionMap.end() 
                    && it.second != unionMap[it.first]){
                        unionMap.erase(it.first);
                }else if(unionMap.find(it.first) == unionMap.end()){
                    unionMap[it.first] = it.second;
                }
            }
        }
        return {unionMap, unionMap == old};
    }

    std::pair<varmap_t, bool> transfer(const CFGNode &node, const varmap_t &influence, const varmap_t &old) override {
        varmap_t outMap(old);
        outMap.insert(influence.begin(), influence.end());

        //TODO: maybe add folding

        if(node.instr.contains("dest")){
            if(node.instr["op"] == "const"){
                ll val = node.instr["value"];
                type_t type = str2type[node.instr["type"]];
                outMap[node.instr["dest"]] = std::make_pair(val, type);
            }else if(node.instr["op"] == "id" && outMap.find(node.instr["args"][0]) != outMap.end()){
                outMap[node.instr["dest"]] = outMap[node.instr["args"][0]];
            }
        }

        return {outMap, outMap == old};
    }

    std::string stringify(varmap_t t) override {
        std::vector<std::string> pairs;
        std::transform(t.begin(), t.end(), std::back_inserter(pairs), 
        [](std::pair<std::string, std::pair<ll, type_t>> entry){
            return "(" + entry.first + " -\\> (" + std::to_string(entry.second.first) + ", " + type2str[entry.second.second] + "))";
        });
        return joinToString<typename std::vector<std::string>::iterator>(pairs.begin(), pairs.end(), "\\{", ", ", "\\}");
    }

};

int main() {
    json prog;
    std::cin >> prog;
    auto func = prog["functions"][0];
    for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DataFlowAnalysis<ConstantPropAnalysis, std::unordered_map<std::string, std::pair<ll, type_t>>>(&cfg);
        analyzer.naiveAnalyze(true);

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
            analyzer.prettifyBlockIn().c_str()
        );
    }
    printf("\n");
}
