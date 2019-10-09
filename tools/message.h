
#pragma once

#include <list>
#include <map>
#include <string>
#include <pthread.h>
#include <stdio.h>

#include "weak_ptr.h"

namespace tools {

class Message
{
public:
    std::string what;
    std::string xml;
    std::string sArg1;
    std::string sArg2;
    std::string sArg3;
    int         nArg1;
    int         nArg2;
    int         nArg3;
    int         nArg4;
    int         nArg5;
    void*       pArg1;
    void*       pArg2;
    void*       pArg3;
};

class MessageQueue
{
public:
    MessageQueue();
    virtual ~MessageQueue();

    class Observer : public SupportsWeakPtr<Observer>
    {
    public:
    };

    typedef void (Observer::*fnMessageObserverFuncO)(Message& msg);

    void addObserver(Observer* ob, fnMessageObserverFuncO func, std::string message);
    void dispatch();

    virtual void push(Message& msg);

protected:
    virtual Message pop();
    virtual bool isEmpty();

    std::list<Message>  queue;

    void Lock();
    void Unlock();

private:
    pthread_mutex_t mMutex;

    class ObParam {
    public:
        WeakPtr<Observer>       thiz;
        fnMessageObserverFuncO  func;

    private:
        // Fixme: error when using mObs[msg] = ObParamImpl; would cause segment fault.
        ObParam& operator=(const ObParam& o) { return *this; }
    };

    std::map<std::string, ObParam>  mObs;
};

#define message_callfunc(_SELECTOR) (tools::MessageQueue::fnMessageObserverFuncO)(&_SELECTOR)
#define TOOLS_DECLARE_MESSAGE(x) static const char * x = #x

class OneWayMessageQueue : public MessageQueue
{
public:
    OneWayMessageQueue();
    virtual ~OneWayMessageQueue();
    virtual void push(Message& msg);

protected:
    virtual Message pop();
    virtual bool isEmpty();

    std::list<Message>  pushqueue;

    void swap();

};

} // namespace tools






