

#include "client.h"
#include "response.h"
#include "request.h"
#include "httpd_config.h"

#include "tools/logging.h"
#include "tools/dump.h"
#include "tools/string_util.h"

#include <string.h>
#include <vector>

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

typedef void (*lpfnAction)(const simple_httpd::Request&, simple_httpd::Response&);

namespace simple_httpd {
Client::Client()
    : tools::CSocket()
{
    mState = S_RESET;
}

Client::~Client()
{
}

void Client::OnReadData(const void * buf, int len)
{
    if (mState == S_RESET) {
        mHeadBuffer.append((const char*)buf, len);
        // ILOG() << mHeadBuffer;
        void * end = memmem(buf, len, "\r\n\r\n", 4);
        if (end != NULL) {
            int pos = mHeadBuffer.length() - (len - ((char*)end - (char*)buf));
            // ILOG() << "pos = " << pos;
            mHeadBuffer.at(pos) = '\0';
            // ILOG() << mHeadBuffer;
            end = (char *)end + 4;
            mContentBuffer.assign(0);
            mRequest.parse(mHeadBuffer.buffer());
            if (mRequest.getMethod() == "POST") {
                mState = S_POST;
            } else {
                doResponse();
            }
        }
    } else if (mState == S_POST) {
        mContentBuffer.append((const char*)buf, len);
    } else {
    }
    // tools::dump(buf, len);
}

void Client::OnRemoteDown(void)
{
    // SLOG() << mBuffer;
}

void Client::doResponse()
{
    mRequest.dump();

    Response r;
    if (r.loadFile(mRequest.getPureUrl())) {
        sendResponse(r);
        return;
    }
#if 0
    std::string path(HttpdConfig::instance()->getRootPath());
    path += mRequest.getPureUrl();
    if (path.find_last_of("/") == path.length() - 1) {
        path += "index.html";
    }

    FILE* fp = fopen(path.c_str(), "rb");
    if (fp != NULL) {
        Response r;

        r.setCode(200);
        r.header().set("Connection", "close");

        struct stat st;
        stat(path.c_str(), &st);
        r.header().set("Content-Length", (int)st.st_size);

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
        r.header().set("Content-Type", HttpdConfig::instance()->getMetaType(extname));

        sendResponse(r);
        char buffer[1024];
        while (!feof(fp)) {
            int ret = fread(buffer, 1, 1024, fp);
            if (ret > 0) {
                Send(buffer, ret);
            }
        }

        fclose(fp);
        return;
    }
    ILOG() << path << " not found.";
#endif

    std::string action;
    std::string method;

    std::string url = mRequest.getPureUrl();
    std::vector<std::string> va;
    tools::split(url, va, '/');
    if (va.size() == 2) {
        action = va[1];
        method = "";
    } else if (va.size() == 3) {
        action = va[1];
        method = va[2];
    } else {
        sendError(404, "Page Not Found");
        return;
        // FATAL() << "not implemented yet.";
    }

    invoke(action, method);
}


void Client::sendError(int errcode, const std::string& msg)
{
    Response r;
    r.setCode(errcode);
    r.header().set("Connection", "close");
    r.header().set("Content-Length", msg.length());
    r.content().append(msg.c_str(), msg.length());
    sendResponse(r);
    // Send("\r\n", 2);
}

void Client::invoke(const std::string& action, const std::string& method)
{
    char devname[PATH_MAX];
    char *filename;
    DIR *dir;
    struct dirent *de;

    std::string path(HttpdConfig::instance()->getRootPath());
    path += "/actions";

    dir = opendir(path.c_str());
    if (dir == NULL) {
        ILOG() << "no actions dir.";
        sendError(404, "Page Not Found");
        return;
    }
    strcpy(devname, path.c_str());
    filename = devname + strlen(devname);
    *filename++ = '/';

    bool found = false;
    while ((de = readdir(dir))) {
        if (de->d_name[0] == '.' && (de->d_name[1] == '\0' || (de->d_name[1] == '.' && de->d_name[2]== '\0')))
            continue;
        if (strstr(de->d_name, ".so") == NULL)
            continue;
        strcpy(filename, de->d_name);
        // ILOG() << "devname = " << devname;
        void * handle = dlopen(devname, RTLD_NOW | RTLD_GLOBAL);
        if (handle == NULL) {
            ILOG() << dlerror();
            continue;
        }

        // find this function:
        //      void action_action_method(const simple_httpd::Request&, simple_httpd::Response&);
        std::string sym = "action_";
        sym += action;
        if (method != "") {
            sym += "_";
            sym += method;
        }
        lpfnAction a = (lpfnAction)dlsym(handle, sym.c_str());
        if (a == NULL) {
            ILOG() << dlerror();
            continue;
        }

        ILOG() << sym << "found from so: " << devname;
        Response r;
        r.setCode(200);
        r.header().set("Connection", "close")
            .set("Content-Type", "text/html");

        a(mRequest, r);
        dlclose(handle);

        r.header().set("Content-Length", r.content().length());

        sendResponse(r);
        found = true;
        break;
    }
    closedir(dir);
    if (!found) {
        sendError(404, "Page Not Found");
    }
}

void Client::sendResponse(Response& r)
{
    std::string h = r.obtainCode();
    ILOG() << h;
    Send(h.c_str(), h.length());

    h = r.header().obtain();
    ILOG() << h;
    Send(h.c_str(), h.length());

    // ILOG() << r.content();
    Send(r.content().buffer(), r.content().length());
}

};


