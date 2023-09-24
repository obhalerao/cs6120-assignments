#ifndef INCLUDE_CFG_
#define INCLUDE_CFG_

#include "json.hpp"
using json = nlohmann::json;

class CFGNode {
public:
    json instr;
    std::vector<int> preds;
    std::vector<int> succs;
    CFGNode() ;

    CFGNode(json instr);

    CFGNode(const CFGNode &cfgNode) ;
};

class CFGBlock {
public:
    std::vector<int> nodes;
    std::vector<int> preds;
    std::vector<int> succs;
    std::string blockName;

    CFGBlock(std::string blockName, std::vector<int> nodes);

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
    template<typename T> std::string prettifyBlocks(std::string (*f)(int, CFG*, T), T t);

    void populate_dfs();

    std::vector<std::vector<int>> dfs();

private:
    std::optional<std::vector<std::vector<int>>> dfs_results;

};

#endif