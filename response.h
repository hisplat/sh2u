
#pragma once


#include <string>
#include <map>

#include "http_header.h"
#include "tools/buffer.h"


namespace simple_httpd {

class Response {
public:

    void setCode(int code) { mCode = code; }

    HttpHeader& header() { return mHeader; }
    tools::Buffer& content() { return mContent; }

    int code() { return mCode; }

    std::string obtainCode();

    bool loadFile(const std::string& file);

private:
    HttpHeader mHeader;
    tools::Buffer   mContent;
    int mCode;
};
};

