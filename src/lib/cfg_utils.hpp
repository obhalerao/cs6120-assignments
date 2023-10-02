#ifndef CFG_UTILS_
#define CFG_UTILS_

#include <string>
#include <sstream>
#include "json.hpp"

using json = nlohmann::json;

// created by ChatGPT, https://chat.openai.com/share/c895b934-964d-4fac-94cd-05f00a233ba7
class PrefixCounter {
public:
    PrefixCounter(const std::string& prefix);

    std::string generate();

private:
    std::string prefix;
    int count;
};

// originally created by ChatGPT, https://chat.openai.com/share/c895b934-964d-4fac-94cd-05f00a233ba7
template <typename IteratorT> std::string joinToString(IteratorT begin,
                         IteratorT end,
                         const std::string& prefix,
                         const std::string& separator,
                         const std::string& suffix){
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

// from https://stackoverflow.com/a/26221725
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

std::string lower(std::string data);

// from briltxt.py
std::string value_to_string(json type, json value);
// from briltxt.py
std::string type_to_string(json type);

// from briltxt.py
std::string instr_to_string(json instr);

#endif