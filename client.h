
#pragma once
#include "tools/socket.h"
#include "tools/buffer.h"

#include "response.h"
#include "request.h"

namespace simple_httpd {

class Client : public tools::CSocket {
public:
    Client();
    virtual ~Client();

    virtual void OnReadData(const void * buf, int len);
    virtual void OnRemoteDown(void);

    void doResponse();

private:
    void invoke(const std::string& action, const std::string& method);
    void sendResponse(Response& r);
    void sendError(int errcode, const std::string& msg);

    tools::Buffer   mHeadBuffer;
    tools::Buffer   mContentBuffer;

    Request         mRequest;
    enum {
        S_RESET,
        S_POST,
    } mState;
};

};

