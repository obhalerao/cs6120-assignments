#include <iostream>
#include <stack>
#include <queue>

#include "dominator_analysis_temp.hpp"
#include "helpers.cpp"

#include "../src/lib/dataflow_analysis.hpp"
#include "../src/lib/cfg.hpp"
#include "../src/lib/cfg_utils.hpp"

std::map<std::pair<std::string, json>, std::set<int>> get_defs(CFG &cfg, json func_args){
  std::map<std::pair<std::string, json>, std::set<int>> defs_map;
  for(auto arg: func_args){
    defs_map[{arg["name"], arg["type"]}] = {0};
  }
  int idx = 0;
  for(auto block: cfg.blocks){
    for(auto node: block.nodes){
      if(cfg.nodes[node].instr.contains("dest")){
        if(defs_map.find({cfg.nodes[node].instr["dest"],
          cfg.nodes[node].instr["type"]}) == defs_map.end()){
          defs_map[{cfg.nodes[node].instr["dest"],
          cfg.nodes[node].instr["type"]}] = {};
        }
        defs_map[{cfg.nodes[node].instr["dest"],
          cfg.nodes[node].instr["type"]}].insert(idx);
      }
    }
    idx++;
  }
  return defs_map;
}

std::vector<std::set<std::pair<std::string,json>>> get_phi_defs(CFG &cfg, DominatorAnalysis &analyzer,
  std::map<std::pair<std::string, json>, std::set<int>> &defs_map){
    std::vector<std::set<std::pair<std::string, json>>> phi_defs;
    for(int i = 0; i < cfg.blocks.size(); i++) phi_defs.push_back({});
    for(auto &[entry, blocks]: defs_map){
      bool bad = true;
      std::set<int> to_process(blocks);
      while(bad){
        bad = false;
        std::set<int> to_add;
        for(int cur_block: to_process){
          for(int blk: analyzer.frontier[cur_block]){
            phi_defs[blk].insert(entry);
            if(blocks.find(blk) == blocks.end()){
              bad = true;
              blocks.insert(blk);
              to_add.insert(blk);
            }
          }
        }
        to_process = to_add;
      }
    }
  return phi_defs;
}

json add_phi_nodes(CFG &cfg, DominatorAnalysis &analyzer, json func_args, PrefixCounter* undef_ref){
  std::map<std::pair<std::string,json>, std::set<int>> defs_map = get_defs(cfg, func_args);
  std::vector<std::set<std::pair<std::string,json>>> phi_defs = get_phi_defs(cfg, analyzer, defs_map);
  json instrs;
  for(auto blk: cfg.blocks){
    json label;
    label["label"] = blk.blockName;
    if(blk.id != 0) instrs.push_back(label);
    for(auto var: phi_defs[blk.id]){
      json phi_instr = json{{"op", "phi"}};
      phi_instr["dest"] = var.first;
      phi_instr["type"] = var.second;
      instrs.push_back(phi_instr);
    }
    for(auto node: blk.nodes){
      instrs.push_back(cfg.nodes[node].instr);
    }
  }
  return instrs;
}

void relabel_block(CFG &cfg, DominatorAnalysis &analyzer, int block_id, int parent_id,
  std::map<std::string, std::stack<std::string>> &variable_names,
  std::map<std::string, PrefixCounter*> &counters,
  std::map<int, std::string> orig_node_label){
  std::map<std::string, int> totals;
  for(auto node_id: cfg.blocks[block_id].nodes){
    CFGNode &cur_node = cfg.nodes[node_id];
    if(cur_node.instr.contains("args")
      && !(cur_node.instr.contains("op") && cur_node.instr["op"] == "phi")){
      json new_args;
      for(std::string arg: cur_node.instr["args"]){
        std::string new_name = variable_names[arg].top();
        new_args.push_back(new_name);
      }
      cur_node.instr["args"] = new_args;
    }
    if(cur_node.instr.contains("dest")){
      std::string old_dest = cur_node.instr["dest"];
      std::string new_dest = counters[old_dest]->generate();
      variable_names[old_dest].push(new_dest);
      if(totals.find(old_dest) == totals.end()) totals[old_dest] = 1;
      else totals[old_dest]++;
      cur_node.instr["dest"] = new_dest;
    }
  }

  for(auto succ: cfg.blocks[block_id].succs){
    for(auto node_id: cfg.blocks[succ].nodes){
      CFGNode &cur_node = cfg.nodes[node_id];
      if(!(cur_node.instr.contains("op") && cur_node.instr["op"] == "phi")){
        break;
      }
      if(variable_names[orig_node_label[cur_node.id]].empty()) continue;
      std::string name = variable_names[orig_node_label[cur_node.id]].top();
      cur_node.instr["args"].push_back(variable_names[orig_node_label[cur_node.id]].top());
      cur_node.instr["labels"].push_back(cfg.blocks[block_id].blockName);
    }
  }

  for(auto child: analyzer.dominator_tree[block_id]){
    if(child == parent_id) continue;
    relabel_block(cfg, analyzer, child, block_id, variable_names, counters, orig_node_label);
  }

  for(auto entry: totals){
    for(int i = 0; i < entry.second; i++) variable_names[entry.first].pop();
  }
}

std::set<std::string> get_vars(json instrs, json func_args){
  std::set<std::string> ans;
  for(auto arg: func_args){
    ans.insert(arg["name"]);
  }
  for(auto instr: instrs){
    if(instr.contains("dest")){
      ans.insert(instr["dest"]);
    }
    if(instr.contains("args")){
      for(std::string arg: instr["args"]){
        ans.insert(arg);
      }
    }
  }
  return ans;
}

