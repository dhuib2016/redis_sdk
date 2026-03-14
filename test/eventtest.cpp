#include "RedisEvent.h"
#include <iostream>

class MyRedis:public RedisHelper::RedisEventProcessor
{
protected:

    void Process(const std::string& event,
                 const std::string& msg) override
    {
        std::cout<<"EVENT "<<event<<" -> "<<msg<<std::endl;
    }
};

int main()
{
    MyRedis redis;

    redis.Init("127.0.0.1",6379);

    redis.Subscribe("alarm");

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(10));
}