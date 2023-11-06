#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"
#include "../src/lib/cfg_utils.hpp"

namespace fs = std::filesystem

bool is_valid(std::string filename){
  int idx = filename.find("trace_");
  if(idx != 0) return false;
  int nidx = 6;
  while(filename[nidx] != '.'){
    char c = filename[nidx];
    if(!isdigit(c)){
      return false;
    }
    nidx++;
  }
  if(filename.length() != nidx+5) return false;
  return filename.substr(nidx) == ".json";
}

json insert_trace(json prog, json trace){
  json new_prog;

  // first, figure out where to insert the trace

  // while inserting, delete all jmp instructions and replace all branches with guards

  // after trace is inserted, add a jmp to a "successfully executed" label

  // failure label should go right after the end of the current trace

  // add "successfully executed" label after the end of the instruction in the old code




  return new_prog;
}

int main(int argc, char* argv[]){
  json prog;
  std::cin >> prog;

  json prog_with_index;
  

  for(auto func: prog["functions"]){
    json instrs = func["instrs"];
    json new_instrs;
    int idx = 0;
    for(auto instr: instrs){
      json new_instr = instr;
      new_instr["index"] = idx;
      new_instrs.push_back(new_instr);
      idx++;
    }
    json new_func = func;
    new_func["instrs"] = new_instrs;
    prog_with_index["functions"].push_back(new_func);
  }

  // general outline

  // include all the traces

  // for each trace, figure out where in the original code it started from
  // (function, instruction index)

  // pass through each trace
  // remove every jump instruction
  // replace every branch instruction with a guard based on whether it's taken or not
  // the failure label for the guard should go right after where the current trace was inserted
  // after the trace is committed, jump forward to a new label
  // new label should be inserted at the end of the code that corresponds to the end of the trace

  // do this for every trace and we should be okay

  // start by finding all the traces

  for(const auto& entry : fs::directory_iterator('.')){
    std::string filename = entry.path().filename();
    if(!is_valid(filename)) continue;
    std::ifstream infile(filename);
    json trace = json::parse(infile);
  }

}