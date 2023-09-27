#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_set>
using json = nlohmann::json;

std::string short_unique_prefix(std::set<std::string> strs){
    bool bad = true;
    std::vector<char> prefix;
    while(bad){
        bad = false;
        prefix.push_back('a');
        for(auto str: strs){
            bool good = false;
            for(int i = 0; i < prefix.size(); i++){
                if(i >= str.length()|| prefix[i] != str[i]){
                    good = true;
                    break;
                }
            }
            if(!good){
                if(prefix[prefix.size()-1] == 'z'){
                    bad = true;
                    break;
                }else{
                    prefix[prefix.size()-1]++;
                }
            }
        }
    }
    std::string ans(prefix.begin(), prefix.end());
    return ans;
}


