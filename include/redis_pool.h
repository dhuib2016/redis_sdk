#ifndef MSGSDK_REDIS_POOL_H
#define MSGSDK_REDIS_POOL_H

#include <sw/redis++/redis++.h>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace msgsdk
{

class RedisPool
{
public:

    bool init(const std::string& uri,int size=8);

    std::shared_ptr<sw::redis::Redis> acquire();

    void release(std::shared_ptr<sw::redis::Redis> redis);

private:

    std::queue<std::shared_ptr<sw::redis::Redis>> pool_;

    std::mutex mutex_;

    std::condition_variable cv_;
};

}

#endif