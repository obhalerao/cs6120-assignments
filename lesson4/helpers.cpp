// both created by ChatGPT, https://chat.openai.com/share/f727676a-4d04-47f7-8924-53678fbec5c0
#include <iostream>
#include <iterator>
#include <sstream>
#include <unordered_set>

class PrefixCounter {
public:
    PrefixCounter(const std::string& prefix) : prefix(prefix), count(0) {}

    std::string generate() {
        std::string result = prefix + "_" + std::to_string(count);
        count++;
        return result;
    }

private:
    std::string prefix;
    int count;
};


std::string joinToString(typename std::unordered_set<std::string>::iterator begin,
                         typename std::unordered_set<std::string>::iterator end,
                         const std::string& prefix,
                         const std::string& suffix,
                         const std::string& separator) {
    if (begin == end) {
        return prefix + suffix;
    }

    std::ostringstream oss;
    oss << prefix;
    oss << *begin;  // Add the first element
    ++begin;

    while (begin != end) {
        oss << separator << *begin;  // Add separator and the next element
        ++begin;
    }
    oss << suffix;

    return oss.str();
}
