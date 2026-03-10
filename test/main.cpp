#include "redis_client.h"
#include <iostream>
#include <chrono>
#include <vector>

using namespace std::chrono;

int main()
{
    msgsdk::RedisClient redis;

    if (!redis.connect()) {
        std::cerr << "connect redis failed" << std::endl;
        return 1;
    }

    const int N = 100000;  // 测试次数

    // 简单的 SET 性能测试
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < N; ++i) {
            std::string key = "perf:set:" + std::to_string(i);
            std::string val = "value-" + std::to_string(i);
            redis.set(key, val);
        }

        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[SET]   count=" << N
                  << ", total=" << ms << " ms"
                  << ", qps=" << qps << std::endl;
    }

    // 简单的 GET 性能测试
    {
        auto start = high_resolution_clock::now();

        std::string val;
        for (int i = 0; i < N; ++i) {
            std::string key = "perf:set:" + std::to_string(i);
            redis.get(key, val);
        }

        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[GET]   count=" << N
                  << ", total=" << ms << " ms"
                  << ", qps=" << qps << std::endl;
    }

    // list 批量写/读性能测试（使用 SetData / GetData）
    {
        const int M = 1000;
        std::vector<std::string> values;
        values.reserve(M);
        for (int i = 0; i < M; ++i) {
            values.emplace_back("item-" + std::to_string(i));
        }

        auto start = high_resolution_clock::now();
        for (int i = 0; i < N; ++i) {
            std::string key = "perf:list:" + std::to_string(i);
            redis.SetData(key, values, false);
        }
        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[LIST-SET]   keys=" << N
                  << ", list_len=" << M
                  << ", total=" << ms << " ms"
                  << ", qps=" << qps << std::endl;

        // 读回 list
        start = high_resolution_clock::now();
        std::vector<std::string> out;
        for (int i = 0; i < N; ++i) {
            std::string key = "perf:list:" + std::to_string(i);
            redis.GetData(key, out);
        }
        end = high_resolution_clock::now();
        ms = duration_cast<milliseconds>(end - start).count();
        qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[LIST-GET]   keys=" << N
                  << ", list_len=" << M
                  << ", total=" << ms << " ms"
                  << ", qps=" << qps << std::endl;
    }

    return 0;
}