#include <utility>
#include <vector>
#include <algorithm>
#include "json.hpp"
#include "helpers.cpp"
#include<unordered_map>

using json = nlohmann::json;

class CFGNode {
public:
    json instr;
    std::vector<CFGNode *> preds;
    std::vector<CFGNode *> succs;

    CFGNode(json instr) {
        this->instr = std::move(instr);
    }
};

class CFGBlock {
public:
    std::vector<CFGNode *> nodes;
    std::vector<CFGBlock *> preds;
    std::vector<CFGBlock *> succs;
    std::string blockName;

    CFGBlock(std::string blockName, std::vector<CFGNode *> nodes) {
        this->blockName = blockName;
        this->nodes = nodes;
    }

};

class CFG {
public:
    CFG(json& instrs) {
        std::string prefix = "anti_clobber_label";
        PrefixCounter pc(prefix);
        std::string firstLabel = pc.generate();

        std::vector<CFGNode> nodes; // all instructions other than labels
        std::vector<uint32_t> blockStarts;  // first instruction in ith block
        std::vector<std::string> labels; // label for ith block
        std::vector<std::string> jumpLabels;

        blockStarts.push_back(0);
        labels.push_back(firstLabel);

        bool unreachable = false;

        for (auto instr : instrs) {
            if (unreachable && !instr.contains("label")) continue;

            unreachable = false;
            if (instr.contains("label")) {
                blockStarts.push_back(nodes.size());
                labels.push_back(instr["label"]);
            } else if (instr.contains("op") && instr["op"] == "jmp" || instr["op"] == "br") {
                nodes.emplace_back(instr);
                unreachable = true;
            } else {
                nodes.emplace_back(instr);
            }
        }

//        printf("labels: ");
//        for (std::string &s : labels) {
//            printf("%s, ", s.c_str());
//        }
//        printf("\n");
//
//        printf("block starts: ");
//        for (int i : blockStarts) {
//            printf("%d, ", i);
//        }
//
//        printf("\n");
//
//        printf("instrs:\n");
//        for (auto &node : nodes) {
//            printf("%s\n", node.instr.dump(2).c_str());
//        }

        numNodes = nodes.size();
        nodeArr = (CFGNode *) malloc(sizeof(CFGNode) * nodes.size());
        std::copy(nodes.begin(), nodes.end(), nodeArr);

        std::vector<CFGBlock> blocks;

        for (int i = 0; i < blockStarts.size(); i++) {
            uint32_t end = (i < blockStarts.size() - 1) ? blockStarts[i + 1] : numNodes;
            for (uint32_t j = blockStarts[i] + 1; j < end; j++) {
                nodeArr[j].preds.push_back(&nodeArr[j - 1]);
            }
            for (uint32_t j = blockStarts[i]; j < end - 1; j++) {
                nodeArr[j].succs.push_back(&nodeArr[j + 1]);
            }

            std::vector<CFGNode *> nodePtrs;
            for (uint32_t j = blockStarts[i]; j < end; j++) {
                nodePtrs.push_back(&nodeArr[j]);
            }

            CFGBlock block(labels[i], nodePtrs);
            blocks.push_back(block);
        }

        numBlocks = blocks.size();
        blockArr = (CFGBlock *) malloc(sizeof(CFGBlock) * numBlocks);
        std::copy(blocks.begin(), blocks.end(), blockArr);

        for (int i = 0; i < numBlocks; i++) {
            
        }



    }

private:
    CFGNode *nodeArr;
    uint32_t numNodes;
    CFGBlock *blockArr;
    uint32_t numBlocks;
};

template<typename T>
class ForwardDataflowBase {
public:
    static T copy(T t);

    static T makeTop();

    static std::pair<T, bool> inCalc(const CFGNode &node, const std::vector<T> &outs, const T &oldIn);

    static std::pair<T, bool> outCalc(const CFGNode &node, const T &inNew, const T &outOld);
};

template<class DataFlowImpl, typename K>
class DataFlowAnalysis {
public:
    CFG cfg;
    std::unordered_map<CFGNode *, K> nodeIn;
    std::unordered_map<CFGNode *, K> nodeOut;

    explicit DataFlowAnalysis(const CFG& cfg) {
        static_assert(std::is_base_of<ForwardDataflowBase<K>, DataFlowImpl>::value, "derived from wrong class??");
        this->cfg = cfg;
    }

    void forwardAnalysis() {
        nodeIn.clear();
        nodeOut.clear();
        for (auto nodePtr : cfg.nodes) {
            nodeIn[nodePtr] = DataFlowImpl::makeTop();
            nodeOut[nodePtr] = DataFlowImpl::makeTop();
        }

        bool changed = true;
        uint32_t count = 0;
        while (changed) {
            changed = false;
            for (auto nodePtr: cfg.nodes) {
                count++;
                auto node = *nodePtr;

                std::vector<K> preds;
                for (auto predNodePtr: node.preds) {
                    preds.push_back(nodeOut[predNodePtr]);
                }

                auto inRes = DataFlowImpl::inCalc(node, preds, nodeIn[nodePtr]);
                auto outRes = DataFlowImpl::outCalc(node, inRes.first, nodeOut[nodePtr]);

                nodeIn[nodePtr] = inRes.first;
                nodeOut[nodePtr] = outRes.first;
                if (!inRes.second || !outRes.second) {
                    changed = true;
                }
            }
        }
    }
};