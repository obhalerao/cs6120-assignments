#include <utility>
#include <vector>
#include <algorithm>
#include "json.hpp"
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
};

class CFG {
public:
    std::vector<CFGBlock *> blocks;
    std::vector<CFGNode *> nodes;

    std::string lastBlockName;

    std::vector<json> curr_block;

    CFG(json& instrs) {
        for (auto instr : instrs) {
            if (instr.contains("label")) {
                if (curr_block.size() != 0) {
                }
            }
        }
    }
};

template<typename T>
class ForwardDataflowBase {
public:
    static T copy(T t) = 0;

    static T makeTop() = 0;

    static std::pair<T, bool> inCalc(const CFGNode &node, const std::vector<T> &outs, const T &oldIn) = 0;

    static std::pair<T, bool> outCalc(const CFGNode &node, const T &inNew, const T &outOld) = 0;
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