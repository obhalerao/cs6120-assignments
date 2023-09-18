#include "dataflow_analysis.hpp"
#include <cstdio>
#include <set>

typedef long long ll;

class DominatorAnalysis : DataflowBaseSingleton<std::optional<std::set<int>>> {
public:
    using doms_t = std::optional<std::set<int>>;
    doms_t makeTop() override {
        return std::make_optional((std::set<int>){});
    }

    std::pair<doms_t, bool> meet(const CFGNode &node, const std::vector<doms_t> &influencers, const doms_t &old) override {
        
    };

    std::pair<doms_t, bool> transfer(const CFGNode &node, const doms_t &influence, const doms_t &old) override {
        
    }

    std::string stringify(doms_t t) override {

    }
};

int main(int argc, char* argv[]) {

    json prog;
    std::cin >> prog;

}
