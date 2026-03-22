#include "redis_client.h"
#include "redis_pipeline.h"
#include <iostream>
#include <fstream>

using namespace sw::redis;

namespace msgsdk
{
    RedisClient::RedisClient()
    {
        running_ = false;
    }
    
    RedisClient::~RedisClient()
    {
        running_ = false;
        sub_.reset();   // unblocks consume()

        if (sub_thread_.joinable())
            sub_thread_.join();
    }
    
    bool RedisClient::connect(const std::string& uri)
    {
        try
        {
            sw::redis::Uri parsed(uri);
            ConnectionPoolOptions pool_opts = parsed.connection_pool_options();
            pool_opts.size = pool_size_;
            redis_ = std::make_unique<sw::redis::Redis>(parsed.connection_options(), pool_opts);
            sub_ = std::make_unique<sw::redis::Subscriber>(redis_->subscriber());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::confConnect()
    {
        try
        {
            ConnectionOptions opts;
            opts.host = ip_;
            opts.port = port_;

            if (!auth_.empty())
                opts.password = auth_;

            ConnectionPoolOptions pool_opts;
            pool_opts.size = pool_size_;
            redis_ = std::make_unique<Redis>(opts, pool_opts);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::SelectDB(int index)
    {
        try
        {
            redis_->command<void>("SELECT", std::to_string(index));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::set(const std::string& key, const std::string& value)
    {
        try
        {
            redis_->set(key, value);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::get(const std::string& key, std::string& value)
    {
        try
        {
            auto val = redis_->get(key);
            if (val)
            {
                value = *val;
                return true;
            }
            return false;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::del(const std::string& key)
    {
        redis_->del(key);
        return true;
    }
    
    bool RedisClient::exists(const std::string& key)
    {
        return redis_->exists(key);
    }
    
    RedisClient::KEYTYPE RedisClient::type(const std::string& key)
    {
        try
        {
            std::string t = redis_->type(key);
            
            if (t == "none")   return NOKEY;
            if (t == "string") return STRING;
            if (t == "list")   return VECTOR;
            if (t == "set")    return SET;
            if (t == "hash")   return MAP;
            if (t == "zset")   return ZSET;
            
            return ERRORKEY;
        }
        catch (...)
        {
            return ERRORKEY;
        }
    }
    
    bool RedisClient::expire(const std::string& key, int seconds)
    {
        return redis_->expire(key, seconds);
    }
    
    int RedisClient::ttl(const std::string& key)
    {
        return redis_->ttl(key);
    }
    
    // 原子计数器操作
    long long RedisClient::incr(const std::string& key)
    {
        try
        {
            return redis_->incr(key);
        }
        catch (...)
        {
            return 0;
        }
    }
    
    long long RedisClient::decr(const std::string& key)
    {
        try
        {
            return redis_->decr(key);
        }
        catch (...)
        {
            return 0;
        }
    }
    
    long long RedisClient::incrby(const std::string& key, long long increment)
    {
        try
        {
            return redis_->incrby(key, increment);
        }
        catch (...)
        {
            return 0;
        }
    }
    
    long long RedisClient::decrby(const std::string& key, long long decrement)
    {
        try
        {
            return redis_->decrby(key, decrement);
        }
        catch (...)
        {
            return 0;
        }
    }
    
    double RedisClient::incrbyfloat(const std::string& key, double increment)
    {
        try
        {
            return redis_->incrbyfloat(key, increment);
        }
        catch (...)
        {
            return 0.0;
        }
    }
    
    double RedisClient::decrbyfloat(const std::string& key, double decrement)
    {
        try
        {
            return redis_->incrbyfloat(key, -decrement);
        }
        catch (...)
        {
            return 0.0;
        }
    }
    
    // 事件发布订阅
    bool RedisClient::PublishEvent(const std::string& eventname, const std::string& message)
    {
        return publish(eventname, message);
    }
    
    bool RedisClient::publish(const std::string& channel, const std::string& msg)
    {
        redis_->publish(channel, msg);
        return true;
    }
    
    void RedisClient::subscribe(const std::string& channel)
    {
        sub_->subscribe(channel);
        sub_->on_message([this](std::string ch, std::string msg)
        {
            on_message(ch, msg);
        });
        
        running_ = true;
        sub_thread_ = std::thread(&RedisClient::sub_loop, this);
    }
    
    void RedisClient::sub_loop()
    {
        while (running_)
        {
            try
            {
                sub_->consume();
            }
            catch (...)
            {
            }
        }
    }
    
    // 通用数据接口
    bool RedisClient::SetData(const std::string& key, const std::string& value)
    {
        return set(key, value);
    }
    
    bool RedisClient::SetData(const std::string& key, const std::vector<std::string>& value, bool append)
    {
        try
        {
            if (!append)
            {
                if (value.empty())
                {
                    redis_->del(key);
                    return true;
                }
                auto pipe = redis_->pipeline();
                pipe.del(key);
                pipe.rpush(key, value.begin(), value.end());
                pipe.exec();
                return true;
            }

            if (value.empty())
                return true;

            redis_->rpush(key, value.begin(), value.end());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::SetData(const std::string& key, const std::set<std::string>& value, bool append)
    {
        try
        {
            if (!append)
            {
                if (value.empty())
                {
                    redis_->del(key);
                    return true;
                }
                auto pipe = redis_->pipeline();
                pipe.del(key);
                pipe.sadd(key, value.begin(), value.end());
                pipe.exec();
                return true;
            }

            if (value.empty())
                return true;

            redis_->sadd(key, value.begin(), value.end());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::SetData(const std::string& key, const std::map<std::string, std::string>& value, bool append)
    {
        try
        {
            if (!append)
            {
                if (value.empty())
                {
                    redis_->del(key);
                    return true;
                }
                auto pipe = redis_->pipeline();
                pipe.del(key);
                pipe.hmset(key, value.begin(), value.end());
                pipe.exec();
                return true;
            }

            if (value.empty())
                return true;

            redis_->hmset(key, value.begin(), value.end());
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    void RedisClient::GetData(const std::string& key, std::string& data)
    {
        data.clear();
        get(key, data);
    }
    
    void RedisClient::GetData(const std::string& key, std::vector<std::string>& data)
    {
        data.clear();
        try
        {
            redis_->lrange(key, 0, -1, std::back_inserter(data));
        }
        catch (...)
        {
        }
    }
    
    void RedisClient::GetData(const std::string& key, std::set<std::string>& data)
    {
        data.clear();
        try
        {
            redis_->smembers(key, std::inserter(data, data.begin()));
        }
        catch (...)
        {
        }
    }
    
    void RedisClient::GetData(const std::string& key, std::map<std::string, std::string>& data)
    {
        data.clear();
        try
        {
            redis_->hgetall(key, std::inserter(data, data.begin()));
        }
        catch (...)
        {
        }
    }
    
    // 删除操作
    void RedisClient::DelKey(const std::string& key)
    {
        try
        {
            redis_->del(key);
        }
        catch (...)
        {
        }
    }
    
    void RedisClient::DelKey(const std::vector<std::string>& keys)
    {
        try
        {
            if (!keys.empty())
                redis_->del(keys.begin(), keys.end());
        }
        catch (...)
        {
        }
    }
    
    void RedisClient::DelField(const std::string& key, const std::vector<std::string>& fields)
    {
        try
        {
            if (!fields.empty())
                redis_->hdel(key, fields.begin(), fields.end());
        }
        catch (...)
        {
        }
    }
    
    // 维护操作
    void RedisClient::Flush()
    {
        try
        {
            redis_->flushdb();
        }
        catch (...)
        {
        }
    }
    
    void RedisClient::PersistentStorage()
    {
        try
        {
            redis_->bgsave();
        }
        catch (...)
        {
        }
    }
    
    // 连接控制
    bool RedisClient::ReConnect()
    {
        try
        {
            redis_.reset();
            return confConnect();
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::IsConnected()
    {
        if (!redis_)
            return false;
        
        try
        {
            auto pong = redis_->ping();
            return !pong.empty();
        }
        catch (...)
        {
            return false;
        }
    }
    
    bool RedisClient::Auth(const std::string& password)
    {
        try
        {
            redis_->auth(password);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    // 管道操作
    RedisPipeline RedisClient::pipeline()
    {
        return RedisPipeline(redis_->pipeline());
    }
    
    // 消息回调
    void RedisClient::on_message(const std::string&, const std::string&)
    {
    }
    
    // 配置加载
    bool RedisClient::loadConfig(const std::string& file)
    {
        std::ifstream in(file);
        std::string line;
        
        while (getline(in, line))
        {
            if (line.empty() || line[0] == '#')
                continue;
            
            auto pos = line.find('=');
            if (pos == std::string::npos)
                continue;
            
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "redis.server.ip")
                ip_ = value;
            
            if (key == "redis.server.port")
                port_ = std::stoi(value);
            
            if (key == "redis.server.auth")
                auth_ = value;
        }
        
        return true;
    }
}