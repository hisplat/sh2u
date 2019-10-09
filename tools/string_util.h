
#ifndef __TOOLS_STRINGUTIL_H__
#define __TOOLS_STRINGUTIL_H__
#pragma once

#include <list>
#include <vector>
#include <string>

namespace tools {

std::string trim(std::string input, const char * trim_chars = " \r\n\t");

// split
int split(std::string input, std::list<std::string>& output, char delimiter = ' ');
int split(std::string input, std::vector<std::string>& output, char delimiter = ' ');

bool string_sort_proc(std::string s1, std::string s2);

}


#endif  // __TOOLS_STRINGUTIL_H__

