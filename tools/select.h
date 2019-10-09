
#pragma once

#include <vector>
#include "socket.h"


namespace tools {

typedef int (* LPFNSELECTCALLBACK)(CSocket * sock);
class CSelect
{
public:
        CSelect();
        virtual ~CSelect();

        void Run();
        void RunOnce();
        void Stop();
        void Clear();
        void SetTimeout(int dwSecs, int dwUSecs = 0){m_timeout = dwSecs; m_utimeout = dwUSecs;}

        void AddReadSocket(CSocket * sock);
        void DelReadSocket(CSocket * sock);
        void AddWriteSocket(CSocket * sock);
        void DelWriteSocket(CSocket * sock);
        void AddErrorSocket(CSocket * sock);
        void DelErrorSocket(CSocket * sock);

        void RegisterReadCallback(LPFNSELECTCALLBACK  callback);
        void RegisterWriteCallback(LPFNSELECTCALLBACK  callback);
        void RegisterErrorCallback(LPFNSELECTCALLBACK  callback);
        void RegisterTimeoutCallback(LPFNSELECTCALLBACK callback);
private:
        LPFNSELECTCALLBACK                      m_read_proc;
        LPFNSELECTCALLBACK                      m_write_proc;
        LPFNSELECTCALLBACK                      m_error_proc;
        LPFNSELECTCALLBACK                      m_timeout_proc;
        std::vector<CSocket *>                  vecRead;
        std::vector<CSocket *>                  vecWrite;
        std::vector<CSocket *>                  vecError;
        std::vector<CSocket *>::iterator        iter;
        std::vector<CSocket *>::iterator        iter1;
        int                                     m_timeout;
        int                                     m_utimeout;
        bool                                    m_bRunning;
        bool                                    m_bVectorChanged;
        CSocket *                               m_controlsocket;
        enum
        {
                SELECT_NONE     = 0,
                SELECT_STOP,
        };
};

} // namespace tools

