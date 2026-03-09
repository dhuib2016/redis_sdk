#include "redis_subscriber.h"

namespace msgsdk
{

RedisSubscriber::RedisSubscriber()
{
    running_ = false;
}

RedisSubscriber::~RedisSubscriber()
{
    running_ = false;

    if(thread_.joinable())
        thread_.join();
}

bool RedisSubscriber::connect(const std::string& uri)
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

void RedisSubscriber::subscribe(const std::string& channel)
{
    sub_->subscribe(channel);

    sub_->on_message(
        [this](std::string ch,std::string msg)
        {
            on_message(ch,msg);
        });

    running_ = true;

    thread_ = std::thread(&RedisSubscriber::sub_loop,this);
}

void RedisSubscriber::sub_loop()
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

void RedisSubscriber::on_message(const std::string&,
                                 const std::string&)
{
}

}