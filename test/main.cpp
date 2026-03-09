#include "redis_client.h"
#include <iostream>

int main()
{
    msgsdk::RedisClient redis;

    redis.connect();

    redis.set("name","alice");

    std::string v;

    redis.get("name",v);

    std::cout<<"value="<<v<<std::endl;

    redis.publish("test","hello");

}