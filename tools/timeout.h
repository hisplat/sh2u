
#pragma once

#include <sys/time.h>
#include <stdlib.h>
#include <list>
#include "tools/lock.h"

namespace tools {
class Timeout
{
public:
    Timeout(bool needLock = false);
    virtual ~Timeout();

    typedef void (Timeout::*DelayFunctor)(void*);

    void checkTimeout();
    void setTimeout(int ms, DelayFunctor functor, void* arg = NULL);
    void clearTimeout(DelayFunctor functor);
    bool isScheduled(DelayFunctor functor);
    void setNeedLock(bool n) { mNeedLock = n; }

#define timeout_selector(x)  (DelayFunctor)(&x)

    int getNearestMs();

private:
    class DelayRunnable {
    public:
        struct timeval  mTimeToRun; 
        void *  arg;
        DelayFunctor    functor;
    };
    std::list<DelayRunnable*>   mDelays;

    bool                mNeedLock;
    tools::CLock        mLock;

    DelayRunnable *     mRunningRunnable;

};

} // namespace tools

