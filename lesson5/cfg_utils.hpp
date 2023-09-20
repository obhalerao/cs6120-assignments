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
    int id;

    CFGBlock(std::string blockName, std::vector<int> nodes, int id) : blockName(blockName), nodes(nodes), id(id) {}

    CFGBlock(const CFGBlock &cfgBlock) : nodes(cfgBlock.nodes), preds(cfgBlock.preds), succs(cfgBlock.succs), blockName(cfgBlock.blockName), id(cfgBlock.id){}

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
            blocks.emplace_back(labels[i], nodePtrs, i);
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

    std::vector<int> dfs() {
        if (!dfs_results.has_value()) populate_dfs();
        return dfs_results.value();
    }

private:
    std::optional<std::vector<int>> dfs_results;

    void topsort_dfs(std::vector<int>& topsort, std::vector<bool>& visited, int i) {
        visited[i] = true;
        for(int j: blocks[i].succs){
            if(!visited[j]) topsort_dfs(topsort, visited, j);
        }
        topsort.push_back(i);
    }

    void populate_dfs(){
        std::vector<int> topsort;
        std::vector<bool> visited(blocks.size(), false);
        for(int i = 0; i < blocks.size(); i++){
            if(!visited[i]) topsort_dfs(topsort, visited, i);
        }
        std::reverse(topsort.begin(), topsort.end());
        dfs_results = topsort;
    }

};

