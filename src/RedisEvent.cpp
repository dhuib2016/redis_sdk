#include "RedisEvent.h"
#include <iostream>

using namespace RedisHelper;

RedisEventProcessor::RedisEventProcessor()
{
    _running=false;
}

RedisEventProcessor::~RedisEventProcessor()
{
    _running=false;
    _subscriber.reset();    // unblocks GuardLoop's consume()
    _cond.notify_all();     // unblocks WorkerLoop's wait()

    if(_guardThread.joinable())
        _guardThread.join();

    if(_workerThread.joinable())
        _workerThread.join();
}

bool RedisEventProcessor::Init(const std::string& host,int port,int db)
{
    _host=host;
    _port=port;
    _db=db;

    try
    {
        sw::redis::ConnectionOptions opt;

        opt.host=host;
        opt.port=port;
        opt.db=db;

        _redis=std::make_shared<sw::redis::Redis>(opt);

        if(!_password.empty())
            _redis->auth(_password);

        _subscriber=std::make_unique<sw::redis::Subscriber>(_redis->subscriber());

        _subscriber->on_message(
        [this](std::string channel,std::string msg)
        {
            PushMessage(channel,msg);
        });

        _subscriber->on_pmessage(
        [this](std::string pattern,std::string channel,std::string msg)
        {
            PushMessage(channel,msg);
        });

        _running=true;

        _guardThread=std::thread(&RedisEventProcessor::GuardLoop,this);
        _workerThread=std::thread(&RedisEventProcessor::WorkerLoop,this);

        return true;
    }
    catch(std::exception& e)
    {
        std::cout<<"Redis init error "<<e.what()<<std::endl;
        return false;
    }
}

void RedisEventProcessor::Auth(const std::string& password)
{
    _password=password;
}

void RedisEventProcessor::Subscribe(const std::string& channel)
{
    _channels.push_back(channel);

    if(_subscriber)
        _subscriber->subscribe(channel);
}

void RedisEventProcessor::Subscribe(const std::vector<std::string>& channels)
{
    for(auto& c:channels)
        Subscribe(c);
}

void RedisEventProcessor::PSubscribe(const std::string& pattern)
{
    _patterns.push_back(pattern);

    if(_subscriber)
        _subscriber->psubscribe(pattern);
}

void RedisEventProcessor::UnSubscribe(const std::string& channel)
{
    if(_subscriber)
        _subscriber->unsubscribe(channel);
}

void RedisEventProcessor::UnSubscribeAll()
{
    if(!_subscriber) return;

    for(auto& c:_channels)
        _subscriber->unsubscribe(c);

    _channels.clear();
}

bool RedisEventProcessor::Publish(const std::string& channel,const std::string& msg)
{
    try
    {
        if(!_redis) return false;

        _redis->publish(channel,msg);
        return true;
    }
    catch(...)
    {
        return false;
    }
}

bool RedisEventProcessor::IsConnected()
{
    try
    {
        if(!_redis) return false;

        _redis->ping();
        return true;
    }
    catch(...)
    {
        return false;
    }
}

void RedisEventProcessor::PushMessage(const std::string& e,const std::string& msg)
{
    std::unique_lock<std::mutex> lock(_mutex);

    _queue.push({e,msg});

    _cond.notify_one();
}

void RedisEventProcessor::WorkerLoop()
{
    while(_running)
    {
        Message m;

        {
            std::unique_lock<std::mutex> lock(_mutex);

            if(_queue.empty())
            {
                _cond.wait(lock);
                continue;
            }

            m=_queue.front();
            _queue.pop();
        }

        try
        {
            Process(m.event,m.payload);
        }
        catch(...)
        {
        }
    }
}

void RedisEventProcessor::GuardLoop()
{
    while(_running)
    {
        try
        {
            if(_subscriber)
                _subscriber->consume();
        }
        catch(std::exception& e)
        {
            std::cout<<"Redis disconnected "<<e.what()<<std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));

            Reconnect();
        }
    }
}

void RedisEventProcessor::Reconnect()
{
    try
    {
        sw::redis::ConnectionOptions opt;

        opt.host=_host;
        opt.port=_port;
        opt.db=_db;

        _redis=std::make_shared<sw::redis::Redis>(opt);

        _subscriber=std::make_unique<sw::redis::Subscriber>(_redis->subscriber());

        _subscriber->on_message(
        [this](std::string channel,std::string msg)
        {
            PushMessage(channel,msg);
        });

        _subscriber->on_pmessage(
        [this](std::string pattern,std::string channel,std::string msg)
        {
            PushMessage(channel,msg);
        });

        ReSubscribe();
    }
    catch(...)
    {
    }
}

void RedisEventProcessor::ReSubscribe()
{
    for(auto& c:_channels)
        _subscriber->subscribe(c);

    for(auto& p:_patterns)
        _subscriber->psubscribe(p);
}