
#include "string_util.h"

#include <sstream>
#include <algorithm>
#include <string.h>

namespace tools {

std::string trim(std::string input, const char * trim_chars)
{
    size_t  pos1 = input.find_first_not_of(trim_chars);
    size_t  pos2 = input.find_last_not_of(trim_chars);
    if (input.empty() || pos1 == std::string::npos || pos2 == std::string::npos)
        return std::string("");
    return input.substr(pos1, pos2 - pos1 + 1);
}

int split(std::string input, std::list<std::string>& output, char delimiter)
{
    std::stringstream   oss;
    oss << input;
    std::string word;
    output.clear();
    while (std::getline(oss, word, delimiter)) {
        output.push_back(word);
    }
    return output.size();
}

int split(std::string input, std::vector<std::string>& output, char delimiter)
{
    std::stringstream   oss;
    oss << input;
    std::string word;
    output.clear();
    while (std::getline(oss, word, delimiter)) {
        output.push_back(word);
    }
    return output.size();
}

bool string_sort_proc(std::string s1, std::string s2)
{
    return strcmp(s1.c_str(), s2.c_str()) > 0;
}

}


