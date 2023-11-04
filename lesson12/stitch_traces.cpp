#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"
#include "../src/lib/cfg_utils.hpp"

int main(int argc, char* argv[]){
  json prog;
  std::cin >> prog;

  json new_prog;

  // general outline

  // include all the traces

  // for each trace, figure out where in the original code it started from
  // (function, instruction index)

  // pass through each trace
  // remove every jump instruction
  // replace every branch instruction with a guard based on whether it's taken or not
  // the failure label for the guard should go right after where the current trace was executed
  // after the trace is committed, jump forward to a new label
  // new label should be inserted at the end of the code that corresponds to the end of the trace

  // do this for every trace and we should be okay


}