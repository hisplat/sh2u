
#include "reliable_udp_socket.h"

#include "logging.h"
#include "dump.h"
#include <string.h>
#include <string>


class SNode : public tools::ReliableUdpSocket::ServerSocket 
{
public:
    virtual void onDataReceived(const char * buffer, int len) {
        RUN_HERE() << "data received: " << buffer;
        if (strcmp(buffer, "END") == 0) {
            close();
        }
    }
};

class SCreator : public tools::ReliableUdpSocket::ServerSocketCreator
{
public:
    virtual tools::ReliableUdpSocket::ServerSocket* create() {
        SNode * n = new SNode();
        RUN_HERE() << "create ServerSocket: " << n;
        return n;
    }
    virtual void remove(tools::ReliableUdpSocket::ServerSocket* s) {
        RUN_HERE() << "delete ServerSocket: " << s;
        delete s;
    }
};

static void* __ser_proc(void* arg)
{
    logging::setThreadLoggingTag(TERMC_GREEN "server" TERMC_NONE);
    tools::ReliableUdpSocket   s;
    SCreator    creator;

    s.registerCreator(&creator);
    s.create();

    s.bind(10234);
    s.listen();

    while (true) {
        int ret = s.accept();
        RUN_HERE() << "accepted. " << ret;
        if (ret != tools::ReliableUdpSocket::R_SUCCESS) {
            break;
        }
    }
    s.close();
    return NULL;
}

static void* __cli_proc(void* arg)
{
    logging::setThreadLoggingTag(TERMC_RED "client" TERMC_NONE);
    tools::ReliableUdpSocket   s;
    SCreator    creator;

    s.registerCreator(&creator);
    s.create();

    ILOG() << "start connectting...";
    int ret = s.connect("127.0.0.1", 10234);
    RUN_HERE() << "connected. " << ret;

#if 0
    std::string str = "this is a test.";
    str += "o";
    s.send(str.c_str(), str.length() + 1);
    str += ".o.";
    s.send(str.c_str(), str.length() + 1);
    str += "o";
    s.send(str.c_str(), str.length() + 1);
#endif
    s.send("END", 4);
    RUN_HERE() << "sent.";

    while (true) {
        char    buffer[65536] = {0};
        int ret = s.recv(buffer, sizeof(buffer));
        RUN_HERE() << ret << " " << buffer;
        if (ret <= 0)
            break;
    }
    RUN_HERE() << "to close.";
    s.close();
    return NULL;
}

int main(int argc, char* argv[])
{
    ILOG() << "test demo.";

    if (argc == 1) {
        pthread_t   ts, tc;
        pthread_create(&ts, NULL, __ser_proc, NULL);
        sleep(1);
        pthread_create(&tc, NULL, __cli_proc, NULL);

        pthread_join(ts, NULL);
        pthread_join(tc, NULL);
    } else {
        if (argv[1][0] == 's') {
            __ser_proc(NULL);
        } else if (argv[1][0] == 'c') {
            __cli_proc(NULL);
        }
    }

 
    return 0;
}

