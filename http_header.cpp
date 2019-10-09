
#include <sstream>
#include <string.h>
#include <string>
#include <sstream>

#include "http_header.h"

#include "tools/logging.h"
#include "tools/string_util.h"


namespace simple_httpd {

std::string HttpHeader::get(const std::string& sec)
{
    std::map<std::string, std::string>::iterator it = mMap.find(sec);
    if (it != mMap.end())
        return it->second;
    return ""; 
}

HttpHeader& HttpHeader::set(const std::string& sec, const std::string& val)
{
    mMap[sec] = val;
    return *this;
}

HttpHeader& HttpHeader::set(const std::string& sec, int val)
{
    std::stringstream oss;
    oss << val;
    return set(sec, oss.str());
}

std::string HttpHeader::obtain()
{
    std::stringstream oss;
    std::map<std::string, std::string>::iterator it;

    for (it = mMap.begin(); it != mMap.end(); it++) {
        oss << it->first
            << ": "
            << it->second
            << "\r\n";
    }
    oss << "\r\n";
    return oss.str();
}

void HttpHeader::dump()
{
    for (std::map<std::string, std::string>::iterator it = mMap.begin(); it != mMap.end(); it++) {
        SLOG() << "[http]" << it->first << " => " << it->second;
    }
}


};

