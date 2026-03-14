#ifndef REDIS_EVENT_PROCESSOR_H
#define REDIS_EVENT_PROCESSOR_H

#include <sw/redis++/redis++.h>
#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>

namespace RedisHelper
{

struct Message
{
    std::string event;
    std::string payload;
};

class RedisEventProcessor
{
public:

    RedisEventProcessor();
    virtual ~RedisEventProcessor();

    bool Init(const std::string& host="127.0.0.1",
              int port=6379,
              int db=0);

    void Auth(const std::string& password);

    void Subscribe(const std::string& channel);
    void Subscribe(const std::vector<std::string>& channels);

    void PSubscribe(const std::string& pattern);

    void UnSubscribe(const std::string& channel);
    void UnSubscribeAll();

    bool Publish(const std::string& channel,
                 const std::string& msg);

    bool IsConnected();

protected:

    virtual void Process(const std::string& event,
                         const std::string& message)=0;

private:

    void GuardLoop();
    void WorkerLoop();
    void Reconnect();
    void ReSubscribe();

    void PushMessage(const std::string& e,
                     const std::string& msg);

private:

    std::string _host;
    int _port;
    int _db;

    std::string _password;

    std::shared_ptr<sw::redis::Redis> _redis;
    std::unique_ptr<sw::redis::Subscriber> _subscriber;

    std::vector<std::string> _channels;
    std::vector<std::string> _patterns;

    std::queue<Message> _queue;

    std::mutex _mutex;
    std::condition_variable _cond;

    std::atomic<bool> _running;

    std::thread _guardThread;
    std::thread _workerThread;
};

}

#endif