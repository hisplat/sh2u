

#pragma once

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
typedef CRITICAL_SECTION        pthread_mutex_t;
#define pthread_equal(A, B)             ((A) == (B))
#define pthread_mutex_init(A, B)        (InitializeCriticalSection(A), 0)
#define pthread_mutex_lock(A)	        (EnterCriticalSection(A), 0)
#define pthread_mutex_unlock(A)         (LeaveCriticalSection(A), 0)
#define pthread_mutex_destroy(A)        (DeleteCriticalSection(A), 0)
#else
#include <pthread.h>
#endif

namespace tools {

class CLock
{
public:
        CLock();
        ~CLock();

        void    Lock(void);
        void    Unlock(void);

private:
        pthread_mutex_t         hMutex;
};

class AutoLock {
public:
    AutoLock(CLock& lock, bool condition = true) : mCondition(condition), lock_(lock) { if (mCondition) lock_.Lock(); }
    ~AutoLock() { if (mCondition) lock_.Unlock(); }
private:
    bool    mCondition;
    CLock&  lock_;
};

} // namespace tools;

