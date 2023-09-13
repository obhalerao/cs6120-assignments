#include <utility>
#include <vector>
#include <algorithm>
#include "json.hpp"
#include "helpers.cpp"
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <memory>
#include <string>
#include <stdexcept>

// from https://stackoverflow.com/a/26221725
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

using json = nlohmann::json;


std::string lower(std::string data) {
    std::string ret = std::string(data);
    std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c){ return std::tolower(c); });
    return ret;
}


std::string value_to_string(json type, json value) {
    const std::unordered_map<int, std::string> control_chars {
        {0, std::string("\\0")},
        {7, std::string("\\a")},
        {8, std::string("\\b")},
        {9, std::string("\\t")},
        {10, std::string("\\n")},
        {11, std::string("\\v")},
        {12, std::string("\\f")},
        {13, std::string("\\r")},
    };

    if (!type.is_object()) {
        auto type_str = std::string(type);
        std::string lowered = lower(type_str);
        if (lowered == "char") {
            std::string value_str = value.get<std::string>();
            uint32_t value_int = value_str.at(0);
            auto search = control_chars.find(value_int);
            if (search != control_chars.end()) {
                std::string ret = std::string("'") + search->second + std::string("'");
                return ret;
            }
        }
    }

    std::string ret = lower(std::to_string(value.get<uint32_t>()));
    return ret;
}

std::string type_to_string(json type) {
    if (type.is_object()) {
        for (auto& el : type.items()) {
            return std::string(el.key()) + "<" + type_to_string(el.value()) + ">";
        }
        return "type_to_string_failed";
    }
    else {
        return type;
    }
}

std::string instr_to_string(json instr) {
    if (instr.contains("op") && instr["op"] == "const") {
        std::string dest_str = instr["dest"];
        std::string tyann = (instr.contains("type")) ? (": " + type_to_string(instr["type"])) : "";
        

        auto dest = instr["dest"].get<std::string>();
        std::string value = value_to_string(instr["type"], instr["value"]);

        return dest + tyann + " = const " + value;
    }
    else {
        std::string rhs = instr["op"];
        if (instr.contains("funcs")) {
            for (auto func : instr["funcs"]) {
                rhs += "@" + func.get<std::string>();
            }
        }
        if (instr.contains("args")) {
            for (auto arg : instr["args"]) {
                rhs += " " + arg.get<std::string>();
            }
        }
        if (instr.contains("labels")) {
            for (auto label : instr["labels"]) {
                rhs += " ." + label.get<std::string>();
            }
        }
        if (instr.contains("dest")) {
            std::string tyann = (instr.contains("type")) ? (": " + type_to_string(instr["type"])) : "";
            auto dest = instr["dest"].get<std::string>();
            return dest + tyann + " = " + rhs;
        } else {
            return rhs;
        }
    }
}

class CFGNode {
public:
    json instr;
    std::vector<int> preds;
    std::vector<int> succs;
    CFGNode() = default;

    CFGNode(json instr) : instr(instr) {
    }

    CFGNode(const CFGNode &cfgNode) : instr(cfgNode.instr), preds(cfgNode.preds), succs(cfgNode.succs) {
    }
};

class CFGBlock {
public:
    std::vector<int> nodes;
    std::vector<int> preds;
    std::vector<int> succs;
    std::string blockName;

    CFGBlock(std::string blockName, std::vector<int> nodes) : blockName(blockName), nodes(nodes) {
    }

    CFGBlock(const CFGBlock &cfgBlock) : nodes(cfgBlock.nodes), preds(cfgBlock.preds), succs(cfgBlock.succs), blockName(cfgBlock.blockName){
    }

};

class CFG {
public:
    std::string funcName;
    std::vector<CFGNode> nodes;
    std::vector<CFGBlock> blocks;
    std::unordered_map<std::string, int> blockOfString;

