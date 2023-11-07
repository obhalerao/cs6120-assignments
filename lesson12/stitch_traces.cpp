#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"
#include "../src/lib/cfg_utils.hpp"

#include <iostream>
#include <fstream>

namespace fs = std::filesystem;

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

json insert_trace(json prog, json trace, int trace_idx){
  json new_prog = prog;

  // first, figure out where to insert the trace

  std::string func_name = trace["function"];
  int start_idx = trace["trace"][0]["index"];
  int end_idx = trace["trace"][trace["trace"].size()-1]["index"];

  json cur_func;
  int func_idx = 0;

  for(auto func: prog["functions"]){
    if(func["name"] == func_name){
      cur_func = func;
      break;
    }
    func_idx++;
  }

  json new_func = cur_func;
  new_func["instrs"] = {};
  
  for(auto instr: cur_func["instrs"]){
    if(instr.contains("index") && instr["index"] == start_idx){
      // while inserting, delete all jmp instructions and replace all branches with guards
      json speculate;
      speculate["op"] = "speculate";
      new_func["instrs"].push_back(speculate);
      for(auto trace_instr: trace["trace"]){
        if(trace_instr.contains("op")){
          std::string op = trace_instr["op"];
          if(op == "jmp"){
            continue;
          }else if(op == "br"){
            bool taken = trace_instr["taken"];
            json guard;
            guard["op"] = "guard";
            std::string boolean_var = trace_instr["args"][0];
            if(taken){
              guard["args"].push_back(boolean_var);
            }else{
              std::string not_boolean_var = "anti_clobber_not_"+boolean_var;
              json assign_instr;
              assign_instr["op"] = "not";
              assign_instr["dest"] = not_boolean_var;
              assign_instr["type"] = "bool";
              assign_instr["args"].push_back(boolean_var);
              new_func["instrs"].push_back(assign_instr);
              guard["args"].push_back(not_boolean_var);
            }
            guard["labels"].push_back("trace_"+std::to_string(trace_idx)+"_failure");
            new_func["instrs"].push_back(guard);
          }else{
            new_func["instrs"].push_back(trace_instr);
          }
        }else{
          new_func["instrs"].push_back(trace_instr);
        }
      }
      json commit;
      commit["op"] = "commit";
      new_func["instrs"].push_back(commit);
      json jmp_to_success;
      jmp_to_success["op"] = "jmp";
      jmp_to_success["labels"].push_back("trace_"+std::to_string(trace_idx)+"_success");
      new_func["instrs"].push_back(jmp_to_success);
      json failure_label;
      failure_label["label"] = "trace_"+std::to_string(trace_idx)+"_failure";
      new_func["instrs"].push_back(failure_label);

    }
    new_func["instrs"].push_back(instr);
    if(instr.contains("index") && instr["index"] == end_idx){
      json success_label;
      success_label["label"] = "trace_"+std::to_string(trace_idx)+"_success";
      new_func["instrs"].push_back(success_label);
    }
  }

  new_prog["functions"][func_idx] = new_func;

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

  json new_prog;

  int trace_idx = 0;

  for(const auto& entry : fs::directory_iterator(".")){
    std::string filename = entry.path().filename();
    if(!is_valid(filename)) continue;
    std::ifstream infile(filename);
    json trace = json::parse(infile);
    new_prog = insert_trace(prog_with_index, trace, trace_idx);
    prog_with_index = new_prog;
    trace_idx++;
  }

  for(auto func: prog_with_index["functions"]){
    for(auto instr: func["instrs"]){
      if(instr.contains("index")){
        instr.erase("index");
      }
    }
  }

  std::cout << prog_with_index << std::endl;

}