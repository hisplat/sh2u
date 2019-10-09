
#include "socket.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string>
//#include "Tools.h"

// #include "dump.h"

#define PRINTF(X, ...) ((void)0)
// #define PRINTF  printf

namespace tools {

CSocket::CSocket()
{
    this->m_sock = (SOCKET)(INVALID_SOCKET);
    this->m_eSockType = eSocketType_TCP;
    m_bListening = false;
    m_host.sin_family       = AF_INET;
    m_host.sin_addr.s_addr  = INADDR_ANY;
    m_host.sin_port         = 0;
    m_remote.sin_family     = AF_INET;
    m_remote.sin_addr.s_addr= INADDR_ANY;
    m_remote.sin_port       = 0;
    m_LastActiveTime        = 0;
}

CSocket::CSocket(SOCKETTYPE eSocketType)
{
    this->m_sock = (SOCKET)(INVALID_SOCKET);
    this->m_eSockType = eSocketType;
    memset(&m_remote, 0, sizeof(sockaddr_in));
    memset(&m_host, 0, sizeof(sockaddr_in));
    m_bListening = false;
    m_host.sin_family       = AF_INET;
    m_host.sin_addr.s_addr  = INADDR_ANY;
    m_host.sin_port         = 0;
    m_remote.sin_family     = AF_INET;
    m_remote.sin_addr.s_addr= INADDR_ANY;
    m_remote.sin_port       = 0;
    m_LastActiveTime        = 0;
}

CSocket::CSocket(const CSocket& o)
{
    this->m_sock = o.m_sock;
    this->m_eSockType = o.m_eSockType;
    memcpy(&m_remote, &o.m_remote, sizeof(m_remote));
    memcpy(&m_host, &o.m_host, sizeof(m_host));
    m_bListening = o.m_bListening;
    m_LastActiveTime = o.m_LastActiveTime;
}

CSocket::~CSocket()
{
    Close();
}

bool CSocket::Create(void)
{
    Close();
    if(m_eSockType == eSocketType_TCP)
        m_sock = socket(PF_INET, SOCK_STREAM, 0);
    else if(m_eSockType == eSocketType_UDP)
        m_sock = socket(PF_INET, SOCK_DGRAM, 0);
    else if(m_eSockType == eSocketType_Console)
        m_sock = 0;
    if(m_sock == (SOCKET)(SOCKET_ERROR) || m_sock == (SOCKET)(INVALID_SOCKET)) {
        return false;
        // throw("socket() error.");
    }
    return true;
}

SOCKET CSocket::GetSocket(void)
{
    return m_sock;
}

std::string CSocket::GetRemoteIP(void)
{
    char    ip[4096] = {0};
    inet_ntop(AF_INET, &m_remote.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(m_remote.sin_addr));
}

int CSocket::SetRemoteIP(const char * ipaddr)
{
#if defined(_WIN32) || defined(WIN32)
    m_remote.sin_addr.s_addr = inet_addr(ipaddr);
#else
    if (inet_aton(ipaddr, &m_remote.sin_addr))
        return 0;
    return -1;
#if 0
    std::string errmsg;
    errmsg = std::string("inet_aton: ") + GetErrorMessage();
    throw(errmsg.c_str());
#endif
#endif
}

int CSocket::GetRemotePort(void)
{
    return static_cast<int>(ntohs(m_remote.sin_port));
}

void CSocket::SetRemotePort(int port)
{
    m_remote.sin_port = htons(port);
}

std::string CSocket::GetHostIP(void)
{
    char    ip[4096] = {0};
    inet_ntop(AF_INET, &m_host.sin_addr, ip, sizeof(ip));
    return ip;
    // return static_cast<const char *>(inet_ntoa(m_host.sin_addr));
}

void CSocket::SetHostIP(const char * ipaddr)
{
    if(m_bListening)
        return; // throw("cannot set host ip.");
    m_host.sin_addr.s_addr = inet_addr(ipaddr);
}

int CSocket::GetHostPort(void)
{
    return static_cast<int>(ntohs(m_host.sin_port));
}

void CSocket::SetHostPort(int port)
{
    if(m_bListening)
        return; // throw("cannot set host port.");
    m_host.sin_port = htons(port);
}

void CSocket::SetBroadCast(bool bBroad)
{
    int     onoff = 0;
    if(bBroad)
        onoff = 1;
    if (0 != setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, (char *)&onoff, sizeof(int))) {
        // throw(DisplayError("setsockopt"));
    }
}

