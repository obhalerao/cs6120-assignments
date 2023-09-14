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
#include <deque>
#include <optional>

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

    CFGBlock(std::string blockName, std::vector<int> nodes) : blockName(blockName), nodes(nodes) {}

    CFGBlock(const CFGBlock &cfgBlock) : nodes(cfgBlock.nodes), preds(cfgBlock.preds), succs(cfgBlock.succs), blockName(cfgBlock.blockName){}

};

class CFG {
public:
    std::string funcName;
    std::vector<CFGNode> nodes;
    std::vector<CFGBlock> blocks;
    std::unordered_map<std::string, int> blockOfString;

    CFG(std::string funcName, json& instrs) : funcName(funcName) {
        std::string prefix = "unreachable_anti_clobber_label";
        PrefixCounter pc(prefix);
        std::string firstLabel = "start_of_function_anti_clobber_label";

        std::vector<uint32_t> blockStarts;  // first instruction in ith block
        std::vector<std::string> labels; // label for ith block

        blockStarts.push_back(0);
        labels.push_back(firstLabel);

        // TODO: end block on ret instruction, maybe always put a nop at the end of the function and point rets there?
        bool unreachable = false;  // TODO: decide whether to put these blocks back in and leave them disconnected from the rest of the function

        for (auto instr : instrs) {
            if (unreachable && !instr.contains("label")) {
                // create a label for this unreachable code
                if (blockStarts.size() > 0 && blockStarts.back() == nodes.size()) {
                    nodes.emplace_back(json({{"op", "nop"}})); // make previous block non-empty
                }
                blockStarts.push_back(nodes.size());
                labels.push_back(pc.generate());
                unreachable = false;
            }

            if (instr.contains("label")) {
                if (blockStarts.size() > 0 && blockStarts.back() == nodes.size())
                    nodes.emplace_back(json({{"op", "nop"}})); // make previous block non-empty
                blockStarts.push_back(nodes.size());
                labels.push_back(instr["label"]);
                unreachable = false;
            } else if (instr.contains("op") && (instr["op"] == "jmp" || instr["op"] == "br" || instr["op"] == "ret")) {
                nodes.emplace_back(instr);
                unreachable = true; // instructions immediately after the end of a basic block are unreachable
            } else {
                nodes.emplace_back(instr);
            }
        }
        if (blockStarts.back() == nodes.size()) {
            nodes.emplace_back(json({{"op", "nop"}})); // make last block non-empty
        }

        auto numBlocks = blockStarts.size();

        for (int i = 0; i < numBlocks; i++) {
            uint32_t end = (i < numBlocks - 1) ? blockStarts[i + 1] : nodes.size();
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

        // populate predecessors and successors in neighboring blocks
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

    // would be used after mutating the nodes e.g. in copy propagation
    json toInstrs() {
        json instrs;
        for (auto block : blocks) {
            json label;
            label["label"] = block.blockName;
            instrs.push_back(label);
            for (auto nodePtr : block.nodes) {
                instrs.emplace_back(nodes[nodePtr].instr);
            }
        }
        return instrs;
    };

    std::string prettifyNodes(std::string (*f)(int, CFG*)) {
        std::string nodeStr;
        for (int i = 0; i < nodes.size(); i++) {
            nodeStr.append(string_format("%s_node_%d [%s];\n", funcName.c_str(), i, f(i, this).c_str()));
        }
        std::string edgeStr;
        for (int i = 0; i < nodes.size(); i++) {
            for (auto j : nodes[i].succs) {
                edgeStr.append(string_format("%s_node_%d -> %s_node_%d;\n", funcName.c_str(), i, funcName.c_str(), j));
            }
        }

        std::string ans = string_format("subgraph cluster_%s_nodes {\n%s}\n", funcName.c_str(), (nodeStr + edgeStr).c_str());
        return ans;
    }

    // T is a catch-all for any additional args you may want to pass into the printer
    template<typename T> std::string prettifyBlocks(std::string (*f)(int, CFG*, T), T t) {
        std::vector<std::string> lines;
        lines.push_back(string_format("label = \"%s\"", funcName.c_str()));
        for (int i = 0; i < blocks.size(); i++) {
            lines.push_back(string_format("%s_block_%d [%s]", funcName.c_str(), i, f(i, this, t).c_str()));
        }
        for (int i = 0; i < blocks.size(); i++) {
            for (auto j : blocks[i].succs) {
                lines.push_back(string_format("%s_block_%d -> %s_block_%d", funcName.c_str(), i, funcName.c_str(), j));
            }
        }

        auto lines_str = joinToString<std::vector<std::string>::iterator>(lines.begin(), lines.end(), "", "\n\t", "");

        std::string ans = string_format("subgraph cluster_%s_blocks {\n\t%s\n}\n", funcName.c_str(), lines_str.c_str());
        return ans;
    }

    void populate_dfs() {
        int i = 0;
        std::unordered_map<int, int> index;
        std::unordered_map<int, int> lowlink;
        std::unordered_set<int> stackSet;
        std::deque<int> stack;
        std::deque<std::pair<int, int>> callStack;
        std::vector<std::vector<int>> SCCs;

        for (int v = 0; v < nodes.size(); v++) {
            if (index.find(v) == index.end()) {
                callStack.push_back({v, 0});
                while (!callStack.empty()) {
                    std::pair<int, int> pair = callStack.back();
                    callStack.pop_back();
                    auto v = pair.first;
                    auto pi = pair.second;
                    auto &neighbors = nodes[v].succs;

                    if (pi == 0) {
                        index[v] = i;
                        lowlink[v] = i;
                        i += 1;
                        stack.push_back(v);
                        stackSet.insert(v);
                    } else if (pi > 0) {
                        int prev = neighbors[pi - 1];
                        lowlink[v] = std::min(lowlink[v], lowlink[prev]);
                    }

                    while (pi < neighbors.size() && index.find(neighbors[pi]) != index.end()) {
                        int w = neighbors[pi];
                        if (stackSet.find(w) != stackSet.end()) {
                            lowlink[v] = std::min(lowlink[v], index[w]);
                        }
                        pi++;
                    }

                    if (pi < neighbors.size()) {
                        int w = neighbors[pi];
                        callStack.push_back({v, pi + 1});
                        callStack.push_back({w, 0});
                        continue;
                    }

                    if (lowlink[v] == index[v]) {
                        std::vector<int> scc;
                        bool keep_going = true;
                        while (keep_going) {
                            int w = stack.back();
                            stack.pop_back();
                            stackSet.erase(w);
                            scc.push_back(w);
                            keep_going = w != v;
                        }
                        SCCs.push_back(scc);
                    }

                }
            }
        }
        dfs_results = SCCs;
    }

    std::vector<std::vector<int>> dfs() {
        if (!dfs_results.has_value()) populate_dfs();
        return dfs_results.value();
    }

private:
    std::optional<std::vector<std::vector<int>>> dfs_results;

};

template<typename T> class DataflowBaseSingleton {
public:
    virtual T makeTop() = 0;

    virtual std::pair<T, bool> meet(const CFGNode &node, const std::vector<T> &influencers, const T &old) = 0;

    virtual std::pair<T, bool> transfer(const CFGNode &node, const T &influence, const T &old) = 0;

    virtual std::string stringify(T t) = 0;
};

class DataFlowAnalysisBase {
public:
    virtual std::string report() { return "dummy-impl"; }

    virtual std::string prettifyBlock() { return "dummy-impl"; }

    virtual void smartAnalyze(bool forwards) { printf("passing\n"); }

    virtual void naiveAnalyze(bool forwards) { printf("passing\n"); }
};

class DataFlowAnalysisAbstract : public DataFlowAnalysisBase {
public:
    virtual std::string report() override = 0;

    virtual std::string prettifyBlock() override = 0;

    virtual void smartAnalyze(bool forwards) override = 0;

    virtual void naiveAnalyze(bool forwards) override = 0;
};

template<class DataFlowImpl, typename K>
class DataFlowAnalysis : public DataFlowAnalysisAbstract {
public:
    CFG *cfg;
    std::vector<K> nodeIn;
    std::vector<K> nodeOut;
    DataFlowImpl factory;
    int count;

    explicit DataFlowAnalysis(CFG *cfg) : factory(), cfg(cfg) {
        static_assert(std::is_base_of<DataflowBaseSingleton<K>, DataFlowImpl>::value, "derived from wrong class??");
    }

    virtual std::string report() override {
        std::vector<std::string> strings;
        for (int i = 0; i < cfg->nodes.size(); i++) {
            strings.push_back(string_format("node %d, instr: %s\n", i, instr_to_string(cfg->nodes[i].instr).c_str()));
            strings.push_back(string_format("in: %s\n", factory.stringify(nodeIn[i]).c_str()));
            strings.push_back(string_format("out: %s\n", factory.stringify(nodeOut[i]).c_str()));
        }
        return joinToString(strings.begin(), strings.end(), "", "", "");
    }

    virtual std::string prettifyBlock() override {
        return cfg->prettifyBlocks<std::pair<std::vector<K>, std::vector<K>>>(
            [](int i, CFG *cfg, std::pair<std::vector<K>, std::vector<K>> nodeInOut) {
                auto [nodeIn, nodeOut] = nodeInOut;
                auto &blockNodes = cfg->blocks[i].nodes;
                auto &nodes = cfg->nodes;

                std::vector<std::string> nodeStrings;
                for (auto it = blockNodes.begin(); it != blockNodes.end(); it++) {
                    nodeStrings.push_back(instr_to_string(nodes[*it].instr));
                }
                
                return string_format(
                    "shape=Mrecord, label=\"{{%s}|%s|{%s}}\"",
                    DataFlowImpl().stringify(nodeIn[blockNodes.front()]).c_str(),
                    joinToString(nodeStrings.begin(), nodeStrings.end(), "", "\\n", "").c_str(),
                    DataFlowImpl().stringify(nodeOut[blockNodes.back()]).c_str()
                );
            },
            {nodeIn, nodeOut}
        );
    }


    virtual void smartAnalyze(bool forwards) override {
        count = 0;
        nodeIn.clear();
        nodeOut.clear();

        for (int i = 0; i < cfg->nodes.size(); i++) {
            nodeIn.push_back(factory.makeTop());
            nodeOut.push_back(factory.makeTop());
        }


        auto SCCs = cfg->dfs(); 
        if (forwards) {
            std::reverse(SCCs.begin(), SCCs.end());
        }

        for (auto iter = SCCs.begin(); iter != SCCs.end(); iter++) {
            auto &scc = *iter;
            std::deque<int> worklist;
            std::unordered_set<int> workSet;
            for (auto it = scc.begin(); it != scc.end(); it++) {
                worklist.push_back(*it);
                workSet.insert(*it);
            }

            while (worklist.size() != 0) {
                count++;
                auto v = worklist.front();
                worklist.pop_front();
                workSet.erase(v);
                auto node = cfg->nodes[v];

                std::vector<K> preds;
                for (auto predNodePtr: (forwards ? node.preds : node.succs)) {
                    preds.push_back(nodeOut[predNodePtr]);
                }

                auto meetRes = factory.meet(node, preds, (forwards ? nodeIn[v] : nodeOut[v]));
                auto transferRes = factory.transfer(node, meetRes.first, (forwards ? nodeOut[v] : nodeIn[v]));

                if (forwards) {
                    nodeIn[v] = meetRes.first;
                    nodeOut[v] = transferRes.first;
                } else {
                    nodeOut[v] = meetRes.first;
                    nodeIn[v] = transferRes.first;
                }

                if (!meetRes.second || !transferRes.second) {
                    auto &nextNodes = forwards ? node.succs : node.preds;
                    for (auto it = nextNodes.begin(); it != nextNodes.end(); it++) {
                        if (workSet.find(*it) == workSet.end()) {
                            workSet.insert(*it);
                            worklist.push_back(*it);
                        }
                    }
                }
            }
        }
    }

    virtual void naiveAnalyze(bool forwards) override {
        count = 0;
        nodeIn.clear();
        nodeOut.clear();
        for (int i = 0; i < cfg->nodes.size(); i++) {
            nodeIn.push_back(factory.makeTop());
            nodeOut.push_back(factory.makeTop());
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 0; i < cfg->nodes.size(); i++) {
                count++;
                auto node = cfg->nodes[i];

                std::vector<K> preds;
                for (auto predNodePtr: (forwards ? node.preds : node.succs)) {
                    preds.push_back(nodeOut[predNodePtr]);
                }

                auto meetRes = factory.meet(node, preds, (forwards ? nodeIn[i] : nodeOut[i]));
                auto transferRes = factory.transfer(node, meetRes.first, (forwards ? nodeOut[i] : nodeIn[i]));

                if (forwards) {
                    nodeIn[i] = meetRes.first;
                    nodeOut[i] = transferRes.first;
                } else {
                    nodeOut[i] = meetRes.first;
                    nodeIn[i] = transferRes.first;
                }
                
                if (!meetRes.second || !transferRes.second) {
                    changed = true;
                }
            }
        }
    }
};
