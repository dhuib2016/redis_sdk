#ifndef MSGSDK_REDIS_CLIENT_H
#define MSGSDK_REDIS_CLIENT_H

#include <sw/redis++/redis++.h>
#include <string>
#include <thread>
#include <atomic>

#include "redis_pipeline.h"

namespace msgsdk
{

class RedisClient
{
public:

    RedisClient();
    ~RedisClient();

    bool connect(const std::string& uri="tcp://127.0.0.1:6379");

    bool set(const std::string& key,
             const std::string& value);

    bool get(const std::string& key,
             std::string& value);

    bool del(const std::string& key);

    bool publish(const std::string& channel,
                 const std::string& msg);

    void subscribe(const std::string& channel);

    RedisPipeline pipeline();

public:

    virtual void on_message(const std::string& ch,
                            const std::string& msg);

private:

    void sub_loop();

private:

    std::unique_ptr<sw::redis::Redis> redis_;

    std::unique_ptr<sw::redis::Subscriber> sub_;

    std::thread sub_thread_;

    std::atomic<bool> running_;

};

}

#endif
