#include "redis_pool.h"

namespace msgsdk
{

bool RedisPool::init(const std::string& uri,int size)
{
    for(int i=0;i<size;i++)
    {
        pool_.push(std::make_shared<sw::redis::Redis>(uri));
    }

    return true;
}

std::shared_ptr<sw::redis::Redis> RedisPool::acquire()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock,[this]{return !pool_.empty();});

    auto r = pool_.front();

    pool_.pop();

    return r;
}

void RedisPool::release(std::shared_ptr<sw::redis::Redis> redis)
{
    std::lock_guard<std::mutex> lock(mutex_);

    pool_.push(redis);

    cv_.notify_one();
}

}