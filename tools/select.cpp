
#include "select.h"
#include <algorithm>
#include <errno.h>

#include <stdio.h>
using namespace std;

namespace tools {

#define LOCALCONTROLPORT        10231
CSelect::CSelect()
{
    m_timeout       = 5;
    m_utimeout      = 0;
    m_bRunning      = false;
    m_read_proc     = NULL;
    m_write_proc    = NULL;
    m_error_proc    = NULL;
    m_timeout_proc  = NULL;
    m_controlsocket = new CSocket(CSocket::eSocketType_UDP);
    m_controlsocket->Create();

    int     port = LOCALCONTROLPORT;
    while (true) {
        m_controlsocket->SetHostPort(port);
        if (m_controlsocket->Bind() == 0) {
            break;
        }
        port++;
    }
}

CSelect::~CSelect()
{
        Clear();
        delete m_controlsocket;
}

void CSelect::RunOnce(void)
{
        CSocket *       sock;
        SOCKET          fd;
        fd_set          rfds;
        fd_set          wfds;
        fd_set          efds;
        struct timeval  tv;
        SOCKET          maxfd = 0;
        int             ret;

        m_bRunning = true;
        for(iter = vecRead.begin(); iter != vecRead.end(); iter++)
        {
                sock = *iter;
                sock->SetLastActiveTime();
        }
        for(iter = vecWrite.begin();iter != vecWrite.end(); iter++)
        {
                sock = *iter;
                sock->SetLastActiveTime();
        }
        for(iter = vecError.begin(); iter != vecError.end(); iter ++)
        {
                sock = *iter;
                sock->SetLastActiveTime();
        }

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        tv.tv_usec = this->m_utimeout;
        tv.tv_sec  = this->m_timeout;

        for(iter = vecRead.begin(); iter != vecRead.end(); iter++)
        {
                sock = *iter;
                fd = sock->GetSocket();
                if(fd > maxfd)
                        maxfd = fd;
                FD_SET(fd, &rfds);
        }
        for(iter = vecWrite.begin();iter != vecWrite.end(); iter++)
        {
                sock = *iter;
                fd = sock->GetSocket();
                if(fd > maxfd)
                        maxfd = fd;
                FD_SET(fd, &wfds);
        }
        for(iter = vecError.begin(); iter != vecError.end(); iter ++)
        {
                sock = *iter;
                fd = sock->GetSocket();
                if(fd > maxfd)
                        maxfd = fd;
                FD_SET(fd, &efds);
        }

        fd = m_controlsocket->GetSocket();
        FD_SET(fd, &rfds);
        if(fd > maxfd)
                maxfd = fd;
        ret = select(maxfd + 1, &rfds, &wfds, &efds, &tv);
        if(!m_bRunning)
                return;
        if(ret == 0)
        {
                if(m_timeout_proc)
                {
                        m_bVectorChanged = false;
                        for(iter = vecRead.begin(); iter != vecRead.end(); iter ++)
                        {
                                sock = *iter;
                                m_timeout_proc(sock);
                                if(m_bVectorChanged)
                                        break;
                        }

                        m_bVectorChanged = false;
                        for(iter = vecWrite.begin(); iter != vecWrite.end(); iter ++)
                        {
                                sock = *iter;
                                m_timeout_proc(sock);
                                if(m_bVectorChanged)
                                        break;

                        }

                        m_bVectorChanged = false;
                        for(iter = vecError.begin(); iter != vecError.end(); iter ++)
                        {
                                sock = *iter;
                                m_timeout_proc(sock);
                                if(m_bVectorChanged)
                                        break;
                        }
                }
        } else if (ret <= 0) {
        } else {
                if(FD_ISSET(m_controlsocket->GetSocket(), &rfds))
                {
                        char    buffer[10];
                        m_controlsocket->Recv(buffer, 1);
                        if(buffer[0] == SELECT_STOP)
                                return;
                }

                if(m_read_proc)
                {
                        m_bVectorChanged = false;
                        for(iter = vecRead.begin(); iter != vecRead.end(); iter++)
                        {
                                sock = *iter;
                                fd = sock->GetSocket();
                                if(FD_ISSET(fd, &rfds))
                                {
                                        if(m_read_proc)
                                                m_read_proc(sock);
                                }
                                if(m_bVectorChanged)
                                        break;
                        }
                }

                if(m_write_proc)
                {
                        m_bVectorChanged = false;
                        for(iter = vecWrite.begin(); iter != vecWrite.end(); iter++)
                        {
                                sock = *iter;
                                fd = sock->GetSocket();
                                if(FD_ISSET(fd, &rfds))
                                {
                                        m_write_proc(sock);
                                }
                                if(m_bVectorChanged)
                                        break;
                        }
                }

                if(m_error_proc)
                {
                        m_bVectorChanged = false;
                        for(iter = vecError.begin(); iter != vecError.end(); iter++)
                        {
                                sock = *iter;
                                fd = sock->GetSocket();
                                if(FD_ISSET(fd, &rfds))
                                {
                                        m_error_proc(sock);
                                }
                                if(m_bVectorChanged)
                                        break;
                        }
                }
        }
}

void CSelect::Run()
{
        m_bRunning = true;
        while(m_bRunning)
                RunOnce();
}

void CSelect::Stop()
{
        m_bRunning = false;
        char    buffer[10];
        buffer[0] = SELECT_STOP;
        // int port = m_controlsocket->GetHostPort();
        m_controlsocket->SendTo("127.0.0.1", m_controlsocket->GetHostPort(), buffer, 1);

}

void CSelect::Clear()
{
        vecRead.clear();
        vecWrite.clear();
        vecError.clear();
}

void CSelect::AddReadSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        if(find(vecRead.begin(), vecRead.end(), sock) != vecRead.end())
                return;
        vecRead.push_back(sock);
        m_bVectorChanged = true;
}

void CSelect::DelReadSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        iter = find(vecRead.begin(), vecRead.end(), sock);
        if(iter == vecRead.end())
                return;
        vecRead.erase(iter);
        m_bVectorChanged = true;
}

void CSelect::AddWriteSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        vecWrite.push_back(sock);
        m_bVectorChanged = true;
}

void CSelect::DelWriteSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        iter = find(vecWrite.begin(), vecWrite.end(), sock);
        if(iter == vecWrite.end())
                return;
        vecWrite.erase(iter);
        m_bVectorChanged = true;
}

void CSelect::AddErrorSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        vecError.push_back(sock);
        m_bVectorChanged = true;
}

void CSelect::DelErrorSocket(CSocket * sock)
{
        if(sock == NULL)
                return;
        iter = find(vecError.begin(), vecError.end(), sock);
        if(iter == vecError.end())
                return;
        vecError.erase(iter);
        m_bVectorChanged = true;
}

void CSelect::RegisterReadCallback(LPFNSELECTCALLBACK  callback)
{
        m_read_proc = callback;
}

void CSelect::RegisterWriteCallback(LPFNSELECTCALLBACK  callback)
{
        m_write_proc = callback;
}

void CSelect::RegisterErrorCallback(LPFNSELECTCALLBACK  callback)
{
        m_error_proc = callback;
}

void CSelect::RegisterTimeoutCallback(LPFNSELECTCALLBACK callback)
{
        m_timeout_proc = callback;
}

} // namespace tools

