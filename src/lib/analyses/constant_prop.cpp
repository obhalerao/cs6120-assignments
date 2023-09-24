#include "../dataflow_analysis.hpp"
#include "../cfg_utils.cpp"

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
                ll val = node.instr["value"];
                type_t type = str2type[node.instr["type"]];
                outMap[node.instr["dest"]] = std::make_pair(std::optional<ll>(val), type);
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
                return "(" + entry.first + " -\\> (" + "MULT_DEF" + ", " + type2str[entry.second.second] + "))"; 
            }
            return "(" + entry.first + " -\\> (" + std::to_string(entry.second.first.value_or(0)) + ", " + type2str[entry.second.second] + "))";
        });
        std::sort(pairs.begin(), pairs.end());
        return joinToString<typename std::vector<std::string>::iterator>(pairs.begin(), pairs.end(), "\\{", ", ", "\\}");
    }

};