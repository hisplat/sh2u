
#include "reliable_udp_socket.h"

#include <string>
#include <algorithm>
#include <string.h>
#include <math.h>
#include "logging.h"
#include "dump.h"

#define SENDSIZE    100
#define RECVSIZE    100

// 超时时间。
#define DEFAULT_RTT 100
#define RTO_FACTOR  (7.0f / 8.0f)
#define MIN_RTO     40
#define MAX_RTO     1280

// 最大重发次数。 如果设为0则无限制重发。
#define MAX_RETRYTIMES  20

// TIME_WAIT 状态等待时间
// MSL先用着RTO吧
#define MAXIMUM_SEGMENT_LIFETIME    mRTO
#define TIMEWAIT_TIMEOUT    (MAXIMUM_SEGMENT_LIFETIME * 2)

// SYN_RCVD状态超时时间
#define SYNRCVD_TIMEOUT     10000
#define CLOSE_TIMEOUT       2000

// LOG:
// #define DUMP_PACKET
// #define DUMP_CONTENT
// #define DUMP_PROCESS

// #define SEND_AND_WAIT
// #define ONE_PACKET_PER_LOOP

namespace tools {

static long long time_now()
{
    struct timeval  now;
    gettimeofday(&now, NULL);
    return now.tv_sec * 1000 + now.tv_usec / 1000;
}

ReliableUdpSocket::Packet::Packet()
    : sendtime(0)
    , retry(0)
{
    header.rcid = 0;
    header.lcid = 0;
    sendtime = time_now();
    needAck = true;
    needSeq = true;
}

ReliableUdpSocket::Packet::Packet(const char * buf, size_t size)
    : retry(0)
{
    buffer.assign(buf, size);
    sendtime = time_now();
    needAck = true;
    needSeq = true;
}

ReliableUdpSocket::ServerSocket::ServerSocket()
    : mState(ST_CLOSED)
    , mSequence(0)
    , mAcknowledge(0)
    , mLocalCid(0)
    , mRemoteCid(0)
    , mLastAcknowledge(0)
    , mReceiver(NULL)
    , mRTT(DEFAULT_RTT)
    , mD(0)
    , mRTO(MIN_RTO)
{
    mSequence = rand() % 1000;
    mRTT = DEFAULT_RTT;
    mD = 0;
}

ReliableUdpSocket::ServerSocket::~ServerSocket()
{
    std::list<Packet*>::iterator it;
    for (it = mSendQueue.begin(); it != mSendQueue.end(); it++) {
        Packet* packet = *it;
        delete packet;
    }
    for (it = mRecvQueue.begin(); it != mRecvQueue.end(); it++) {
        Packet* packet = *it;
        delete packet;
    }
    mSendQueue.clear();
    mRecvQueue.clear();
}

int ReliableUdpSocket::ServerSocket::send(const char * buffer, int len, bool force)
{
    if (getState() == ST_CLOSED) {
        return R_ILLEGAL_STATE;
    }

    // if (getState() != ST_ESTABLISHED) {
    //     FATAL();    // Fixme.
    //     return R_ILLEGAL_STATE;
    // }

    if (!force && mSendQueue.size() >= SENDSIZE) {
        return R_NOBUFFER;
    }
    Packet* packet = new Packet(buffer, len);
    packet->header.ack = 1;
    int ret = send(packet, force);
    if (ret != R_SUCCESS) {
        delete packet;
    }
    return ret;
}

int ReliableUdpSocket::ServerSocket::send(ReliableUdpSocket::Packet* packet, bool force)
{
    if (packet == NULL) {
        return R_INVALID_PACKET;
    }
    if (!force && mSendQueue.size() >= SENDSIZE) {
        return R_NOBUFFER;
    }

    // 如果不需要回包，类似纯ACK，则直接用上一个Sequence就好.
    if (packet->needSeq) {
        mSequence++;
    }
    packet->header.sequence = mSequence;
    packet->sendtime = time_now();
    packet->header.lcid = getLocalCid();
    packet->header.rcid = getRemoteCid();
    mSendQueue.push_back(packet);
    return R_SUCCESS;
}

void ReliableUdpSocket::ServerSocket::onDataReceived(const char * buffer, int len)
{
    if (mReceiver != NULL) {
        mReceiver->onDataReceived(buffer, len);
    }
    // SLOG() << "Data received.";
    // tools::dump(buffer, len);
}

void ReliableUdpSocket::ServerSocket::calcRTO(long long ms)
{
    // RTT = aRTT + (1-a)M
    // D   = aD + (1-a)|RTT-M|
    // RTO = RTT + 4D
    if (ms < 0)
        ms = 0;
    mRTT = (long long)(RTO_FACTOR * mRTT + (1.0f - RTO_FACTOR) * ms);
    mD   = (long long)(RTO_FACTOR * mD + (1.0f - RTO_FACTOR) * fabsf(mRTT - ms));
    mRTO = mRTT + 4 * mD;
    if (mRTO < MIN_RTO)
        mRTO = MIN_RTO;
    if (mRTO > MAX_RTO)
        mRTO = MAX_RTO;
    // ILOG() << "ms = " << ms << ", RTO changed to " << mRTO;
}

void ReliableUdpSocket::ServerSocket::removingPackets(int ack)
{
    if (mSendQueue.empty())
        return;

    std::list<Packet*>::iterator it;
    for (it = mSendQueue.begin(); it != mSendQueue.end(); ) {
        Packet* packet = *it;
        if (packet->header.sequence <= ack) {

            // 计算RTO
            if (packet->header.sequence == ack) {
                if (packet->retry == 1) {
                    calcRTO(time_now() - packet->sendtime);
                }
            }

            ReliableUdpSocket::dumpPacket(packet, "Removing");
            delete packet;
            it = mSendQueue.erase(it);
        } else {
            it++;
        }
    }
#ifdef DUMP_PROCESS
    SLOG() << mSendQueue.size() << " packet(s) left in sendding queue of lcid " << getLocalCid() << ".";
#endif
}

void ReliableUdpSocket::ServerSocket::changeState(STATE state)
{
    const char * _table[] = {"CLOSED", "LISTEN", "SYN_RCVD", "SYN_SENT", 
        "ESTABLISHED", "FIN_WAIT_1", "FIN_WAIT_2", "CLOSING", "TIME_WAIT",
        "CLOSE_WAIT", "LASTACK"};

    SLOG() << "changing " << getLocalCid() << " state from " << _table[mState] << " to " << _table[state];
    if (state == mState) {
        return;
    }
    mState = state;
    if (state == ST_LISTEN) { 
        clearTimeout(timeout_selector(ReliableUdpSocket::ServerSocket::onSendAckTimeout));
    } else {
        if (!isScheduled(timeout_selector(ReliableUdpSocket::ServerSocket::onSendAckTimeout)))
            setTimeout(200, timeout_selector(ReliableUdpSocket::ServerSocket::onSendAckTimeout));
    }
    if (state == ST_SYN_RCVD) {
        if (!isScheduled(timeout_selector(ReliableUdpSocket::ServerSocket::onSynRcvdTimeout)))
            setTimeout(SYNRCVD_TIMEOUT, timeout_selector(ReliableUdpSocket::ServerSocket::onSynRcvdTimeout));
    } else {
        clearTimeout(timeout_selector(ReliableUdpSocket::ServerSocket::onSynRcvdTimeout));
    }

    if (state == ST_TIME_WAIT) {
        if (!isScheduled(timeout_selector(ReliableUdpSocket::ServerSocket::onTimeWaitTimeout)))
            setTimeout(TIMEWAIT_TIMEOUT, timeout_selector(ReliableUdpSocket::ServerSocket::onTimeWaitTimeout));
        clearTimeout(timeout_selector(ReliableUdpSocket::ServerSocket::onCloseTimeout));
    }
    if (state == ST_CLOSED) {
        onClose();
    }
#if 0
    if (state == ST_CLOSED) {
        mSocket.Close();
    } else {
        if (!mSocket.IsValid()) {
            mSocket.Create();
        }
    }
#endif
}

void ReliableUdpSocket::ServerSocket::dumpRecvQueue()
{
#ifdef DUMP_PACKET
    // test print.
    if (true) {
        logging::LogMessage logger;
        logger.stream() << "recv buffer: (" << mRecvQueue.size() << ") {";

        std::list<Packet*>::iterator it;
        for (it = mRecvQueue.begin(); it != mRecvQueue.end(); it++) {
            if (it != mRecvQueue.begin()) {
                logger.stream() << ", ";
            }
            logger.stream() << (*it)->header.sequence;
        }
        logger.stream() << "}";
    }
#endif
}

void ReliableUdpSocket::ServerSocket::dumpSendQueue()
{
    // test print.
    if (true) {
        logging::LogMessage logger;
        logger.stream() << "send queue: (" << mRecvQueue.size() << ") {";

        std::list<Packet*>::iterator it;
        for (it = mRecvQueue.begin(); it != mRecvQueue.end(); it++) {
            if (it != mRecvQueue.begin()) {
                logger.stream() << ", ";
            }
            logger.stream() << (*it)->header.sequence;
        }
        logger.stream() << "}";
    }
}

void ReliableUdpSocket::ServerSocket::recv(const char * buffer, int len)
{
    if (buffer == NULL || len <= 0) {
        return;
    }

    // 缓冲区满了直接扔掉。.
    if (mRecvQueue.size() >= RECVSIZE) {
        return;
    }

    Packet * packet = new Packet();
    if (len > (int)sizeof(Packet::Header)) {
        packet->buffer.assign(buffer + sizeof(Packet::Header), len - (int)sizeof(Packet::Header));
    }
    packet->header = *(Packet::Header*)buffer;

    // 插到接收缓冲区里去排序。
    if (mRecvQueue.empty()) {
        mRecvQueue.push_back(packet);
    } else if (packet->header.sequence < mRecvQueue.front()->header.sequence) {
        mRecvQueue.push_front(packet);
    } else {
        std::list<Packet*>::iterator it;
        for (it = mRecvQueue.begin(); it != mRecvQueue.end(); it++) {
            if ((*it)->header.sequence == packet->header.sequence) {
                // 队列中有这个包。。说明在这之前还有其它包没到达。。直接扔掉.
                // SLOG() << "duplicated packet. ignore.";
                delete packet;
                // Fixme: test sendAck
                // sendAck();
                return;
            }
            if ((*it)->header.sequence > packet->header.sequence) {
                break;
            }
        }
        if (it == mRecvQueue.end()) {
            mRecvQueue.push_back(packet);
        } else {
            mRecvQueue.insert(it, packet);
        }
    }
    // dumpRecvQueue();

    while (!mRecvQueue.empty()) {
        Packet* p = mRecvQueue.front();
        ReliableUdpSocket::dumpPacket(p, "Check");
#ifdef DUMP_PACKET
        SLOG() << "RecvQueue: seq: " << p->header.sequence << ", ack: " << mAcknowledge;
#endif
        // 乱序的包先放在缓存中.
        if (p->header.sequence != mAcknowledge + 1) {
            // Fixme: 
            // SLOG() << "disorder. expect " << mAcknowledge + 1 << ", meet " << p->header.sequence;
            // dumpRecvQueue();
            // sendAck();
            break;
        }
        if (!p->buffer.isEmpty()) {
            onDataReceived(p->buffer, p->buffer.length());
        }
        mAcknowledge = p->header.sequence;
        // SLOG() << "mAcknowledge changing to " << mAcknowledge;

        /*
        // 关闭时状态切换的两种情况：
        // 1. 一方主动关闭, 主动的一方会进入TIME_WAIT状态:
        //         CLOSE_WAIT          LAST_ACK      CLOSED
        // ------------o----------------o---------------o------------------
        //            / \                \             /
        //           /   \                \           /
        //          /     \            FIN \         / FIN ACK
        //     FIN /       \ FIN ACK        \       /
        //        /         \                \     /
        //       /           \                \   /
        //      /             \                \ /
        // ----o---------------o----------------o---------------o----------
        //  FIN_WAIT_1      FIN_WAIT_2      TIME_WAIT        CLOSED
        //  (主动关闭)                     (2MSL 超时)
        //
        //
        // 2. 双方同时主动关闭，双方同时进入TIME_WAIT状态:
        //   FIN_WAIT_1   CLOSING      TIME_WAIT    CLOSED
        // -----o-----------o-------------o------------o---------
        //       \         / \           /
        //    FIN \       /   \         /
        //         \     /     \       /
        //          \   /       \     /
        //           \ /         \   /
        //            X           \ /
        //           / \           X
        //          /   \         / \
        //         /     \       /   \
        //     FIN/       \     /     \
        //       /         \   /       \
        //      /           \ /         \
        // ----o-------------o-----------o-------------o--------
        //  FIN_WAIT_1    CLOSING    TIME_WAIT      CLOSED
        //  (主动关闭)              (2MSL 超时)
        */

        // FIN: 
        if (p->header.fin == 1 && p->header.ack == 0) {
            switch (mState) {
            case ST_ESTABLISHED:
                // 两个状态直接连续切换吧.
                changeState(ST_CLOSE_WAIT);
                sendFinAck();
                changeState(ST_LAST_ACK);
                // if (mSendQueue.empty())
                sendFin();
                break;
            case ST_FIN_WAIT_1:
                // 同时收到FIN，等待对方ACK.
                changeState(ST_CLOSING);
                break;
            case ST_FIN_WAIT_2:
            case ST_TIME_WAIT:
                // TIME_WAIT状态.
                // 对端没收到Last ACK时会重发FIN. 这里需要维持回复ACK的逻辑。.
                changeState(ST_TIME_WAIT);
                sendFinAck();
                break;
            default:
                RUN_HERE() << "state = " << mState;
                FATAL() << "Fixme.";
            }
        } else if (p->header.fin == 1 && p->header.ack == 1) {
            // FIN ACK: 虽然不需要回包，但是会影响状态切换.
            switch (mState) {
            case ST_FIN_WAIT_1:
                changeState(ST_FIN_WAIT_2);
                break;
            case ST_LAST_ACK:
                changeState(ST_CLOSED);
                break;
            case ST_CLOSING:
                // 两端同时进入到TIME_WAIT状态。。.
                changeState(ST_TIME_WAIT);
                break;
            case ST_ESTABLISHED:
                // 直接当作 ACK 包处理，扔.
                // RUN_HERE();
                FATAL();
                return; // R_SUCCESS;
            default:
                // 其它状态下收到FIN ACK应该扔掉吧...先放个FATAL看看有没什么错误.
                FATAL();
            }
        } else {
            sendAck();
        }

        delete p;
        mRecvQueue.pop_front();
    }
    // dumpRecvQueue();
}

int ReliableUdpSocket::ServerSocket::parseRUPacket(const char * buffer, int len)
{
    int headerlen = (int)sizeof(Packet::Header);
    if (len < headerlen) {
        return R_INVALID_PACKET;
    }

    ReliableUdpSocket::dumpPacket(buffer, len, "Parse");

    Packet::Header* header = (Packet::Header*)buffer;

    // 三次握手包不参与排序。
    if (mState == ST_SYN_RCVD) {
        // 不走ACK了。。收到ACK就直接应答。
        if (header->ack == 1) {
            changeState(ST_ESTABLISHED);
        }
        // 理论上说握手的第三个包丢失时应该直接发RST.
        // 但是会造成多次重连。。 暂且先无视掉第三个ACK包.
#if 0
        // 如果三次握手的第三个包ACK丢失时应该直接RST.
        if (header->sync == 1 || header->fin == 1 || header->rst == 1 || header->ack == 0 || header->acknowledge != mSendQueue.front()->header.sequence) {
            // ReliableUdpSocket::dumpPacket(buffer, len);
            ILOG() << "bogus packet.";
            ILOG() << header->acknowledge << ", " << mSendQueue.front()->header.sequence;
            // FATAL() << "should send RST";
            sendRst();
            changeState(ST_CLOSED);
            return R_INVALID_PACKET;
        }
        // 握手时收到 ACK 的处理
        changeState(ST_ESTABLISHED);
        removingPackets(header->acknowledge);
        return R_ACCEPTED;
#endif
    } else if (mState == ST_SYN_SENT) {
        // 收到 SYN ACK. 清掉队列中的SYN就可以了。
        if (header->sync == 1 && header->ack == 1 && header->rst == 0 && header->fin == 0 && len == headerlen) {
            removingPackets(header->acknowledge);
        }
        return R_SUCCESS;
    }

    // 收到ack后直接清掉发送队列中相应的包.
    if (header->ack == 1) {
        removingPackets(header->acknowledge);
    }

    // 没有数据的纯ACK不影响Sequence，所以不需要排序或者去重, 清掉发送队列中的包后可以直接扔掉。
    // Fixme: 看看是不是需要把needAck放到Header里来做这个逻辑。
    if (len == (int)sizeof(Packet::Header) && header->sync == 0 && header->fin == 0 && header->rst == 0 && header->ack == 1) {
        return R_SUCCESS;
    }

    // 修改: FIN ACK也给个Seq.
#if 0
    // FIN ACK 虽然不需要回包，但是会影响状态切换.
    if (len == (int)sizeof(Packet::Header) && header->sync == 0 && header->fin == 1 && header->rst == 0 && header->ack == 1) {
        switch (mState) {
        case ST_FIN_WAIT_1:
            changeState(ST_FIN_WAIT_2);
            break;
        case ST_LAST_ACK:
            changeState(ST_CLOSED);
            break;
        case ST_CLOSING:
            // 两端同时进入到TIME_WAIT状态。。.
            changeState(ST_TIME_WAIT);
            break;
        case ST_ESTABLISHED:
            // 直接当作 ACK 包处理，扔.
            // RUN_HERE();
            return R_SUCCESS;
        default:
            // 其它状态下收到FIN ACK应该扔掉吧...先放个FATAL看看有没什么错误.
            FATAL();
        }
        return R_SUCCESS;
    }
#endif

    // 重复的包直接扔掉..200ms后自会有ACK.
    if (header->sequence <= mAcknowledge) {
        // SLOG() << "duplicated packet. ignore...";
        return R_SUCCESS;
    }

    // 正常的数据包以及FIN包都扔去排序处理。
    recv(buffer, len);
    return R_SUCCESS;
}
        
bool ReliableUdpSocket::ServerSocket::isSendQueueContainsAck()
{
    if (!mSendQueue.empty()) {
        std::list<Packet*>::iterator it;
        for (it = mSendQueue.begin(); it != mSendQueue.end(); it++) {
            Packet* p = *it;
            if (p->header.ack == 1 && p->sendtime + mRTO < time_now() + 200)
                return true;
        }
    }
    return false;
}

void ReliableUdpSocket::ServerSocket::onCloseTimeout(void* arg)
{
    changeState(ST_CLOSED);
}

void ReliableUdpSocket::ServerSocket::onSendAckTimeout(void* arg)
{
    if (mLastAcknowledge < mAcknowledge && !isSendQueueContainsAck()) {
        sendAck();
    }
    setTimeout(200, timeout_selector(ReliableUdpSocket::ServerSocket::onSendAckTimeout));
}

void ReliableUdpSocket::ServerSocket::onTimeWaitTimeout(void* arg)
{
    changeState(ST_CLOSED);
}

void ReliableUdpSocket::ServerSocket::onSynRcvdTimeout(void* arg)
{
    RUN_HERE() << "syn timedout.";
    changeState(ST_CLOSED);
}

void ReliableUdpSocket::ServerSocket::sendAck()
{
    Packet* packet = new Packet();
    packet->header.sync = 0;
    packet->header.ack = 1;
    packet->header.rst = 0;
    packet->header.fin = 0;
    packet->needAck = false;
    packet->needSeq = false;
    send(packet);
}

void ReliableUdpSocket::ServerSocket::sendFinAck()
{
    Packet* packet = new Packet();
    packet->header.sync = 0;
    packet->header.ack = 1;
    packet->header.rst = 0;
    packet->header.fin = 1;
    packet->needAck = false;
    packet->needSeq = true;
    send(packet);
}

void ReliableUdpSocket::ServerSocket::sendFin()
{
    Packet* packet = new Packet();
    packet->header.sync = 0;
    packet->header.ack = 0;
    packet->header.fin = 1;
    packet->header.rst = 0;
    send(packet);
}

void ReliableUdpSocket::ServerSocket::sendRst()
{
    Packet* packet = new Packet();
    packet->header.sync = 0;
    packet->header.ack = 0;
    packet->header.fin = 0;
    packet->header.rst = 1;
    packet->needAck = false;
    packet->needSeq = false;
    send(packet);
}

void ReliableUdpSocket::ServerSocket::doSend(tools::CSocket& socket)
{
    if (mSendQueue.empty())
        return;

    if (mRTO == 0) FATAL();
    long long& timeout = mRTO;

    // 就算多个包在队列里，也只处理第一个。第一个超时包没重发完的情况下不重发后面的。
    bool    hasTimeoutPacketResent = false;

    std::list<Packet*>::iterator it = mSendQueue.begin();
#ifndef SEND_AND_WAIT
    while (it != mSendQueue.end()) {
#endif
        Packet* packet = *it;

        if (packet->retry <= 0 || (!hasTimeoutPacketResent && packet->sendtime + timeout <= time_now())) {
            packet->retry++;
            packet->sendtime = time_now(); //  + 5000;

            // Karn算法： 当重传时RTO翻倍，重传达到一定次数时直接断链。.
            if (packet->retry > 1) {
                // SLOG() << "resend: " << packet->header.sequence << ", " << packet->retry << " times, RTO = " << timeout;
                dumpPacket(packet, "Resend");
                hasTimeoutPacketResent = true;
                timeout = timeout * 2;
                if (timeout > MAX_RTO)
                    timeout = MAX_RTO;
                // ReliableUdpSocket::dumpPacket(packet, "Resend");
            }
            if (MAX_RETRYTIMES > 0 && packet->retry > MAX_RETRYTIMES) {
                SLOG() << "reach max retry times. close.";
                changeState(ST_CLOSED);
                return;
            }

            // 发送ACK时带上acknowledge.
            if (packet->header.ack == 1) {
                packet->header.acknowledge = mAcknowledge;
                mLastAcknowledge = mAcknowledge;
            }

            char    buffer[65536] = {0};
            int ret = sizeof(Packet::Header);
            memcpy(buffer, &packet->header, ret);
            if (packet->buffer.length() > 0) {
                int len = packet->buffer.length();
                if (len > (int)sizeof(buffer) - ret) {
                    len = sizeof(buffer) - ret;
                }
                memcpy(buffer + ret, packet->buffer, len);
                ret += len;
            } else {
                // ret += 1;
            }
            dumpPacket(buffer, ret, "Send");
            socket.SendTo(mSynIP.c_str(), mSynPort, buffer, ret);

            // 不需ACK回应的包即为不需要重传的包，发完了直接扔掉.
            if (ret == (int)sizeof(Packet::Header) && packet->needAck == false /* packet->header.sync == 0 && packet->header.ack == 1 */) {
                dumpPacket(packet, "Drop");
                delete packet;
                it = mSendQueue.erase(it);
            } else {
                it++;
            }
#ifdef DUMP_PROCESS
            SLOG() << mSendQueue.size() << " packet(s) left in sendding queue of lcid " << getLocalCid() << ".";
#endif
#ifdef ONE_PACKET_PER_LOOP
            break;
#else
            // usleep(10);
#endif
        } else {
            it++;
            hasTimeoutPacketResent = true;
        }
#ifndef SEND_AND_WAIT
    }
#endif
}

void ReliableUdpSocket::ServerSocket::close()
{
    // 与TCP稍有区别, 这里直接发个Fin报文，不把Fin标志与数据报文合并了.
    // if (mSendQueue.empty())
    sendFin();
    changeState(ST_FIN_WAIT_1);

    setTimeout(CLOSE_TIMEOUT, timeout_selector(ReliableUdpSocket::ServerSocket::onCloseTimeout));
}

ReliableUdpSocket::ServerSocket* ReliableUdpSocket::ServerSocketCreator::create()
{
    return new ServerSocket();
}

void ReliableUdpSocket::ServerSocketCreator::remove(ServerSocket* s)
{
    delete s;
}

static ReliableUdpSocket::ServerSocketCreator  DefaultCreator;

///////////////////////////////////////////////////////////////////////////////////

ReliableUdpSocket::ReliableUdpSocket()
    : mSocket(CSocket::eSocketType_UDP)
    , mCreator(NULL)
{
    mCid = 0;
    registerCreator(&DefaultCreator);

    // ServerSocket* s = DefaultCreator.create();
    // mServerSockets[0] = s;
}

ReliableUdpSocket::~ReliableUdpSocket()
{
    clearServerSockets();
}

ReliableUdpSocket::ServerSocket* ReliableUdpSocket::createServerSocket()
{
    if (mCreator == NULL) {
        mCreator = &DefaultCreator;
    }
    ServerSocket* s = mCreator->create();
    return s;
}

ReliableUdpSocket::ServerSocket* ReliableUdpSocket::mainSocket()
{
    std::map<int, ServerSocket*>::iterator it = mServerSockets.find(0);
    if (it == mServerSockets.end()) {
        return NULL;
    }
    return it->second;
}

void ReliableUdpSocket::changeState(STATE state, ServerSocket* s)
{
    if (s == NULL)
        s = mainSocket();
    if (s != NULL)
        s->changeState(state);
}

ReliableUdpSocket::STATE ReliableUdpSocket::getState(ServerSocket* s) 
{
    if (s == NULL)
        s = mainSocket();
    if (s != NULL)
        return s->mState;
    return ST_CLOSED;
}

void ReliableUdpSocket::registerCreator(ReliableUdpSocket::ServerSocketCreator* creator)
{
    mCreator = creator;
    if (mCreator == NULL) {
        mCreator = &DefaultCreator;
    }
}

void ReliableUdpSocket::create(DataReceiver* receiver)
{
    mSocket.Close();
    mSocket.Create();
    mSocket.SetBroadCast(true);

    startSendingProc();

    if (mCreator == NULL) {
        mCreator = &DefaultCreator;
    }

    if (mainSocket() == NULL) {
        ServerSocket* s = DefaultCreator.create(); // createServerSocket();
        s->mReceiver = receiver;
        mServerSockets[0] = s;
    }
}
    
int ReliableUdpSocket::listen(void)
{
    if (getState() != ST_CLOSED)
        return R_ILLEGAL_STATE;
    changeState(ST_LISTEN);
    return R_SUCCESS;
}

int ReliableUdpSocket::bind(int port)
{
    int ret = mSocket.Bind(port);
    if (ret == 0) {
        return R_SUCCESS;
    }
    return R_CHECK_ERRNO;
}

int ReliableUdpSocket::accept(int mstimeout)
{
    if (getState() != ST_LISTEN) {
        return R_ILLEGAL_STATE;
    }

    while (true) {
        char buffer[65536] = {0};
        std::string ip;
        int port;
        int ret = readValidPacket(buffer, sizeof(buffer), ip, port, mstimeout);
        if (ret == R_CLOSED) {
            changeState(ST_CLOSED);
            return ret;
        }
        if (ret <= R_SUCCESS) {
            changeState(ST_LISTEN);
            return ret;
        }
        dumpPacket(buffer, ret, "Acpt");

        Packet::Header* header = (Packet::Header*)buffer;
        if (header->rcid == 0) {
            if (header->sync == 0 || header->fin == 1 || header->rst == 1 || header->ack == 1) {
                dumpPacket(buffer, ret);
                ILOG() << "bogus packet.";
                FATAL();
                continue;
            }
            // received SYN:
            dumpPacket(buffer, ret);

            // 两种情况， 一种是对端没收到SYN ACK，因而重发了SYN
            // 另一种情况是正常的SYN
            // RUN_HERE() << "r: " << ip << ":" << port;
            std::map<int, ServerSocket*>::iterator it;
            for (it = mServerSockets.begin(); it != mServerSockets.end(); it++) {
                ServerSocket* s = it->second;
                // RUN_HERE() << "s: " << s->mSynIP << ":" << s->mSynPort << ", cid = " << s->mLocalCid;

                if (ip == s->mSynIP && port == s->mSynPort && s->getState() == ST_SYN_RCVD) {
                    break;
                }
            }
            if (it != mServerSockets.end()) {
                ServerSocket* s = it->second;
                int res = s->parseRUPacket(buffer, ret);
                if (res == R_ACCEPTED) {
                    break;
                }
                if (res == R_CLOSED)
                    return R_CLOSED;
            } else {
                // if (mCid > 10) {
                //     FATAL() << "Fixme";
                // }
                int cid = ++mCid;
                ServerSocket* s = createServerSocket();
                changeState(ST_SYN_RCVD, s);
                s->mSynIP = ip; //  GetRemoteIP();
                s->mSynPort = port; // GetRemotePort();
                s->mAcknowledge = header->sequence;
                s->setLocalCid(cid);
                s->setRemoteCid(0);
                mServerSockets[cid] = s;
                handshake(true, true, s);
                SLOG() << "connected from: " << ip << ":" << port << ", cid = " << cid;
            }
        } else {
            std::map<int, ServerSocket*>::iterator it;
            it = mServerSockets.find(header->rcid);

            if (it == mServerSockets.end()) {
                // 服务器SYN_RECV超时后关掉连接的情况下会走到这里.
                // 应该给个RST
                // ILOG() << "bogus packet.";
                ILOG() << "received packet after closing. send RST.";
                sendRst(ip.c_str(), port);
                // FATAL();
                continue;
            }
            if (header->sync == 1) {
                ILOG() << "bogus packet.";
                FATAL();
                continue;
            }
            if (header->rst == 1) {
                // FATAL() << "Fixme: RST not implement.";
                changeState(ST_CLOSED);
                return R_CLOSED;
            }
            ServerSocket* s = it->second;
            if (s->mSynIP != ip || s->mSynPort != port) {
                // dumpPacket(buffer, ret, "Cheat", true);
                ILOG() << "cheat packet.";
                ILOG() << "cid = " << it->first;
                ILOG() << "s->mSynIP = " << s->mSynIP;
                ILOG() << "ip = " << ip;
                ILOG() << "s->mSynPort = " << s->mSynPort;
                ILOG() << "port = " << port;
                FATAL();
                continue;
            }
            int res = s->parseRUPacket(buffer, ret);
            if (res == R_ACCEPTED) {
                break;
            }

            if (res == R_CLOSED)
                return R_CLOSED;
        }
    } // while (true)
    changeState(ST_LISTEN);
    return R_SUCCESS;
}

int ReliableUdpSocket::connect(const char * ipaddr, int port, int mstimeout)
{
    //
    // 握手时的状态切换:
    //
    //  LISTEN  SYN_RCVD        ESTABLISHED
    // ---o--------o---------------o----------
    //            / \             /
    //           /   \SYN ACK    /
    //          /     \         / 
    //     SYN /       \       / ACK
    //        /         \     /
    //       /           \   /
    //      /             \ /
    // ----o---------------o-----------------
    //  SYN_SENT       ESTABLISHED
    //
    if (getState() != ST_CLOSED)
        return R_ILLEGAL_STATE;

    if (ipaddr != NULL) {
        int ret = SetRemoteIP(ipaddr);
        if (ret == -1) {
            FATAL() << mSocket.GetErrorMessage();
        }
    }
    if (port != -1)
        SetRemotePort(port);

    if (mstimeout <= 0)
        mstimeout = 10000;

    mainSocket()->mSynIP = GetRemoteIP();
    mainSocket()->mSynPort = GetRemotePort();
    mainSocket()->mRemoteCid = 0;
    
    // SYN
    int ret = handshake(true, false);
    if (ret != R_SUCCESS) {
        return ret;
    }
    changeState(ST_SYN_SENT);

    while (true) {
        char buffer[65536] = {0};
        std::string ip;
        int port;
        ret = readValidPacket(buffer, sizeof(buffer), ip, port, mstimeout);
        if (ret <= R_SUCCESS) {
            changeState(ST_CLOSED);
            return ret;
        }
        // valid packet.
        Packet::Header* header = (Packet::Header*)buffer;
        // check if the packet is SYN ACK
        if (header->sync == 0 || header->fin == 1 || header->rst == 1 || header->ack == 0) {
            ILOG() << "bogus packet.";
            FATAL();
            continue;
        }
        SLOG() << "get SYN ACK from " << ip << ":" << port;
        // dumpPacket(buffer, ret);
        // Fixme 
        // removingPackets(header->acknowledge);
        mainSocket()->mAcknowledge = header->sequence;
        mainSocket()->setRemoteCid(header->lcid);
        mainSocket()->setLocalCid(0);
        mainSocket()->mSynIP = ip;
        mainSocket()->mSynPort = port;
        mainSocket()->parseRUPacket(buffer, ret);

#if 0
        // UNITTEST
        static int flag = 0;
        if (flag % 3 == 1) {
            usleep(SYNRCVD_TIMEOUT * 1000 * 1.2);
        }
        flag ++;
#endif

        handshake(false, true);

        changeState(ST_ESTABLISHED);
        break;
    }
    return R_SUCCESS;
}

void ReliableUdpSocket::dumpPacket(Packet* packet, const char * tag, bool force)
{
#ifndef DUMP_PACKET
    if (!force)
        return;
#endif
    if (packet == NULL) {
        return;
    }

    Packet::Header* header = &packet->header;
    {
        logging::LogMessage logger;
        logger.stream() << tag << " packet: ";
        logger.stream() << "l" << header->lcid << "r" << header->rcid << " ( ";
        if (header->sync)
            logger.stream() << "SYN ";
        if (header->fin)
            logger.stream() << "FIN ";
        if (header->rst)
            logger.stream() << "RST ";
        if (header->ack)
            logger.stream() << "ACK ";
        if (!packet->buffer.isEmpty())
            logger.stream() << "DATA ";
        logger.stream() << ") SEQ: " << header->sequence;
        logger.stream() << "  ACK: " << header->acknowledge;
        if (!packet->buffer.isEmpty()) {
#ifdef DUMP_CONTENT
            logger.stream() << "  " << packet->buffer;
#else
            logger.stream() << "  length: " << packet->buffer.length();
#endif
        }
    }
}

void ReliableUdpSocket::dumpPacket(const char * buffer, int len, const char * tag, bool force)
{
#ifndef DUMP_PACKET
    if (!force)
        return;
#endif
    if (buffer == NULL) {
        return;
    }
    Packet::Header* header = (Packet::Header*)buffer;
    {
        logging::LogMessage logger;
        logger.stream() << tag << " packet: ";
        logger.stream() << "l" << header->lcid << "r" << header->rcid << " ( ";
        if (header->sync)
            logger.stream() << "SYN ";
        if (header->fin)
            logger.stream() << "FIN ";
        if (header->rst)
            logger.stream() << "RST ";
        if (header->ack)
            logger.stream() << "ACK ";
        if (len > (int)sizeof(Packet::Header))
            logger.stream() << "DATA ";
        logger.stream() << ") SEQ: " << header->sequence;
        logger.stream() << "  ACK: " << header->acknowledge;
        if (len > (int)sizeof(Packet::Header)) {
#ifdef DUMP_CONTENT
            Buffer  buf(buffer + sizeof(Packet::Header), len - (int)sizeof(Packet::Header));
            logger.stream() << "  " << buf;
#else
            logger.stream() << "  length: " << len - (int)sizeof(Packet::Header);
#endif
        }
    }
}


int ReliableUdpSocket::readValidPacket(char* buffer, size_t len, std::string& ip, int& port, int mstimeout)
{
    long long timeout = time_now() + mstimeout;
    while (true) {
        char    tip[1024] = {0};
        int ret = mSocket.RecvFrom(tip, port, buffer, len, 5);
        ip = tip;
        if (mstimeout > 0 && time_now() > timeout) {
            // ILOG() << "timeout.";
            return R_TIMEOUT;
        }
        if (ret < 0) {
            return R_CHECK_ERRNO;
        }

        checkTimeout();
        std::map<int, ServerSocket*>::iterator  it;
        for (it = mServerSockets.begin(); it != mServerSockets.end(); /* */) {
            ServerSocket* s = it->second;
            s->checkTimeout();

            if (s->getState() == ST_CLOSED) {
                int cid = s->getLocalCid();
                if (cid != 0) {
                    std::map<int, ServerSocket*>::iterator  it1 = it;
                    it++;
                    mServerSockets.erase(it1);
                    SLOG() << "disconnect from: " << s->mSynIP << ":" << s->mSynPort << ", cid = " << s->mLocalCid;
                    mCreator->remove(s);
                }
                if (cid == 0 || mServerSockets.size() <= 1)
                    return R_CLOSED;
            } else {
                it++;
            }
        }
        doSend();

        if (ret == 0)
            continue;

        if (ret < (int)sizeof(Packet::Header)) {
            ILOG() << "bogus packet.";
            // tools::dump(buffer, ret);
            continue;
        }
        Packet::Header* header = (Packet::Header*)buffer;
        if (!header->check()) {
            ILOG() << "bogus packet.";
            continue;
        }
        return ret;
    }
}

int ReliableUdpSocket::recv(int mstimeout)
{
#if 0
    if (getState() != ST_ESTABLISHED) {
        return R_ILLEGAL_STATE;
    }
#endif

    while (true) {
        char buffer[65536] = {0};
        std::string ip;
        int port;
        int ret = readValidPacket(buffer, sizeof(buffer), ip, port, mstimeout);
        if (ret <= R_SUCCESS) {
            // changeState(ST_LISTEN);
            return ret;
        }

        Packet::Header* header = (Packet::Header*)buffer;
        std::map<int, ServerSocket*>::iterator it;
        it = mServerSockets.find(header->rcid);
        if (it == mServerSockets.end()) {
            ILOG() << "bogus packet. rcid = " << header->rcid;
            FATAL();
            continue;
        }

        if (header->sync == 1) {
            // ILOG() << "bogus packet.";
            dumpPacket(buffer, ret, "Bogus", true);
            // FATAL();
            continue;
        }
        if (header->rst == 1) {
            // FATAL() << "Fixme: RST not implement.";
            changeState(ST_CLOSED);
            return R_CLOSED;
        }
        if (header->lcid != mainSocket()->getRemoteCid()) {
            FATAL();
        }

        ServerSocket* s = it->second;

        if (ip != s->mSynIP || port != s->mSynPort) {
            RUN_HERE() << "expected: " << s->mSynIP << ":" << s->mSynPort;
            RUN_HERE() << "received: " << ip << ":" << port;
            FATAL();
        }
        // dumpPacket(buffer, ret, "Recv", true);

        int res = s->parseRUPacket(buffer, ret);
        if (res == R_SUCCESS) {
            continue;
        } else {
            return res;
        }
    }
    // won't run here.
    return R_SUCCESS;
}

int ReliableUdpSocket::send(const void * buf, size_t len)
{
    int ret = mServerSockets[0]->send((const char *)buf, (int)len);
    doSend();
    return ret;
}

int ReliableUdpSocket::send(ReliableUdpSocket::Packet* p)
{
    int ret = mServerSockets[0]->send(p);
    doSend();
    return ret;
}

void ReliableUdpSocket::close()
{
    ServerSocket* s = mainSocket();
    if (s != NULL) {
        if (s->getState() == ST_CLOSED)
            return;

        if (s->getState() == ST_LISTEN || s->getState() == ST_ESTABLISHED) {
            s->close();
#if 0
            // 没办法在这里阻塞。。这个close()本身会在checkTimeout()时执行，再阻塞会引起函数嵌套调用.
            while (true) {
                char buffer[65536] = {0};
                std::string ip;
                int port;
                // Fixme:
                int ret = readValidPacket(buffer, sizeof(buffer), ip, port, 4000);
                if (ret <= R_SUCCESS) {
                    s->changeState(ST_CLOSED);
                    return;
                }
                RUN_HERE();
                s->parseRUPacket(buffer, ret);
                RUN_HERE();
            }
#endif
        }
    }
}

void ReliableUdpSocket::doSend()
{
    std::map<int, ServerSocket*>::iterator  it;
    for (it = mServerSockets.begin(); it != mServerSockets.end(); it++) {
        ServerSocket* s = it->second;

        s->doSend(mSocket);
    }
}

void ReliableUdpSocket::sendRst(const char * ip, int port)
{
    Packet packet;
    packet.header.rst = 1;
    char    buffer[65536] = {0};
    int ret = sizeof(Packet::Header);
    memcpy(buffer, &packet.header, ret);
    mSocket.SendTo(ip, port, buffer, ret);
}

int ReliableUdpSocket::handshake(bool syn, bool ack, ServerSocket* s)
{
    if (s == NULL)
        s = mainSocket();
    Packet* packet = new Packet();
    packet->header.sync = syn ? 1 : 0;
    packet->header.ack = ack ? 1 : 0;
    if (!syn && ack) {
        packet->needAck = false;
        packet->needSeq = false;
    }
    s->send(packet);
    doSend();
    return R_SUCCESS;
}

void ReliableUdpSocket::clearServerSockets()
{
    if (mainSocket() != NULL) {
        DefaultCreator.remove(mainSocket());
        mServerSockets.erase(0);
    }

    std::map<int, ServerSocket*>::iterator   it;
    for (it = mServerSockets.begin(); it != mServerSockets.end(); it++) {
        ServerSocket* d = it->second;
        if (mCreator != NULL) {
            mCreator->remove(d);
        } else {
            // try delete...
            delete d;
        }
    }
    mServerSockets.clear();
}

void ReliableUdpSocket::startSendingProc()
{
    clearTimeout(timeout_selector(ReliableUdpSocket::onDoSendTimeout));
    setTimeout(10, timeout_selector(ReliableUdpSocket::onDoSendTimeout));
}

void ReliableUdpSocket::onDoSendTimeout(void* arg)
{
    doSend();
    setTimeout(10, timeout_selector(ReliableUdpSocket::onDoSendTimeout));
}

//////////////////////////////////////////////////////////////////////

} // namespace tools







