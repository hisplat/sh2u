
#include "timeout.h"
#include "logging.h"
#include <sys/time.h>
#include <stdlib.h>

namespace tools {

Timeout::Timeout(bool needLock)
    : mNeedLock(needLock)
    , mRunningRunnable(NULL)
{
}

Timeout::~Timeout()
{
    std::list<DelayRunnable*>::iterator it;
    for (it = mDelays.begin(); it != mDelays.end(); it++) {
        DelayRunnable* dr = *it;
        delete dr;
    }
    mDelays.clear();
}

void Timeout::setTimeout(int ms, DelayFunctor functor, void* arg)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    long long now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    long long delay = now + ms;

    tv.tv_sec = delay / 1000;
    tv.tv_usec = (delay % 1000) * 1000;

    DelayRunnable* dr = new DelayRunnable();
    dr->mTimeToRun = tv;
    dr->arg = arg;
    dr->functor = functor;

    tools::AutoLock lk(mLock, mNeedLock);
    mDelays.push_back(dr);
}

void Timeout::clearTimeout(DelayFunctor  functor)
{
    if (mDelays.empty())
        return;

    tools::AutoLock lk(mLock, mNeedLock);

    std::list<DelayRunnable*>::iterator it;
    for (it = mDelays.begin(); it != mDelays.end(); /* */) {
        DelayRunnable* dr = *it;
        if (dr == mRunningRunnable) {
            it++;
            continue;
        }
        if (dr->functor == functor) {
            delete dr;
            it = mDelays.erase(it);
        } else {
            it++;
        }
    }
}

bool Timeout::isScheduled(DelayFunctor functor)
{
    if (mDelays.empty())
        return false;

    tools::AutoLock lk(mLock, mNeedLock);

    std::list<DelayRunnable*>::iterator it;
    for (it = mDelays.begin(); it != mDelays.end(); it++) {
        DelayRunnable* dr = *it;
        if (dr->functor == functor) {
            return true;
        }
    }

    return false;
}

void Timeout::checkTimeout()
{
    if (mDelays.empty())
        return;

    std::list<DelayRunnable*>::iterator it;
    { // braces for autolock.
        tools::AutoLock lk(mLock, mNeedLock);
        for (it = mDelays.begin(); it != mDelays.end(); it++) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            DelayRunnable* dr = *it;
            if ((tv.tv_sec - dr->mTimeToRun.tv_sec) * 1000000 + (tv.tv_usec - dr->mTimeToRun.tv_usec) >= 0) {
                break;
            }
        }
    }
    // must unlock before calling dr->functor

    if(it != mDelays.end()) {
        DelayRunnable* dr = *it;
        mRunningRunnable = dr;
        (this->*(dr->functor))(dr->arg);
        mDelays.erase(it);
        mRunningRunnable = NULL;
        delete dr;
    }
}

int Timeout::getNearestMs()
{
    if (mDelays.empty()) {
        return 500;
    }

    struct timeval nearest;
    gettimeofday(&nearest, NULL);

    tools::AutoLock lk(mLock, mNeedLock);

    std::list<DelayRunnable*>::iterator it;
    for (it = mDelays.begin(); it != mDelays.end(); it++) {
        DelayRunnable* dr = *it;
        if (it == mDelays.begin()) {
            nearest = dr->mTimeToRun;
        } else {
            if (nearest.tv_sec * 1000000 + nearest.tv_usec > dr->mTimeToRun.tv_sec * 1000000 + dr->mTimeToRun.tv_usec)
                nearest = dr->mTimeToRun;
        }
    }
    struct timeval  now;
    gettimeofday(&now, NULL);

    return (nearest.tv_sec - now.tv_sec) * 1000 + ((nearest.tv_usec - now.tv_usec) % 1000);
}

} // namespace tools








