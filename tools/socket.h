// CSocket 类
//
// 提供 socket 常用函数封装 ( windows / linux )
//

#pragma once

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <winsock.h>
typedef int             socklen_t;
#define ioctl ioctlsocket
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
typedef int             SOCKET;
#define closesocket(x)  close(x)
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR    (-1)
#endif

#include <time.h>
#include <string>

namespace tools {

class CSocket
{
public:
        typedef enum
        {
                eSocketType_TCP,
                eSocketType_UDP,
                eSocketType_Console,            // Linux only!.
        }SOCKETTYPE;
public:
        CSocket();
        CSocket(SOCKETTYPE eSocketType);
        CSocket(const CSocket& o);
        virtual ~CSocket();

        static int              InitNetwork(void);

        virtual bool            IsValid(void);
        virtual bool            Create(void);
        virtual SOCKET          GetSocket(void);
        virtual std::string     GetRemoteIP(void);
        virtual int             SetRemoteIP(const char * ipaddr);
        virtual int             GetRemotePort(void);
        virtual void            SetRemotePort(int port);
        virtual std::string     GetHostIP(void);
        virtual void            SetHostIP(const char * ipaddr);
        virtual int             GetHostPort(void);
        virtual void            SetHostPort(int port);
        virtual void            SetLastActiveTime(void);
        virtual time_t          GetLastActiveTime(void);
        virtual void            SetBroadCast(bool bBroad);
        virtual const char *    GetErrorMessage(void);
        virtual void            SetNonBlock(bool bBlock);

public:
        virtual void            OnReadData(const void * buf, int len);
        virtual void            OnRemoteDown(void);
        virtual void            OnTimeout(void);

public:
        virtual int             Listen(void);
        virtual int             Bind(int port = -1);
        virtual int             Connect(const char * ipaddr = NULL, int port = -1, int mstimeout = 0);
        virtual void            Accept(CSocket& sock);
        virtual int             Recv(void * buf, size_t len, int mstimeout = 0);
        virtual int             Send(const void * buf, size_t len);
        virtual int             RecvFrom(char * ipaddr, int& port, void * buf, size_t len,int mstimeout = 0);
        virtual int             SendTo(const char * ipaddr, int port, const void * buf, size_t len);
        virtual void            Close();

protected:
        static const char *     DisplayError(const char * str);

private:
        SOCKET          m_sock;
        // SOCKET          m_accept;
        SOCKETTYPE      m_eSockType;
        sockaddr_in     m_remote;
        sockaddr_in     m_host;
        bool            m_bListening;
        time_t          m_LastActiveTime;
};

} // namespace tools

