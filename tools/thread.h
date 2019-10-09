

#pragma once

#include <stdlib.h>
#include <string>

namespace tools {

class Thread {
public:
    class Delegate {
    public:
        Delegate() {}
        virtual ~Delegate() {}
        virtual bool BeforeFirstRun(void * arg) { return true; }
        virtual bool RunOnce(void * arg) = 0;
    };

    class Args {
    public:
        Args() : pThis(NULL), arg(NULL) {}
        Thread*     pThis;
        void *      arg;
        std::string name;
    };

    Thread(Delegate* delegate);
    virtual ~Thread();

    void Start(void * arg, const char * name = NULL);
    void Stop();
    bool isRunning() { return running_; }
    Delegate* delegate() { return delegate_; }

private:
    Delegate * delegate_;
    void *  thread_;
    Args    args_;
    bool    running_;

    static void * thread_proc(void * arg);
};

}