void CSocket::SetNonBlock(bool bBlock)
{
    int    onoff = 0;
    if(bBlock)
        onoff = 1;
#if defined(_WIN32) || defined(WIN32)
    /*
       if( 0 != ioctlsocket(m_sock, FIONBIO, (unsigned long *)&onoff) )
       {
       throw(DisplayError("ioctl.FIONBIO"));
       }
       */
    throw("cannot support non-block socket now.");
#else
    if ( 0 != ioctl(m_sock, FIONBIO, &onoff)) {
        // throw(DisplayError("ioctl.FIONBIO"));
    }
#endif
}

int CSocket::Listen(void)
{
    return listen(m_sock, 10);
}

int CSocket::Bind(int port)
{
    if(port != -1)
        SetHostPort(port);
    PRINTF("bind to: %s:%d\n", GetHostIP(), GetHostPort());
    int     op = 1;
    PRINTF("set reuseaddr.\n");
    int ret = setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&op, sizeof(op));
    PRINTF("ret = %d\n", ret);
    if(ret != 0) {
        perror("setsockopt.SO_REUSEADDR");
    }
    ret = bind(m_sock, (struct sockaddr *)&m_host, sizeof(m_host));
    if (ret != 0) {
        // throw(DisplayError("bind"));
    }
    return ret;
}

int CSocket::Connect(const char * ipaddr, int port, int mstimeout)
{
    if(ipaddr != NULL)
        SetRemoteIP(ipaddr);
    if(port != -1)
        SetRemotePort(port);
    PRINTF("connect to %s:%d\n", GetRemoteIP(), GetRemotePort());
    if(mstimeout <= 0) {
        if(connect(m_sock, (struct sockaddr *)&m_remote, sizeof(m_remote)) != 0) {
            // throw(DisplayError("connect"));
            return -1;
        }
    } else {
        timeval tm;
        fd_set  rfds;
        unsigned long ul = 1;
        ioctl(m_sock, FIONBIO, &ul);
        int     error;
        int     len = sizeof(int);
        int     ret;
        if(connect(m_sock, (struct sockaddr *)&m_remote, sizeof(m_remote)) == -1) {
            tm.tv_sec = mstimeout / 1000;
            tm.tv_usec = (mstimeout % 1000) * 1000;
            FD_ZERO(&rfds);
            FD_SET(m_sock, &rfds);

            ret = select(m_sock + 1, NULL, &rfds, NULL, &tm);
            ul = 0;
            ioctl(m_sock, FIONBIO, &ul);

            if(ret <= 0) {
                // throw(DisplayError("connect"));
                return ret;
            } else {
                getsockopt(m_sock, SOL_SOCKET, SO_ERROR, (char *)&error, (socklen_t *)&len);
                if(error != 0) {
                    // throw(DisplayError("connect"));
                    return error;
                }
            }
        }
    }
    return 0;
}

void CSocket::Accept(CSocket& sock)
{
    socklen_t       len;
    SOCKET          fd;
    len = sizeof(m_remote);
    fd = accept(m_sock, (struct sockaddr *)&m_remote, &len);
    PRINTF("accept: %s:%d\n", GetRemoteIP(), GetRemotePort());
    sock.Close();
    sock.m_sock = fd;
    sock.SetRemoteIP(GetRemoteIP().c_str());
    sock.SetRemotePort(GetRemotePort());
}

int CSocket::Recv(void * buf, size_t len, int mstimeout)
{
    if (mstimeout > 0) {
        timeval tm;
        fd_set  rfds;

        tm.tv_sec = mstimeout / 1000;
        tm.tv_usec = (mstimeout % 1000) * 1000;

        FD_ZERO(&rfds);
        FD_SET(m_sock, &rfds);

        int ret = select(m_sock + 1, &rfds, NULL, NULL, &tm);
        if (ret <= 0)
            return ret;
    }

    socklen_t       sl;
    int             ret;
    sl = sizeof(m_remote);
    if(m_eSockType == eSocketType_TCP)
    {
        ret = recv(m_sock, (char *)buf, len, 0);
    }
    else if(m_eSockType == eSocketType_UDP)
    {
        ret = recvfrom(m_sock, (char *)buf, len, 0, (struct sockaddr *)&m_remote, &sl);
    }
    else
    {
#if defined(WIN32) || defined(_WIN32)
        ret = recv(m_sock, (char *)buf, len, 0);
#else
        ret = read(m_sock, (char *)buf, len);
#endif
    }
    if(ret <= 0) {
        OnRemoteDown();
    } else {
        OnReadData(buf, ret);
    }
    return ret;

}

