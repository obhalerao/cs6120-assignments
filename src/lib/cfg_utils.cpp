#include <string>
#include <unordered_map>
#include <algorithm>
#include "json.hpp"
#include "cfg_utils.hpp"

using json = nlohmann::json;

// created by ChatGPT, https://chat.openai.com/share/c895b934-964d-4fac-94cd-05f00a233ba7
PrefixCounter::PrefixCounter(const std::string& prefix) : prefix(prefix), count(0) {}

std::string PrefixCounter::generate() {
    std::string result = prefix + "_" + std::to_string(count);
    count++;
    return result;
}

#ifdef IM_BAD__
// originally created by ChatGPT, https://chat.openai.com/share/c895b934-964d-4fac-94cd-05f00a233ba7
template <typename IteratorT> std::string joinToString(IteratorT begin,
                         IteratorT end,
                         const std::string& prefix,
                         const std::string& separator,
                         const std::string& suffix) {
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
// template std::string string_format<const char*, int, const char*, int>(const std::string& format, const char* a1, int a2, const char* a3, int a4);
// template std::string string_format<const char*, int, const char*>(const std::string& format, const char* a1, int a3, const char* a2);
// template std::string string_format<const char*, const char*>(const std::string& format, const char* a1, const char* a2);
// template std::string string_format<const char*, const char*>(const std::string& format, const char* a1, const char* a2);

#endif

std::string lower(std::string data) {
    std::string ret = std::string(data);
    std::transform(ret.begin(), ret.end(), ret.begin(), [](unsigned char c){ return std::tolower(c); });
    return ret;
}

// from briltxt.py
std::string value_to_string(json type, json value) {
    const std::unordered_map<int, std::string> control_chars {
        {0, std::string("\\0")},
        {7, std::string("\\a")},
        {8, std::string("\\b")},
        {9, std::string("\\t")},
        {10, std::string("\\n")},
        {11, std::string("\\v")},
        {12, std::string("\\f")},
        {13, std::string("\\r")},
    };

    if (!type.is_object()) {
        auto type_str = std::string(type);
        std::string lowered = lower(type_str);
        if (lowered == "char") {
            std::string value_str = value.get<std::string>();
            uint32_t value_int = value_str.at(0);
            auto search = control_chars.find(value_int);
            if (search != control_chars.end()) {
                std::string ret = std::string("'") + search->second + std::string("'");
                return ret;
            }
        }
    }

    std::string ret;

    if (value.is_number_integer()) {
        ret = std::to_string(value.get<int64_t>());
    } else if (value.is_boolean()) {
        ret = value.get<bool>() ? "true" : "false";
    } else if (value.is_number_float()) {
        ret = std::to_string(value.get<float>());
    }
    return ret;
}

// from briltxt.py
std::string type_to_string(json type) {
    if (type.is_object()) {
        for (auto& el : type.items()) {
            return std::string(el.key()) + "<" + type_to_string(el.value()) + ">";
        }
        return "type_to_string_failed";
    }
    else {
        return type;
    }
}

// from briltxt.py
std::string instr_to_string(json instr) {
    if (instr.contains("op") && instr["op"] == "const") {
        std::string dest_str = instr["dest"];
        std::string tyann = (instr.contains("type")) ? (": " + type_to_string(instr["type"])) : "";
        

        auto dest = instr["dest"].get<std::string>();
        std::string value = value_to_string(instr["type"], instr["value"]);

        return dest + tyann + " = const " + value;
    }
    else {
        std::string rhs = instr["op"];
        if (instr.contains("funcs")) {
            for (auto func : instr["funcs"]) {
                rhs += " @" + func.get<std::string>();
            }
        }
        if (instr.contains("args")) {
            for (auto arg : instr["args"]) {
                rhs += " " + arg.get<std::string>();
            }
        }
        if (instr.contains("labels")) {
            for (auto label : instr["labels"]) {
                rhs += " ." + label.get<std::string>();
            }
        }
        if (instr.contains("dest")) {
            std::string tyann = (instr.contains("type")) ? (": " + type_to_string(instr["type"])) : "";
            auto dest = instr["dest"].get<std::string>();
            return dest + tyann + " = " + rhs;
        } else {
            return rhs;
        }
    }
}