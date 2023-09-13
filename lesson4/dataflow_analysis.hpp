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
        if (blockStarts.back() == nodes.size()) {
            nodes.emplace_back(json({{"op", "nop"}}));
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

    std::string prettifyNodes(std::string (*f)(int, CFG*)) {
        std::string nodeStr;
        for (int i = 0; i < nodes.size(); i++) {
            nodeStr.append(string_format("node_%d [%s];\n", i, f(i, this).c_str()));
        }
        std::string edgeStr;
        for (int i = 0; i < nodes.size(); i++) {
            for (auto j : nodes[i].succs) {
                edgeStr.append(string_format("node_%d -> node_%d;\n", i, j));
            }
        }

        std::string ans = string_format("digraph %s_nodes {\n%s}\n", funcName.c_str(), (nodeStr + edgeStr).c_str());
        return ans;
    }

    template<typename T> std::string prettifyBlocks(std::string (*f)(int, CFG*, T), T t) {
        std::string nodeStr;
        for (int i = 0; i < blocks.size(); i++) {
            nodeStr.append(string_format("node_%d [%s]\n", i, f(i, this, t).c_str()));
        }
        std::string edgeStr;
        for (int i = 0; i < blocks.size(); i++) {
            for (auto j : blocks[i].succs) {
                edgeStr.append(string_format("node_%d -> node_%d\n", i, j));
            }
        }

        std::string ans = string_format("digraph %s_blocks {\n%s}\n", funcName.c_str(), (nodeStr + edgeStr).c_str());
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

    virtual std::string stringify(T t) = 0;
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

    std::string prettifyBlockIn() {
        return cfg->prettifyBlocks<std::vector<K>>(
            [](int i, CFG *cfg, std::vector<K> nodeIn) {
                auto &blockNodes = cfg->blocks[i].nodes;
                auto &nodes = cfg->nodes;

                std::vector<std::string> nodeStrings;
                for (auto it = blockNodes.begin(); it != blockNodes.end(); it++) {
                    nodeStrings.push_back(string_format("{%s}", instr_to_string(nodes[*it].instr).c_str()));
                }
                
                return string_format(
                    "shape=record, label=\"{{%s}|{}|%s}\"",
                    DataFlowImpl().stringify(nodeIn[blockNodes.front()]).c_str(),
                    joinToString(nodeStrings.begin(), nodeStrings.end(), "", "|", "").c_str()
                );
            },
            nodeIn
        );
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
