#include "cfg_utils.hpp"
#include <cstdio>
#include <set>

typedef long long ll;

using doms_t = std::optional<std::set<int>>;

std::string stringify(doms_t t) {
        if(!t.has_value()){
            return "{all blocks}";
        }else{
            std::ostringstream ans;
            if(t.value().size() == 0) return "{}";
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

class DominatorAnalysis {
public:
    using doms_t = std::optional<std::set<int>>;
    
    CFG *cfg;
    std::vector<doms_t> block_in;
    std::vector<doms_t> dominators;
    std::vector<std::vector<int>> dominator_tree;
    std::vector<std::set<int>> frontier;
    int count;

    explicit DominatorAnalysis(CFG *cfg) : cfg(cfg) {}

    doms_t makeTop() {
        return std::nullopt;
    }

    std::pair<doms_t, bool> meet(const CFGBlock &block, const std::vector<doms_t> &influencers, const doms_t &old) {
        doms_t res = std::nullopt;
        bool changed = false;
        for(auto doms: influencers){
            if(doms.has_value()){
                if(!res.has_value()){
                    res = std::make_optional(doms.value());
                }else{
                    std::vector<int> toRemove;
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
            res.value().insert(block.id);
        }
        return {res, res == old};
    }

    void naiveAnalyze(){
        count = 0;
        dominators.clear();

        for (int i = 0; i < cfg->blocks.size(); i++) {
            dominators.push_back(makeTop());
        }

        for(int i = 0; i < cfg->blocks.size(); i++){
            std::vector<int> path;
            std::set<int> path_set;
            naive_dfs(path, path_set, 0, i);
        }
        compute_tree();
        compute_frontier();

    }

    void smartAnalyze() {
        count = 0;
        block_in.clear();
        dominators.clear();

        for (int i = 0; i < cfg->blocks.size(); i++) {
            block_in.push_back(makeTop());
            dominators.push_back(makeTop());
        }

        block_in[0] = std::make_optional((std::set<int>){});
        dominators[0] = std::make_optional((std::set<int>){0});

        auto blockOrder = cfg->dfs();

        std::deque<int> worklist;
        std::unordered_set<int> workSet;
        for (auto it = blockOrder.begin(); it != blockOrder.end(); it++) {
            if(*it == 0) continue;
            worklist.push_back(*it);
            workSet.insert(*it);
        }

        while (worklist.size() != 0) {
            count++;
            auto v = worklist.front();
            worklist.pop_front();
            workSet.erase(v);
            auto block = cfg->blocks[v];

            std::vector<doms_t> preds;
            for (auto predNodePtr: block.preds) {
                preds.push_back(dominators[predNodePtr]);
            }

            auto meetRes = meet(block, preds, block_in[v]);
            auto transferRes = transfer(block, meetRes.first, dominators[v]);

            block_in[v] = meetRes.first;
            dominators[v] = transferRes.first;
                
            if (!transferRes.second) {
                auto &nextNodes = block.succs;
                for (auto it = nextNodes.begin(); it != nextNodes.end(); it++) {
                    if (workSet.find(*it) == workSet.end()) {
                        workSet.insert(*it);
                        worklist.push_back(*it);
                    }
                }
            }
        }
        compute_tree();
        compute_frontier();

    }

    std::string report() {
        std::vector<std::string> strings;
        for (int i = 0; i < cfg->blocks.size(); i++) {
            strings.push_back(string_format("block %d, name: %s\n", i, cfg->blocks[i].blockName.c_str()));
            strings.push_back(string_format("dominators: %s\n", stringify(dominators[i]).c_str()));
            strings.push_back(string_format("dominance frontier: %s\n", stringify(frontier[i]).c_str()));
            strings.push_back("\n");
        }
        strings.push_back(prettify_tree());
        strings.push_back("\n");
        return joinToString(strings.begin(), strings.end(), "", "", "");
    }

    std::string prettifyBlock() {
        return cfg->prettifyBlocks<std::pair<std::vector<doms_t>, std::vector<doms_t>>>(
            [](int i, CFG *cfg, std::pair<std::vector<doms_t>, std::vector<doms_t>> block_in_out) {
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
            {block_in, dominators}
        );
    }

    std::string prettify_tree(){
        std::vector<std::string> strings;
        std::set<std::pair<int, int>> edges;
        for(int i = 0; i < cfg->blocks.size(); i++){
            for(auto j: dominator_tree[i]){
                int a = i;
                int b = j;
                if(a > b) std::swap(a, b);
                edges.insert({a, b});
            }
        }
        strings.push_back("Edges in dominator tree:\n");
        if(edges.size() == 0) strings.push_back("N/A\n");
        for(auto edge: edges){
            strings.push_back(string_format("%d -> %d\n", edge.first, edge.second));
        }
        return joinToString(strings.begin(), strings.end(), "", "", "");
    }

private:

    void compute_tree(){
        std::vector<std::vector<int>> edges(cfg->blocks.size());
        for(int i = 0; i < cfg->blocks.size(); i++){
            std::unordered_set<int> preds;
            for(auto pred: cfg->blocks[i].preds) preds.insert(pred);
            if(!dominators[i].has_value()) continue;
            for(auto dom: dominators[i].value()){
                int a = dom;
                int b = i;
                if(preds.find(dom) == preds.end()) continue;
                edges[a].push_back(b);
                edges[b].push_back(a);
            }
        }
        dominator_tree = edges;
    }
    
    void compute_frontier(){
        frontier.resize(cfg->blocks.size());
        compute_frontier_dfs(0, -1);
    }

    void compute_frontier_dfs(int n, int par){
        for(int c: dominator_tree[n]){
            if(c == par) continue;
            compute_frontier_dfs(c, n);
            for(auto nd: frontier[c]){
                if(dominators[n].value().find(nd) == dominators[n].value().end()){
                    bool good = false;
                    for(auto i: cfg->blocks[nd].preds){
                        if(dominators[n].value().find(i) != dominators[n].value().end()){
                            good = true;
                            break;
                        }
                    }
                    if(good) frontier[n].insert(nd);
                }
            }
        }
        for(auto nd: cfg->blocks[n].succs){
            if(dominators[n].value().find(nd) == dominators[n].value().end()){
                bool good = false;
                for(auto i: cfg->blocks[nd].preds){
                    if(dominators[n].value().find(i) != dominators[n].value().end()){
                        good = true;
                        break;
                    }
                }
                if(good) frontier[n].insert(nd);
            }
        }
    }

    void naive_dfs(std::vector<int> &path, std::set<int> &path_set,
    int i, int n){
        count++;
        path.push_back(i);
        path_set.insert(i);
        if(i == n){
            if(!dominators[n].has_value()){
                dominators[n] = std::make_optional(path_set);
            }else{
                std::vector<int> toRemove;
                for(int j: dominators[n].value()){
                    if(path_set.find(j)
                    == path_set.end()){
                        toRemove.push_back(j);
                    }
                }
                for(int val: toRemove) dominators[n].value().erase(val);
            }
        }else{
            for(auto j: cfg->blocks[i].succs){
                if(path_set.find(j) == path_set.end()){
                    naive_dfs(path, path_set, j, n);
                }
            }
        }
        path.pop_back();
        path_set.erase(i);
    }

};

int main(int argc, char* argv[]) {

    json prog;
    std::cin >> prog;

    bool profileMode = false;
    bool naiveMode = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-p") {
            profileMode = true;
        } else if (arg == "-n") {
            naiveMode = true;
        }
    }

    for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DominatorAnalysis(&cfg);
        if(naiveMode){
            analyzer.naiveAnalyze();
        }else{
            analyzer.smartAnalyze();
        }
        printf("%s\n", analyzer.report().c_str());
        if (profileMode) {
            fprintf(stderr, "Total function calls for function %s when computing dominators: %d\n", func["name"].get<std::string>().c_str(), analyzer.count);
        }
    }

}
