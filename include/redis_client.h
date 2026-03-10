#ifndef MSGSDK_REDIS_CLIENT_H
#define MSGSDK_REDIS_CLIENT_H

#include <sw/redis++/redis++.h>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <set>
#include <map>
#include "redis_pipeline.h"

namespace msgsdk
{
    class RedisClient
    {
    public:
        enum KEYTYPE
        {
            NOKEY,
            STRING,
            VECTOR,  // list
            SET,
            MAP,     // hash
            ZSET,
            ERRORKEY
        };
        
    public:
        RedisClient();
        ~RedisClient();
        
        // 连接相关
        bool connect(const std::string& uri = "tcp://127.0.0.1:6379");
        bool confConnect();
        bool ReConnect();
        bool IsConnected();
        bool Auth(const std::string& password);
        
        // 数据库操作
        bool SelectDB(int index);
        
        // 基本键值操作
        bool set(const std::string& key, const std::string& value);
        bool get(const std::string& key, std::string& value);
        bool del(const std::string& key);
        
        // 键管理
        bool expire(const std::string& key, int ttl_seconds);
        bool exists(const std::string& key);
        KEYTYPE type(const std::string& key);
        int ttl(const std::string& key);
        
        // 原子计数器
        long long incr(const std::string& key);
        long long decr(const std::string& key);
        long long incrby(const std::string& key, long long increment);
        long long decrby(const std::string& key, long long decrement);
        double incrbyfloat(const std::string& key, double increment);
        double decrbyfloat(const std::string& key, double decrement);
        
        // 发布订阅
        bool PublishEvent(const std::string& eventname, const std::string& message);
        bool publish(const std::string& channel, const std::string& msg);
        void subscribe(const std::string& channel);
        
        // 通用数据接口
        bool SetData(const std::string& key, const std::string& value);
        bool SetData(const std::string& key, const std::vector<std::string>& value, bool append = false);
        bool SetData(const std::string& key, const std::set<std::string>& value, bool append = false);
        bool SetData(const std::string& key, const std::map<std::string, std::string>& value, bool append = false);
        
        void GetData(const std::string& key, std::string& data);
        void GetData(const std::string& key, std::vector<std::string>& data);
        void GetData(const std::string& key, std::set<std::string>& data);
        void GetData(const std::string& key, std::map<std::string, std::string>& data);
        
        // 删除操作
        void DelKey(const std::string& key);
        void DelKey(const std::vector<std::string>& keys);
        void DelField(const std::string& key, const std::vector<std::string>& fields);
        
        // 维护操作
        void Flush();
        void PersistentStorage();
        
        // 管道操作
        RedisPipeline pipeline();
        
        // 事件回调
        virtual void on_message(const std::string& ch, const std::string& msg);
        
    private:
        bool loadConfig(const std::string& file);
        void sub_loop();
        
    private:
        // 配置
        std::string ip_;
        int port_ = 6379;
        std::string auth_;
        
        // Redis 连接
        std::unique_ptr<sw::redis::Redis> redis_;
        std::unique_ptr<sw::redis::Subscriber> sub_;
        
        // 订阅线程
        std::thread sub_thread_;
        std::atomic<bool> running_;
    };
}

#endif