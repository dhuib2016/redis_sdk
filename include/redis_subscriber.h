#ifndef MSGSDK_REDIS_SUBSCRIBER_H
#define MSGSDK_REDIS_SUBSCRIBER_H

#include <sw/redis++/redis++.h>
#include <thread>
#include <atomic>

namespace msgsdk
{

class RedisSubscriber
{
public:

    RedisSubscriber();

    ~RedisSubscriber();

    bool connect(const std::string& uri="tcp://127.0.0.1:6379");

    void subscribe(const std::string& channel);

public:

    virtual void on_message(const std::string& channel,
                            const std::string& msg);

private:

    void sub_loop();

private:

    std::unique_ptr<sw::redis::Redis> redis_;

    std::unique_ptr<sw::redis::Subscriber> sub_;

    std::thread thread_;

    std::atomic<bool> running_;
};

}

#endif