    CFG(std::string funcName, json& instrs) : funcName(funcName) {
        std::string prefix = "anti_clobber_label";
        PrefixCounter pc(prefix);
        std::string firstLabel = pc.generate();

        // std::vector<CFGNode> nodes; // all instructions other than labels
        std::vector<uint32_t> blockStarts;  // first instruction in ith block
        std::vector<std::string> labels; // label for ith block

        blockStarts.push_back(0);
        labels.push_back(firstLabel);

        bool unreachable = false;  // TODO: put these blocks back in and leave them disconnected

        for (auto instr : instrs) {
            if (unreachable && !instr.contains("label")) continue;

            unreachable = false;
            if (instr.contains("label")) {
                if (blockStarts.size() > 0 && blockStarts.back() == nodes.size())
                    nodes.emplace_back(json({{"op", "nop"}})); // make this block non-empty
                blockStarts.push_back(nodes.size());
                labels.push_back(instr["label"]);
            } else if (instr.contains("op") && instr["op"] == "jmp" || instr["op"] == "br") {
                nodes.emplace_back(instr);
                unreachable = true; // instructions immediately after an unconditional jump, or a branch with two labels, are unreachable
            } else {
                nodes.emplace_back(instr);
            }
        }

        for (int i = 0; i < blockStarts.size(); i++) {
            uint32_t end = (i < blockStarts.size() - 1) ? blockStarts[i + 1] : nodes.size();
            for (uint32_t j = blockStarts[i]; j < end - 1; j++) {
                nodes[j + 1].preds.push_back(j);
                nodes[j].succs.push_back(j + 1);
            }

            std::vector<int> nodePtrs;
            for (uint32_t j = blockStarts[i]; j < end; j++) {
                nodePtrs.push_back(j);
            }
            blocks.emplace_back(labels[i], nodePtrs);
        }

        auto numBlocks = blocks.size();

        for (int i = 0; i < numBlocks; i++) {
            blockOfString[blocks[i].blockName] = i;
        }

        for (int i = 0; i < numBlocks; i++) {
            auto &block = blocks[i];
            json instr = nodes[block.nodes.back()].instr;
            if (instr.contains("op") && (instr["op"] == "jmp" || instr["op"] == "br")) {
                for (std::string label : instr["labels"]) {
                    block.succs.push_back(blockOfString[label]);
                    blocks[blockOfString[label]].preds.push_back(i);
                }
            }
            else if (i < numBlocks - 1) {
                block.succs.push_back(i + 1);
                blocks[i + 1].preds.push_back(i);
            }
        }

        for (auto block : blocks) {
            std::vector<int> &blockPreds = nodes[block.nodes.front()].preds;
            for (auto predBlockPtr : block.preds) {
                (blockPreds).push_back((blocks[predBlockPtr]).nodes.back());
            }

            std::vector<int> &blockSuccs = nodes[block.nodes.back()].succs;
            for (auto succBlockPtr : block.succs) {
                blockSuccs.push_back((blocks[succBlockPtr]).nodes.front());
            }
        }
    }

    json toInstrs() {
        json instrs;
        for (auto block : blocks) {
            json inner;
            inner["label"] = block.blockName;
            instrs.push_back(inner);
            for (auto nodePtr : block.nodes) {
                instrs.emplace_back(nodes[nodePtr].instr);
            }
        }
        return instrs;
    };

    std::string prettify() {
        std::string nodeStr;
        for (int i = 0; i < nodes.size(); i++) {
            nodeStr.append(string_format("node_%d [label=\"%s\"]\n", i, instr_to_string(nodes[i].instr).c_str()));
        }
        std::string edgeStr;
        for (int i = 0; i < nodes.size(); i++) {
            for (auto j : nodes[i].succs) {
                edgeStr.append(string_format("node_%d -> node_%d\n", i, j));
            }
        }

        std::string ans = string_format("digraph %s {\n%s}\n", funcName.c_str(), (nodeStr + edgeStr).c_str());
        return ans;
    }

};

template<typename T>
class ForwardDataflowBaseSingleton {
public:
    virtual T makeTop() = 0;

    virtual std::pair<T, bool> inCalc(const CFGNode &node, const std::vector<T> &outs, const T &oldIn) = 0;

    virtual std::pair<T, bool> outCalc(const CFGNode &node, const T &inNew, const T &outOld) = 0;
};

template<typename T> class DataflowBaseSingleton {
public:
    virtual T makeTop() = 0;

    virtual std::pair<T, bool> meet(const CFGNode &node, const std::vector<T> &influencers, const T &old) = 0;

    virtual std::pair<T, bool> transfer(const CFGNode &node, const T &influence, const T &old) = 0;
};

template<class DataFlowImpl, typename K>
class DataFlowAnalysis {
public:
    CFG *cfg;
    std::vector<K> nodeIn;
    std::vector<K> nodeOut;
    DataFlowImpl factory;

    explicit DataFlowAnalysis(CFG *cfg) : factory(), cfg(cfg) {
        static_assert(std::is_base_of<DataflowBaseSingleton<K>, DataFlowImpl>::value, "derived from wrong class??");
    }

    void naiveAnalyze(bool forwards) {
        nodeIn.clear();
        nodeOut.clear();
        for (int i = 0; i < cfg->nodes.size(); i++) {
            nodeIn.push_back(factory.makeTop());
            nodeOut.push_back(factory.makeTop());
        }

        bool changed = true;
        uint32_t count = 0;
        while (changed) {
            changed = false;
            for (int i = 0; i < cfg->nodes.size(); i++) {
                count++;
                auto node = cfg->nodes[i];

                std::vector<K> preds;
                for (auto predNodePtr: (forwards ? node.preds : node.succs)) {
                    preds.push_back(nodeOut[predNodePtr]);
                }

                auto inRes = factory.meet(node, preds, nodeIn[i]);
                auto outRes = factory.transfer(node, inRes.first, nodeOut[i]);

                nodeIn[i] = inRes.first;
                nodeOut[i] = outRes.first;
                if (!inRes.second || !outRes.second) {
                    changed = true;
                }
            }
        }
    }

};
