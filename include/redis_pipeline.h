#ifndef MSGSDK_REDIS_PIPELINE_H
#define MSGSDK_REDIS_PIPELINE_H

#include <sw/redis++/redis++.h>
#include <string>

namespace msgsdk {

// 简单的 Redis pipeline 包装类
class RedisPipeline {
public:

    // 从 redis++ 的 Pipeline 构造，使用右值引用避免拷贝
    explicit RedisPipeline(sw::redis::Pipeline&& pipeline)
        : pipeline_(std::move(pipeline)) {}

    // 禁止拷贝，允许移动
    RedisPipeline(const RedisPipeline&) = delete;
    RedisPipeline& operator=(const RedisPipeline&) = delete;

    RedisPipeline(RedisPipeline&&) = default;
    RedisPipeline& operator=(RedisPipeline&&) = default;

    // 常用操作封装，可按需要扩展
    void set(const std::string& key, const std::string& value) {
        pipeline_.set(key, value);
    }

    void del(const std::string& key) {
        pipeline_.del(key);
    }

    void rpush(const std::string& key, const std::vector<std::string>& values) {
        pipeline_.rpush(key, values.begin(), values.end());
    }

    // 将 GET 命令加入 pipeline；调用 exec() 时统一发送
    void get(const std::string& key) {
        pipeline_.get(key);
    }

    // 执行 pipeline 中累积的命令
    void exec() {
        pipeline_.exec();
    }

private:
    sw::redis::Pipeline pipeline_;
};

} // namespace msgsdk

#endif // MSGSDK_REDIS_PIPELINE_H