void relabel_vars(CFG &cfg, DominatorAnalysis &analyzer, std::set<std::string> &vars,
json func_args){
  std::map<std::string, std::stack<std::string>> variable_names;
  std::map<std::string, PrefixCounter*> counters;
  std::string prefix = short_unique_prefix(vars);
  for(auto var: vars){
    std::stack<std::string> curstk;
    variable_names[var] = curstk;
    PrefixCounter* pc = new PrefixCounter(prefix + "_" + var);
    counters[var] = pc;
  }
  for(auto arg: func_args){
    variable_names[arg["name"]].push(arg["name"]);
  }
  std::map<int, std::string> orig_labels;
  for(auto node: cfg.nodes){
    if(node.instr.contains("op") && node.instr["op"] == "phi"){
      orig_labels[node.id] = node.instr["dest"];
    }
  }
  relabel_block(cfg, analyzer, 0, -1, variable_names, counters, orig_labels);
  for(auto var: vars){
    delete counters[var];
  }
}

std::map<std::string, int> gen_block2index(CFG &cfg){
  std::map<std::string, int> block2index;
  for(auto block: cfg.blocks){
    block2index[block.blockName] = block.id;
  }
  return block2index;
}

std::vector<std::vector<std::pair<std::pair<std::string, json>, std::string>>> 
get_phi_vars(CFG &cfg, json func_args){
  std::map<std::string, int> block2index = gen_block2index(cfg);
  std::vector<std::vector<std::pair<std::pair<std::string, json>, std::string>>> phi_vars;
  std::map<std::pair<std::string, json>, std::set<int>> var2block = get_defs(cfg, func_args);
  for(auto block: cfg.blocks) phi_vars.push_back({}); 
  for(auto block: cfg.blocks){
    for(auto node_idx: block.nodes){
      auto instr = cfg.nodes[node_idx].instr;
      if(!(instr.contains("op") && instr["op"] == "phi")) break;
      std::string dest = instr["dest"];
      json b_type = instr["type"];
      std::map<std::string, std::vector<int>> args2blocks;
      for(int i = 0; i < instr["args"].size(); i++){
        std::string arg = instr["args"][i];
        int pred = block2index[instr["labels"][i]];
        if(args2blocks.find(arg) == args2blocks.end()) args2blocks[arg] = {};
        args2blocks[arg].push_back(pred);
      }
      for(auto arg: args2blocks){
        std::queue<int> q;
        int dest_blk = *(var2block[{arg.first, b_type}].begin());
        for(int blk: arg.second) q.push(blk);
        std::set<int> seen;
        while(!q.empty()){
          int nd = q.front();
          q.pop();
          if(seen.find(nd) != seen.end()) continue;
          seen.insert(nd);
          phi_vars[nd].push_back({{dest, b_type}, arg.first});
          if(nd == dest_blk) continue;
          for(int pred: cfg.blocks[nd].preds) q.push(pred);
        }
      }
    }
  }
  return phi_vars;
}

json remove_phi_nodes(CFG &cfg, json func_args){
  std::vector<std::vector<std::pair<std::pair<std::string, json>, std::string>>> 
    phi_vars = get_phi_vars(cfg, func_args);
  json new_instrs;
  for(auto blk: cfg.blocks){
    json label;
    label["label"] = blk.blockName;
    new_instrs.push_back(label);
    for(auto node: blk.nodes){
      auto instr = cfg.nodes[node].instr;
      if(instr.contains("op") && instr["op"] == "phi") continue;
      if(instr.contains("op") && instr["op"] == "br" || instr["op"] == "jmp") break;
      new_instrs.push_back(instr);
    }
    for(auto entry: phi_vars[blk.id]){
      json cur_instr;
      cur_instr["op"] = "id";
      cur_instr["dest"] = entry.first.first;
      cur_instr["type"] = entry.first.second;
      cur_instr["args"].push_back(entry.second);
      new_instrs.push_back(cur_instr);
    }
    auto last_instr = cfg.nodes[blk.nodes[blk.nodes.size()-1]].instr;
    if(last_instr.contains("op") && last_instr["op"] == "br" || last_instr["op"] == "jmp"){
      new_instrs.push_back(last_instr);
    }
  }
  return new_instrs;
}

int main(int argc, char* argv[]){
  json prog;
  std::cin >> prog;

  bool phiMode = false;

  for (int i = 1; i < argc; ++i) {
      std::string arg = argv[i];
      if (arg == "-p") {
          phiMode = true;
      }
  }

  json new_prog;


  for(auto func : prog["functions"]){
        CFG cfg(func["name"].get<std::string>(), func["instrs"]);
        auto analyzer = DominatorAnalysis(&cfg);
        analyzer.smartAnalyze();
        json new_instrs = add_phi_nodes(cfg, analyzer, func["args"]);
        CFG new_cfg(func["name"].get<std::string>(), new_instrs);
        auto new_analyzer = DominatorAnalysis(&new_cfg);
        new_analyzer.smartAnalyze();
        std::set<std::string> vars = get_vars(new_instrs, func["args"]);
        relabel_vars(new_cfg, new_analyzer, vars, func["args"]);
        json new_func = func;
        if(phiMode) new_func["instrs"] = new_cfg.toInstrs();
        else new_func["instrs"] = remove_phi_nodes(new_cfg, func["args"]);
        new_prog["functions"].push_back(new_func);
        // std::cout << new_cfg.toInstrs() << std::endl;
    }
    
    std::cout << new_prog << std::endl;
}