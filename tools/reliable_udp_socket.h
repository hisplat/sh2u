
#pragma once

#include <list>
#include <map>
#include <string>
#include <string.h>
#include "tools/socket.h"
#include "tools/timeout.h"
#include "tools/buffer.h"

namespace tools {

// 不另起线程做发送的重传，而是把重传的动作放在某个特定的循环里。
// 现在是直接放在了 recv() 和 accept() 两个函数里。 这样的话
// 直接把这两个函数中的一个放在 while() 里就可以了。
// 实际上这里面的是在select()超时时去做的判断，与暴露出socket来
// 外面去做超时检测是一回事。
class ReliableUdpSocket : public Timeout
{
public:
    ReliableUdpSocket();
    virtual ~ReliableUdpSocket();

    // result.
    static const int R_SUCCESS = 0;
    static const int R_CHECK_ERRNO = -1;
    static const int R_ILLEGAL_STATE = -2;
    static const int R_TIMEOUT = -3;
    static const int R_BOGUS_PACKET = -4;
    static const int R_INVALID_PACKET = -5;
    static const int R_ACCEPTED = -6;
    static const int R_NOBUFFER = -7;
    static const int R_CLOSED = -8;
    static const int R_MAX_RETRY = -9;

private:
    typedef enum {
        ST_CLOSED = 0,
        ST_LISTEN,
        ST_SYN_RCVD,
        ST_SYN_SENT,
        ST_ESTABLISHED,
        ST_FIN_WAIT_1,
        ST_FIN_WAIT_2,
        ST_CLOSING,
        ST_TIME_WAIT,
        ST_CLOSE_WAIT,
        ST_LAST_ACK,
    } STATE;

    class Packet {
    public:
        class  Header {
        public:
            Header() : magic(0x12345678), rcid(0), lcid(0), sync(0), fin(0), rst(0), ack(0) {}
            bool check() { return magic == 0x12345678; }

            unsigned int magic;
            int rcid;
            int lcid;
            int sequence;
            int acknowledge;

            int sync;
            int fin;
            int rst;
            int ack;
        };
        Packet();
        Packet(const char * buf, size_t size);
        ~Packet() {}

        Header  header;
        Buffer  buffer;
        long long sendtime;
        bool    needAck;
        bool    needSeq;
        int     retry;
    };


public:
    // 单一通信时可以在create的时候直接给个这个类的实例。.
    class DataReceiver {
    public:
        virtual void onDataReceived(const char * buffer, int len) = 0;
    };

    // 在accept时会创建多个ServerSocket用于处理收到的消息。.
    class ServerSocket : public Timeout {
    public:
        ServerSocket();
        virtual ~ServerSocket();

        // 给出的是排好序的数据.
        virtual void onDataReceived(const char * buffer, int len);

        // 加入到发送队列中。
        int send(const char * buffer, int len, bool force = false);
        void close();
        virtual void onClose() {}

    private:
        friend class ReliableUdpSocket;
        STATE   mState;
        int     mSequence;

        // 已收到的排好序的包的序号.
        int     mAcknowledge;

        // cid: 用于区分对端哪一个实例来处理数据。类似于端口的用途.
        int     mLocalCid;
        int     mRemoteCid;

        // SYN时记录的IP地址.
        std::string mSynIP;
        int         mSynPort;

        // 发送接收队列.
        std::list<Packet*>   mSendQueue;
        std::list<Packet*>   mRecvQueue;

        // 已发的最后一个ACK
        int     mLastAcknowledge;
        DataReceiver*   mReceiver;

        // 超时重传的 Jacobson + Karn 算法.
        long long mRTT;
        long long mD;
        long long mRTO;
        void calcRTO(long long ms);

        void changeState(STATE state);
        STATE getState() { return mState; }

        // 根据ACK包清理掉发送队列里的缓存.
        void removingPackets(int ack);

        // 处理收到的包.
        int  parseRUPacket(const char * buffer, int len);

        // 放到发送队列里.
        int  send(Packet* packet, bool force = false);

        // 对收到的包排序以及送给onDataReceived()
        void recv(const char * buffer, int len);

        // 处理发送队列.
        void doSend(CSocket& socket);

        int getRemoteCid() { return mRemoteCid; }
        void setRemoteCid(int c) { mRemoteCid = c; }
        int getLocalCid() { return mLocalCid; }
        void setLocalCid(int c) { mLocalCid = c; }

        void sendAck();
        void sendFin();
        void sendRst();
        void sendFinAck();

        // DEBUG
        void dumpRecvQueue();
        void dumpSendQueue();

        // 每200ms一次的定时器，只用于发送ACK
        void onSendAckTimeout(void* arg);

        // 各种超时.
        void onTimeWaitTimeout(void* arg);
        void onSynRcvdTimeout(void* arg);
        void onCloseTimeout(void* arg);

        bool isSendQueueContainsAck();
    };

    // 继承这个类以创建不同的ServerSocket.
    class ServerSocketCreator {
    public:
        virtual ServerSocket* create();
        virtual void remove(ServerSocket* s);
    };

    void registerCreator(ServerSocketCreator* creator);

public:
    std::string     GetRemoteIP(void) { return mSocket.GetRemoteIP(); }
    int             SetRemoteIP(const char * ipaddr) { return mSocket.SetRemoteIP(ipaddr); }
    int             GetRemotePort(void) { return mSocket.GetRemotePort(); }
    void            SetRemotePort(int port) { mSocket.SetRemotePort(port); }
    std::string     GetHostIP(void) { return mSocket.GetHostIP(); }
    void            SetHostIP(const char * ipaddr) { mSocket.SetHostIP(ipaddr); }
    int             GetHostPort(void) { return mSocket.GetHostPort(); }
    void            SetHostPort(int port) { mSocket.SetHostPort(port); }
    int             GetSocket() { return mSocket.GetSocket(); }

    void create(DataReceiver* receiver = NULL);
    int  listen(void);
    int  bind(int port = -1);
    int  accept(int mstimeout = 0);
    int  recv(int mstimeout = 0);
    int  connect(const char * ipaddr = NULL, int port = -1, int mstimeout = 0);
    int  send(const void * buf, size_t len);
    int  send(Packet* p);
    void close();

    int  getNodeCount() { return mServerSockets.size(); }

public:
    static void dumpPacket(const char * buffer, int len, const char * tag = "Recv", bool force = false);
    static void dumpPacket(Packet* packet, const char * tag = "Recv", bool force = false);

private:
    tools::CSocket                  mSocket;
    std::map<int, ServerSocket*>    mServerSockets;
    ServerSocketCreator *           mCreator;
    int                             mCid;


    // serversocket.
    ServerSocket* mainSocket();
    ServerSocket* createServerSocket();
    void changeState(STATE state, ServerSocket* s = NULL);
    STATE getState(ServerSocket* s = NULL);

    // send
    int Send(Packet* p);
    void doSend();
    int handshake(bool syn, bool ack, ServerSocket* s = NULL);
    void sendRst(const char * ip, int port);

    // 
    void clearServerSockets();

    // main loop
    int readValidPacket(char* buffer, size_t len, std::string& ip, int& port, int mstimeout = 0);

    // Timeout.
    void startSendingProc();
    void onDoSendTimeout(void* arg);
};

} // namespace tools









