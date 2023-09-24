#ifndef INCLUDE_CFG_
#define INCLUDE_CFG_

#include "json.hpp"
#include "cfg_utils.hpp"

using json = nlohmann::json;

class CFGNode {
public:
    json instr;
    std::vector<int> preds;
    std::vector<int> succs;
    int id;
    CFGNode() ;

    CFGNode(json instr, int id);

    CFGNode(const CFGNode &cfgNode) ;
};

class CFGBlock {
public:
    std::vector<int> nodes;
    std::vector<int> preds;
    std::vector<int> succs;
    std::string blockName;
    int id;

    CFGBlock(std::string blockName, std::vector<int> nodes, int id);

    CFGBlock(const CFGBlock &cfgBlock);

};

class CFG {
public:
    std::string funcName;
    std::vector<CFGNode> nodes;
    std::vector<CFGBlock> blocks;
    std::unordered_map<std::string, int> blockOfString;

    CFG(std::string funcName, json& instrs);

    json toInstrs();

    std::string prettifyNodes(std::string (*f)(int, CFG*));

    // T is a catch-all for any additional args you may want to pass into the printer
    template<typename T> std::string prettifyBlocks(std::string (*f)(int, CFG*, T), T t) {
        std::vector<std::string> lines;
        lines.push_back(string_format<const char*>("label = \"%s\"", funcName.c_str()));
        for (int i = 0; i < blocks.size(); i++) {
            lines.push_back(string_format<const char*, int, const char*>("%s_block_%d [%s]", funcName.c_str(), i, f(i, this, t).c_str()));
        }
        for (int i = 0; i < blocks.size(); i++) {
            for (auto j : blocks[i].succs) {
                lines.push_back(string_format<const char*, int, const char*, int>("%s_block_%d -> %s_block_%d", funcName.c_str(), i, funcName.c_str(), j));
            }
        }

        auto lines_str = joinToString<std::vector<std::string>::iterator>(lines.begin(), lines.end(), "", "\n\t", "");

        std::string ans = string_format<const char*, const char*>("subgraph cluster_%s_blocks {\n\t%s\n}\n", funcName.c_str(), lines_str.c_str());
        return ans;
    }

    std::vector<std::vector<int>> node_dfs();

    std::vector<int> block_dfs();


private:
    std::optional<std::vector<int>> block_dfs_results;
    std::optional<std::vector<std::vector<int>>> node_dfs_results;

    void topsort_dfs(std::vector<int>& topsort, std::vector<bool>& visited, int i);

    void populate_dfs();

    void populate_node_dfs();

};

#endif