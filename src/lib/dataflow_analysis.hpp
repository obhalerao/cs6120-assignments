#ifndef INCLUDE_DATAFLOW_
#define INCLUDE_DATAFLOW_

#include "cfg.hpp"

template<typename T> class DataflowBaseSingleton {
public:
    virtual T makeTop();

    virtual std::pair<T, bool> meet(const CFGNode &node, const std::vector<T> &influencers, const T &old);

    virtual std::pair<T, bool> transfer(const CFGNode &node, const T &influence, const T &old);

    virtual std::string stringify(T t);
};

class DataFlowAnalysisBase {
public:
    virtual std::string report();

    virtual std::string prettifyBlock();

    virtual void smartAnalyze(bool forwards);

    virtual void worklistAnalyze(bool forwards);

    virtual void naiveAnalyze(bool forwards);
};

template<class DataFlowImpl, typename K>
class DataFlowAnalysis : public DataFlowAnalysisBase {
public:
    CFG *cfg;
    std::vector<K> nodeIn;
    std::vector<K> nodeOut;
    DataFlowImpl factory;
    int count;

    explicit DataFlowAnalysis(CFG *cfg);

    virtual std::string report() override;

    virtual std::string prettifyBlock() override;


    virtual void smartAnalyze(bool forwards) override;

    virtual void worklistAnalyze(bool forwards) override;

    virtual void naiveAnalyze(bool forwards) override;
};


#endif