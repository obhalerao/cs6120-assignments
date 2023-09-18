#include "cfg_utils.hpp"
#include <cstdio>
#include <set>

typedef long long ll;

class DominatorAnalysis {
public:
    using doms_t = std::optional<std::set<std::string>>;
    
    CFG *cfg;
    std::vector<doms_t> block_in;
    std::vector<doms_t> block_out;
    int count;

    doms_t makeTop() {
        return std::make_optional((std::set<std::string>){});
    }

    std::pair<doms_t, bool> meet(const CFGBlock &block, const std::vector<doms_t> &influencers, const doms_t &old) {
        doms_t res = std::nullopt;
        bool changed = false;
        for(auto doms: influencers){
            if(doms.has_value()){
                if(!res.has_value()){
                    res = std::make_optional(doms.value());
                }else{
                    std::vector<std::string> toRemove;
                    for(auto elem: res.value()){
                        if(doms.value().find(elem) == doms.value().end()){
                            toRemove.push_back(elem);
                        }
                    }
                    for(auto elem: toRemove) res.value().erase(elem);
                }
            }
        }
        return {res, res == old};
    };

    std::pair<doms_t, bool> transfer(const CFGBlock &block, const doms_t &influence, const doms_t &old) {
        doms_t res(influence);
        if(res.has_value()){
            res.value().insert(block.blockName);
        }
        return {res, res == old};
    }

    std::string stringify(doms_t t) {
        if(!t.has_value()){
            return "{all blocks}";
        }else{
            std::ostringstream ans;
            auto begin = t.value().begin();
            auto end = t.value().end();
            ans << "{";
            ans << *begin;
            begin++;
            while(begin != end){
                ans << ",";
                ans << *begin;
                begin++;
            }
            ans << "}";
            return ans.str();
        }
    }

    void smartAnalyze() {
        count = 0;
        block_in.clear();
        block_out.clear();

        for (int i = 0; i < cfg->blocks.size(); i++) {
            block_in.push_back(makeTop());
            block_out.push_back(makeTop());
        }

        block_in[0] = std::make_optional((std::set<std::string>){});
        block_out[0] = std::make_optional((std::set<std::string>){cfg->blocks[0].blockName});

        auto SCCs = cfg->dfs(); 
        std::reverse(SCCs.begin(), SCCs.end());

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
                auto block = cfg->blocks[v];

                std::vector<doms_t> preds;
                for (auto predNodePtr: block.preds) {
                    preds.push_back(block_out[predNodePtr]);
                }

                auto meetRes = meet(block, preds, block_in[v]);
                auto transferRes = transfer(block, meetRes.first, block_in[v]);

                block_in[v] = meetRes.first;
                block_out[v] = transferRes.first;
                 
                if (!meetRes.second || !transferRes.second) {
                    auto &nextNodes = block.succs;
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

    std::string prettifyBlock() {
        return cfg->prettifyBlocks<std::pair<std::vector<doms_t>, std::vector<doms_t>>>(
            [this](int i, CFG *cfg, std::pair<std::vector<doms_t>, std::vector<doms_t>> block_in_out) {
                auto [block_in, block_out] = block_in_out;
                auto &blockNodes = cfg->blocks[i].nodes;
                auto &nodes = cfg->nodes;

                std::vector<std::string> nodeStrings;
                for (auto it = blockNodes.begin(); it != blockNodes.end(); it++) {
                    nodeStrings.push_back(instr_to_string(nodes[*it].instr));
                }
                
                return string_format(
                    "shape=Mrecord, label=\"{{%s}|%s|{%s}}\"",
                    stringify(block_in[i]).c_str(),
                    joinToString(nodeStrings.begin(), nodeStrings.end(), "", "\\n", "").c_str(),
                    stringify(block_out[i]).c_str()
                );
            },
            {block_in, block_out}
        );
    }

};

int main(int argc, char* argv[]) {

    json prog;
    std::cin >> prog;

}
