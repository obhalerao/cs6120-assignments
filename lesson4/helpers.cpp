// created by ChatGPT, https://chat.openai.com/share/f727676a-4d04-47f7-8924-53678fbec5c0
#include <iostream>

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
