
#include "message.h"
#include "logging.h"

namespace tools {

MessageQueue::MessageQueue()
{
    pthread_mutex_init(&mMutex, NULL);
}

MessageQueue::~MessageQueue()
{
    pthread_mutex_destroy(&mMutex);
}

void MessageQueue::Lock()
{
    pthread_mutex_lock(&mMutex);
}

void MessageQueue::Unlock()
{
    pthread_mutex_unlock(&mMutex);
}


void MessageQueue::push(Message& msg)
{
    Lock();
    queue.push_back(msg);
    Unlock();
}

void MessageQueue::addObserver(Observer* ob, fnMessageObserverFuncO func, std::string message)
{
    ObParam o;
    o.thiz = ob->AsWeakPtr();
    o.func = func;

    // Fixme: error when using mObs[msg] = ObParamImpl; would cause segment fault.
    // mObs[message] = o;

    mObs.insert(std::pair<std::string, ObParam>(message, o));
}
 
bool MessageQueue::isEmpty()
{
    return queue.empty();
}

void MessageQueue::dispatch()
{
    while (!isEmpty()) {
        Message message = pop();

        std::map<std::string, ObParam>::iterator  it = mObs.find(message.what);
        if (it == mObs.end()) {
            RUN_HERE() << "no message handler for " << message.what;
            continue;
        }

        ObParam o = it->second;
        if (o.thiz) {
            (o.thiz->*(o.func))(message);
        } else {
            mObs.erase(it);
            RUN_HERE() << "drop message: " << message.what;
        }
    }
}


Message MessageQueue::pop()
{
    Lock();
    Message  msg = queue.front();
    queue.pop_front();
    Unlock();
    return msg;
}


//////////////////////////////////////////////////////////////////////

OneWayMessageQueue::OneWayMessageQueue()
    : MessageQueue()
{
}

OneWayMessageQueue::~OneWayMessageQueue()
{
}

void OneWayMessageQueue::push(Message& msg)
{
    Lock();
    pushqueue.push_back(msg);
    Unlock();
}

Message OneWayMessageQueue::pop()
{
    if (queue.empty())
        swap();
    Message msg = queue.front();
    queue.pop_front();
    return msg;
}

void OneWayMessageQueue::swap()
{
    Lock();
    queue.swap(pushqueue);
    Unlock();
}

bool OneWayMessageQueue::isEmpty()
{
    return queue.empty() && pushqueue.empty();
}

} // namespace tools





