

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include "tools/select.h"
#include "tools/socket.h"
#include "tools/logging.h"
#include "client.h"

static tools::CSelect  selectA;
static tools::CSocket sockL(tools::CSocket::eSocketType_TCP);

int _read_proc(tools::CSocket * sock)
{
    if (sock == &sockL) {
        simple_httpd::Client * c = new simple_httpd::Client();
        sockL.Accept(*c);
        SLOG() << "accept " << c->GetRemoteIP() << ":" << c->GetRemotePort() << ".";
        selectA.AddReadSocket(c);
        return 0;
    }
    char buffer[4096] = {0};
    int ret = sock->Recv(buffer, sizeof(buffer));
    if (ret <= 0) {
        selectA.DelReadSocket(sock);
    }
    return 0;
}

int main()
{
    tools::CSocket::InitNetwork();

    sockL.Create();
    sockL.Bind(10080);
    sockL.Listen();

    selectA.RegisterReadCallback(_read_proc);
    selectA.AddReadSocket(&sockL);
    selectA.Run();

    return 0;
}

