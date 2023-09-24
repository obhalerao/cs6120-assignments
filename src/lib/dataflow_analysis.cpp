#include <unordered_set>
#include <deque>
#include "cfg.hpp"
#include "cfg_utils.cpp"
#include "dataflow_analysis.hpp"

template<typename T> T DataflowBaseSingleton<T>::makeTop() = 0;

template<typename T> std::pair<T, bool> DataflowBaseSingleton<T>::meet(const CFGNode &node, const std::vector<T> &influencers, const T &old) = 0;

template<typename T> std::pair<T, bool> DataflowBaseSingleton<T>::transfer(const CFGNode &node, const T &influence, const T &old) = 0;

template<typename T> std::string DataflowBaseSingleton<T>::stringify(T t) = 0;

std::string DataFlowAnalysisBase::report() { return "dummy-impl"; }

std::string DataFlowAnalysisBase::prettifyBlock() { return "dummy-impl"; }

void DataFlowAnalysisBase::smartAnalyze(bool forwards) { printf("passing\n"); }

void DataFlowAnalysisBase::worklistAnalyze(bool forwards) { printf("passing\n"); }

void DataFlowAnalysisBase::naiveAnalyze(bool forwards) { printf("passing\n"); }


class DataFlowAnalysisAbstract : public DataFlowAnalysisBase {
public:
    virtual std::string report() override = 0;

    virtual std::string prettifyBlock() override = 0;

    virtual void smartAnalyze(bool forwards) override = 0;

    virtual void worklistAnalyze(bool forwards) override = 0;

    virtual void naiveAnalyze(bool forwards) override = 0;
};

template<class DataFlowImpl, typename K> DataFlowAnalysis<DataFlowImpl, K>::DataFlowAnalysis(CFG *cfg) : factory(), cfg(cfg) {
        static_assert(std::is_base_of<DataflowBaseSingleton<K>, DataFlowImpl>::value, "derived from wrong class??");
    }

    template<class DataFlowImpl, typename K> std::string DataFlowAnalysis<DataFlowImpl, K>::report()  {
        std::vector<std::string> strings;
        for (int i = 0; i < cfg->nodes.size(); i++) {
            strings.push_back(string_format("node %d, instr: %s\n", i, instr_to_string(cfg->nodes[i].instr).c_str()));
            strings.push_back(string_format("in: %s\n", factory.stringify(nodeIn[i]).c_str()));
            strings.push_back(string_format("out: %s\n", factory.stringify(nodeOut[i]).c_str()));
        }
        return joinToString(strings.begin(), strings.end(), "", "", "");
    }

    template<class DataFlowImpl, typename K> std::string DataFlowAnalysis<DataFlowImpl, K>::prettifyBlock()  {
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


    template<class DataFlowImpl, typename K> void DataFlowAnalysis<DataFlowImpl, K>::smartAnalyze(bool forwards) {
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
            std::unordered_set<int> sccSet;
            std::deque<int> worklist;
            std::unordered_set<int> workSet;
            for (auto it = scc.begin(); it != scc.end(); it++) {
                worklist.push_back(*it);
                workSet.insert(*it);
                sccSet.insert(*it);
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
                        if (workSet.find(*it) == workSet.end()
                            && sccSet.find(*it) != sccSet.end()) {
                            workSet.insert(*it);
                            worklist.push_back(*it);
                        }
                    }
                }
            }
        }
    }

    template<class DataFlowImpl, typename K> void DataFlowAnalysis<DataFlowImpl, K>::worklistAnalyze(bool forwards) {
        count = 0;
        nodeIn.clear();
        nodeOut.clear();
        std::deque<int> worklist;
        std::unordered_set<int> workSet;
        for (int i = 0; i < cfg->nodes.size(); i++) {
            nodeIn.push_back(factory.makeTop());
            nodeOut.push_back(factory.makeTop());
            worklist.push_back(i);
            workSet.insert(i);
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

    template<class DataFlowImpl, typename K> void DataFlowAnalysis<DataFlowImpl, K>::naiveAnalyze(bool forwards) {
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