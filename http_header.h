
#pragma once
#include <string>
#include <map>

namespace simple_httpd {

class HttpHeader {
public:
    std::string get(const std::string& sec);
    HttpHeader& set(const std::string& sec, const std::string& val);
    HttpHeader& set(const std::string& sec, int val);

    std::string obtain();

    void clear() { mMap.clear(); }
    void dump();

private:
    std::map<std::string, std::string> mMap;
};

};

