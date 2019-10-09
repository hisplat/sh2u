
#include <sstream>
#include <string.h>
#include <string>

#include "request.h"

#include "tools/logging.h"
#include "tools/string_util.h"


namespace simple_httpd {

void Request::parse(const std::string& buffer)
{
    ILOG() << buffer;

    std::stringstream   oss;
    oss << buffer;
    std::string line;

    while (std::getline(oss, line, '\n')) {
        ILOG() << line;
        if (strncmp(line.c_str(), "POST ", 5) == 0 || strncmp(line.c_str(), "GET ", 4) == 0) {
            std::string::size_type d1 = line.find_first_of(" ");
            std::string::size_type d2 = line.find_last_of(" ");
            mMethod = line.substr(0, d1);
            mUrl = tools::trim(line.substr(d1 + 1, d2 - d1 - 1));
            mProtocol = line.substr(d2 + 1);
            continue;
        }
        std::string::size_type d = line.find_first_of(":");
        std::string n = line.substr(0, d);
        std::string v = tools::trim(line.substr(d + 1));
        mHttpHeader.set(n, v);
    }
    parseUrl();
}

void Request::parseUrl()
{
    mParameter.clear();

    std::string::size_type d = mUrl.find_first_of("#");
    std::string url = mUrl;

    if (d != std::string::npos) {
        mUrlComment = mUrl.substr(d + 1);
        url = mUrl.substr(0, d);
    }

    d = url.find_first_of("?");
    if (d == std::string::npos) {
        mPureUrl = url;
        return;
    }
    mPureUrl = url.substr(0, d);

    std::string tmp = url.substr(d + 1);
    std::list<std::string>  l;
    tools::split(tmp, l, '&');
    for (std::list<std::string>::iterator it = l.begin(); it != l.end(); it++) {
        tmp = *it;
        std::vector<std::string>  vp;
        tools::split(tmp, vp, '=');
        if (vp.size() == 1) {
            mParameter[vp[0]] = "";
        } else if (vp.size() > 1) {
            mParameter[vp[0]] = vp[1];
        }
    }
}

void Request::clear()
{
    mHttpHeader.clear();
}


void Request::dump()
{
    SLOG() << "method: " << mMethod;
    SLOG() << "url: " << mUrl;
    SLOG() << "pureUrl: " << mPureUrl;
    SLOG() << "urlComment: " << mUrlComment;
    SLOG() << "protocol: " << mProtocol;
    mHttpHeader.dump();
    for (std::map<std::string, std::string>::iterator it = mParameter.begin(); it != mParameter.end(); it++) {
        SLOG() << "[parameter] " << it->first << " => " << it->second;
    }
}

bool Request::hasParameter(const std::string& k) const
{
    std::map<std::string, std::string>::const_iterator it = mParameter.find(k);
    return it != mParameter.end();
}

std::string Request::getParameter(const std::string& k) const
{
    std::map<std::string, std::string>::const_iterator it = mParameter.find(k);
    if (it != mParameter.end())
        return it->second;
    return "";
}

};

