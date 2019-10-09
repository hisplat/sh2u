
#pragma once

#include <string>
#include <map>
#include "http_header.h"

namespace simple_httpd {

class Request {
public:
    void parse(const std::string& buffer);
    void clear();

    const HttpHeader& header() { return mHttpHeader; }

    std::string getMethod() const { return mMethod; }
    std::string getUrl() const { return mUrl; }
    std::string getPureUrl() const { return mPureUrl; }
    std::string getProtocol() const { return mProtocol; }
    std::string getUrlComment() const { return mUrlComment; }

    std::map<std::string, std::string>  getParameters() const { return mParameter; }

    bool hasParameter(const std::string& k) const;
    std::string getParameter(const std::string& k) const;

    void dump();
private:

    void parseUrl();

    HttpHeader  mHttpHeader;
    std::map<std::string, std::string>    mParameter;

    std::string mMethod;
    std::string mUrl;
    std::string mPureUrl;
    std::string mUrlComment;
    std::string mProtocol;
};
};

