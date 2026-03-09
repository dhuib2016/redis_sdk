#include "redis_client.h"
#include "redis_pipeline.h"
#include <iostream>

namespace msgsdk
{

RedisClient::RedisClient()
{
    running_ = false;
}

RedisClient::~RedisClient()
{
    running_ = false;

    if(sub_thread_.joinable())
        sub_thread_.join();
}

bool RedisClient::connect(const std::string& uri)
{
    try
    {
        redis_ = std::make_unique<sw::redis::Redis>(uri);

        sub_ = std::make_unique<sw::redis::Subscriber>(
            redis_->subscriber());

        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool RedisClient::set(const std::string& key,
                      const std::string& value)
{
    try
    {
        redis_->set(key,value);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool RedisClient::get(const std::string& key,
                      std::string& value)
{
    try
    {
        auto val = redis_->get(key);

        if(val)
        {
            value = *val;
            return true;
        }

        return false;
    }
    catch(...)
    {
        return false;
    }
}

bool RedisClient::del(const std::string& key)
{
    redis_->del(key);
    return true;
}

bool RedisClient::publish(const std::string& channel,
                          const std::string& msg)
{
    redis_->publish(channel,msg);

    return true;
}

void RedisClient::subscribe(const std::string& channel)
{
    sub_->subscribe(channel);

    sub_->on_message(
        [this](std::string ch,std::string msg)
        {
            on_message(ch,msg);
        });

    running_ = true;

    sub_thread_ = std::thread(&RedisClient::sub_loop,this);
}

void RedisClient::sub_loop()
{
    while(running_)
    {
        try
        {
            sub_->consume();
        }
        catch(...)
        {
        }
    }
}

RedisPipeline RedisClient::pipeline()
{
    // 将 redis++ 的 Pipeline 移交给自定义 RedisPipeline 包装类
    return RedisPipeline(redis_->pipeline());
}

void RedisClient::on_message(const std::string&,
                             const std::string&)
{
}

}