int CSocket::Send(const void * buf, size_t len)
{
    socklen_t       sl;
    sl = sizeof(m_remote);

    if(m_eSockType == eSocketType_TCP)
        return send(m_sock, (const char *)buf, len, 0);
    else if(m_eSockType == eSocketType_UDP)
        return sendto(m_sock, (const char *)buf, len, 0, (struct sockaddr *)&m_remote, sl);
    else
    {
#if defined(WIN32) || defined(_WIN32)
        return send(m_sock, (const char *)buf, len, 0);
#else
        return write(m_sock, (const char *)buf, len);
#endif
    }

}

int CSocket::RecvFrom(char * ipaddr, int& port, void * buf, size_t len, int mstimeout)
{
    int             ret;
    ret = Recv(buf, len, mstimeout);
    strcpy(ipaddr, GetRemoteIP().c_str());
    port = GetRemotePort();
    return ret;
}

int CSocket::SendTo(const char * ipaddr, int port, const void * buf, size_t len)
{
    SetRemoteIP(ipaddr);
    SetRemotePort(port);
    // return Send(buf, len);

    // for thread safe.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

#if defined(_WIN32) || defined(WIN32)
    addr.sin_addr.s_addr = inet_addr(ipaddr);
#else
    if (inet_aton(ipaddr, &addr.sin_addr) == 0) {
        return -1;
    }
#endif

    socklen_t       sl;
    sl = sizeof(addr);

    if(m_eSockType == eSocketType_TCP) {
        return send(m_sock, (const char *)buf, len, 0);
    } else if(m_eSockType == eSocketType_UDP) {
        return sendto(m_sock, (const char *)buf, len, 0, (struct sockaddr *)&addr, sl);
    } else {
#if defined(WIN32) || defined(_WIN32)
        return send(m_sock, (const char *)buf, len, 0);
#else
        return write(m_sock, (const char *)buf, len);
#endif
    }

}

void CSocket::Close()
{
    if(m_sock == (SOCKET)INVALID_SOCKET || m_sock == (SOCKET)SOCKET_ERROR)
        return;
    closesocket(m_sock);
    m_sock                  = (SOCKET)(INVALID_SOCKET);
    m_bListening            = false;
    m_eSockType             = eSocketType_TCP;
    m_host.sin_family       = AF_INET;
    m_host.sin_addr.s_addr  = INADDR_ANY;
    m_host.sin_port         = 0;
    m_remote.sin_family     = AF_INET;
    m_remote.sin_addr.s_addr= INADDR_ANY;
    m_remote.sin_port       = 0;
    m_LastActiveTime        = 0;
}

bool CSocket::IsValid(void)
{
    return (m_sock != (SOCKET)(INVALID_SOCKET));
}

void CSocket::SetLastActiveTime(void)
{
    m_LastActiveTime = time(NULL);
}

time_t CSocket::GetLastActiveTime(void)
{
    return m_LastActiveTime;
}

void CSocket::OnReadData(const void* /*buf*/, int /*len*/)
{
    // to make the compiler happy.
    //    buf = buf;
    //    len = len;
}

const char * CSocket::GetErrorMessage(void)
{
#if defined(WIN32) || defined(_WIN32)
    DWORD dw = WSAGetLastError();
    LPVOID  lpMsgBuf;
    static char   buff[1024];
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR) &lpMsgBuf,
        0,
        NULL
        );
    lstrcpy(buff, (LPCSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return buff;
#else
    return strerror(errno);
#endif

}

const char * CSocket::DisplayError(const char * str)
{
#if defined(WIN32) || defined(_WIN32)
    DWORD dw = WSAGetLastError();
    LPVOID  lpMsgBuf;
    static char   buff[1024];
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPSTR) &lpMsgBuf,
        0,
        NULL
        );
    printf("[WIN32]%s:error %lu, %s\n", str, dw, (char *)lpMsgBuf);
    fflush(stdout);
    lstrcpy(buff, (LPCSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return buff;
#else
    perror(str);
    return strerror(errno);
#endif
}

void CSocket::OnRemoteDown(void)
{
}

void CSocket::OnTimeout(void)
{
}

int CSocket::InitNetwork(void)
{
#if defined(_WIN32) || defined(WIN32)
    WORD            wVersionRequested;
    WSADATA         wsaData;
    int             err;
    wVersionRequested = MAKEWORD( 2, 2 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
        DisplayError("WSAStartup");
        return -1;
    }
#endif
    return 0;
}

} // namespace tools

