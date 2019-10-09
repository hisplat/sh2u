

#include "thread.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <stdio.h>
#include "logging.h"

namespace tools {


#ifdef _WIN32
#error "Not implemented yet."

Thread::Thread()
    : delegate_(NULL)
    , thread_(NULL)
    , args_()
    , running_(false)
{
}

Thread::~Thread()
{
}

void Thread::Start(void * arg)
{
}
#else

void * Thread::thread_proc(void * arg)
{
    Args * a = (Args*)arg;
    Thread * thread = a->pThis;
    void * ag = a->arg;

    logging::setThreadLoggingTag(a->name.c_str());

    while (thread->running_) {
        if (thread->delegate_->BeforeFirstRun(ag))
            break;
    }

    while (thread->running_) {
        if (!thread->delegate_->RunOnce(ag))
            thread->running_ = false;
    }
    return NULL;
}

Thread::Thread(Delegate* delegate)
    : delegate_(delegate)
    , thread_(NULL)
    , args_()
    , running_(false)
{
}

Thread::~Thread()
{
}

void Thread::Start(void * arg, const char * name)
{
    if (running_)
        return;
    args_.pThis = this;
    args_.arg = arg;
    if (name != NULL)
        args_.name = name;
    else
        args_.name = "thread";

    running_ = true;
    pthread_create((pthread_t*)&thread_, NULL, thread_proc, (void*)&args_);
}

void Thread::Stop()
{
    // printf("this = %p, thread = %p\n", this, thread_);
    running_ = false;
    pthread_join((pthread_t)thread_, NULL);
}

#endif

}


