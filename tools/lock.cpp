

#include "lock.h"


namespace tools {
CLock::CLock()
{
        pthread_mutex_init(&hMutex, NULL);
}

CLock::~CLock()
{
}

void CLock::Lock(void)
{
        pthread_mutex_lock(&hMutex);
}

void CLock::Unlock(void)
{
        pthread_mutex_unlock(&hMutex);
}

} // namespace tools

