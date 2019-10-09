#pragma once

#include <string>
#include <map>

namespace simple_httpd {

class HttpdConfig {
public:
    ~HttpdConfig() {}
    int getSitePort() { return 10080; }
    const char * getRootPath() { return "."; }
    static HttpdConfig* instance();

    std::string getMetaType(const std::string& extname);
private:
    HttpdConfig();
    std::map<std::string, std::string> mMetaMap;
};
} // namespace simple_httpd

