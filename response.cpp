
#include "response.h"
#include "httpd_config.h"
#include "tools/logging.h"

#include <sstream>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

namespace simple_httpd {

static struct http_code {
    int code;
    const char * desc;
} mHttpCodes[] = {
    { 200, "OK" },
    { 404, "NOT FOUND" },
};

static const char * findHttpCodeDesc(int code)
{
    for (size_t i = 0; i < sizeof(mHttpCodes) / sizeof(mHttpCodes[0]); i++) {
        if (mHttpCodes[i].code == code)
            return mHttpCodes[i].desc;
    }
    return NULL;
}

std::string Response::obtainCode()
{
    const char * desc = findHttpCodeDesc(mCode);

    std::stringstream    oss;
    oss << "HTTP/1.1 ";
    oss << mCode;
    oss << " ";
    if (desc != NULL)
        oss << desc;
    else
        oss << " UNKNOWN";
    oss << "\r\n";
    return oss.str();
}

bool Response::loadFile(const std::string& file)
{
    std::string path(HttpdConfig::instance()->getRootPath());
    path += file;
    if (path.find_last_of("/") == path.length() - 1) {
        path += "index.html";
    }

    FILE* fp = fopen(path.c_str(), "rb");
    if (fp != NULL) {
        setCode(200);

        header().clear();
        header().set("Connection", "close");

        struct stat st;
        stat(path.c_str(), &st);
        header().set("Content-Length", (int)st.st_size);

        std::string::size_type dpos = path.find_last_of("/");
        std::string basename;
        std::string extname;
        if (dpos == std::string::npos) {
            basename = path;
        } else {
            basename = path.substr(dpos + 1);
        }
        dpos = basename.find_last_of(".");
        if (dpos == std::string::npos) {
            extname = "";
        } else {
            extname = basename.substr(dpos);
        }
        header().set("Content-Type", HttpdConfig::instance()->getMetaType(extname));

        char buffer[1024];
        while (!feof(fp)) {
            int ret = fread(buffer, 1, 1024, fp);
            if (ret > 0) {
                content().append(buffer, ret);
            }
        }

        fclose(fp);
        return true;
    }
    ILOG() << path << " not found.";
    return false;
}

}; 

