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

class ConstantPropAnalysis: DataflowBaseSingleton<std::unordered_map<std::string, std::pair<std::optional<ll>, type_t>>>{
public:
    using varmap_t = std::unordered_map<std::string, std::pair<std::optional<ll>, type_t>>;
    
    varmap_t makeTop() override {
        return {};
    }

    std::pair<varmap_t, bool> meet(const CFGNode &node, const std::vector<varmap_t> &influencers, const varmap_t &old) override {
        varmap_t unionMap;
        for(varmap_t inConsts: influencers){
            for(auto it: inConsts){
                if(unionMap.find(it.first) != unionMap.end() 
                    && it.second != unionMap[it.first]){
                        unionMap[it.first] = std::make_pair(std::nullopt, type_t::b_int);
                }else if(unionMap.find(it.first) == unionMap.end()){
                    unionMap[it.first] = it.second;
                }
            }
        }
        return {unionMap, unionMap == old};
    }

    std::pair<varmap_t, bool> transfer(const CFGNode &node, const varmap_t &influence, const varmap_t &old) override {
        varmap_t outMap;
        outMap.insert(influence.begin(), influence.end());

        //TODO: maybe add folding

        //remove things that are in In but not in out

        if(node.instr.contains("dest")){
            if(node.instr["op"] == "const" && (node.instr["type"] == "int"
                || node.instr["type"] == "bool")){
                std::cout << "hi" << std::endl;
                ll val = node.instr["value"];
                type_t type = str2type[node.instr["type"]];
                outMap[node.instr["dest"]] = std::make_pair(std::optional<ll>(val), type);
            }else if(node.instr["op"] == "id" && outMap.find(node.instr["args"][0]) != outMap.end()){
                outMap[node.instr["dest"]] = outMap[node.instr["args"][0]];
            }else if(node.instr["type"].type_name() == "string" && str2type.find(node.instr["type"]) != str2type.end()){
                outMap[node.instr["dest"]] = std::make_pair(std::nullopt, str2type[node.instr["type"]]);
            }
        }

        return {outMap, outMap == old};
    }

    std::string stringify(varmap_t t) override {
        std::vector<std::string> pairs;
        std::transform(t.begin(), t.end(), std::back_inserter(pairs), 
        [](std::pair<std::string, std::pair<std::optional<ll>, type_t>> entry){
            if(!entry.second.first.has_value()){
                return "(" + entry.first + " -\\> (" + "UNDEF" + ", " + type2str[entry.second.second] + "))"; 
            }
            return "(" + entry.first + " -\\> (" + std::to_string(entry.second.first.value_or(0)) + ", " + type2str[entry.second.second] + "))";
        });
        std::sort(pairs.begin(), pairs.end());
        return joinToString<typename std::vector<std::string>::iterator>(pairs.begin(), pairs.end(), "\\{", ", ", "\\}");
    }

};

int main(int argc, char* argv[]) {

    bool smartMode = false;
    bool profileMode = false;
    bool graphMode = false;
    std::string analysis = "none";

    // this loop was basically written by ChatGPT
    // Start from 1 to skip the program name (argv[0])
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s") {
            smartMode = true;
        } else if (arg == "-p") {
            profileMode = true;
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
            if (smartMode) {
                analyzer.smartAnalyze(forwards);
            } else {
                analyzer.naiveAnalyze(forwards);
            }

            if (graphMode) {
                printf("%s\n", analyzer.prettifyBlock().c_str());
            } else {
                printf("%s\n", analyzer.report().c_str());
            }

            if (profileMode) {
                fprintf(stderr, "Total times meet called: %d\n", analyzer.count);
            }

        } else {
            auto analyzer = DataFlowAnalysis<DefinedVarsAnalysis, std::unordered_set<std::string>>(&cfg);
            forwards = true;
            if (smartMode) {
                analyzer.smartAnalyze(forwards);
            } else {
                analyzer.naiveAnalyze(forwards);
            }

            if (graphMode) {
                printf("%s\n", analyzer.prettifyBlock().c_str());
            } else {
                printf("%s\n", analyzer.report().c_str());
            }

            if (profileMode) {
                fprintf(stderr, "Total times meet called: %d\n", analyzer.count);
            }
        
        }        
    }

    if (graphMode) {
        printf("}\n");
    }
}
