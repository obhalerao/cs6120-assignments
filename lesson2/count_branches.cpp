#include "rapidjson/document.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace rapidjson;

// Counts the number of branches in the JSON representation of a bril program.

int main(int argc, char* argv[]){

  // expects json filename as argument
  const char* infile = argv[1];
  std::ifstream fin(infile);
  std::stringstream buf;
  buf << fin.rdbuf();
  std::string json = buf.str();
  fin.close();

  // parses text of file into json
  Document document;
  document.Parse(json.c_str());

  const Value& functions = document["functions"];

  int branches {0};

  // iterate through functions and instrs
  for(Value::ConstValueIterator function = functions.Begin();
      function != functions.End(); function++){
        const Value& instrs = (*function)["instrs"];
        for(Value::ConstValueIterator instr = instrs.Begin();
            instr != instrs.End(); instr++){
            if(instr->HasMember("op") && (*instr)["op"] == "br") branches++;
        }
  }

  //print number of branches
  std::cout << "This program has " << branches << " branches." << std::endl;
       

}