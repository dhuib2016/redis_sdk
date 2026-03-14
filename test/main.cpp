#include "redis_client.h"
#include "redis_pipeline.h"
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

    const int N = 100000;
    const int BATCH = 1000;

    // SET sequential
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

        std::cout << "[SET sequential]  count=" << N
                  << ", total=" << ms << "ms"
                  << ", qps=" << qps << std::endl;
    }

    // SET pipelined
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < N; i += BATCH) {
            auto pipe = redis.pipeline();
            for (int j = i; j < i + BATCH && j < N; ++j) {
                pipe.set("perf:set:" + std::to_string(j),
                         "value-" + std::to_string(j));
            }
            pipe.exec();
        }

        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[SET pipelined]   count=" << N
                  << ", batch=" << BATCH
                  << ", total=" << ms << "ms"
                  << ", qps=" << qps << std::endl;
    }

    // GET sequential
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

        std::cout << "[GET sequential]  count=" << N
                  << ", total=" << ms << "ms"
                  << ", qps=" << qps << std::endl;
    }

    // GET pipelined
    {
        auto start = high_resolution_clock::now();

        for (int i = 0; i < N; i += BATCH) {
            auto pipe = redis.pipeline();
            for (int j = i; j < i + BATCH && j < N; ++j) {
                pipe.get("perf:set:" + std::to_string(j));
            }
            pipe.exec();
        }

        auto end = high_resolution_clock::now();
        auto ms = duration_cast<milliseconds>(end - start).count();
        double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

        std::cout << "[GET pipelined]   count=" << N
                  << ", batch=" << BATCH
                  << ", total=" << ms << "ms"
                  << ", qps=" << qps << std::endl;
    }

    // LIST-SET sequential + GET + pipelined SET
    {
        const int M = 1000;
        std::vector<std::string> values;
        values.reserve(M);
        for (int i = 0; i < M; ++i)
            values.emplace_back("item-" + std::to_string(i));

        // LIST-SET sequential
        {
            auto start = high_resolution_clock::now();
            for (int i = 0; i < N; ++i) {
                std::string key = "perf:list:" + std::to_string(i);
                redis.SetData(key, values, false);
            }
            auto end = high_resolution_clock::now();
            auto ms = duration_cast<milliseconds>(end - start).count();
            double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

            std::cout << "[LIST-SET sequential]  keys=" << N
                      << ", list_len=" << M
                      << ", total=" << ms << "ms"
                      << ", qps=" << qps << std::endl;
        }

        // LIST-SET pipelined
        {
            auto start = high_resolution_clock::now();
            for (int i = 0; i < N; i += BATCH) {
                auto pipe = redis.pipeline();
                for (int j = i; j < i + BATCH && j < N; ++j) {
                    std::string key = "perf:list:" + std::to_string(j);
                    pipe.del(key);
                    pipe.rpush(key, values);
                }
                pipe.exec();
            }
            auto end = high_resolution_clock::now();
            auto ms = duration_cast<milliseconds>(end - start).count();
            double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

            std::cout << "[LIST-SET pipelined]   keys=" << N
                      << ", list_len=" << M
                      << ", batch=" << BATCH
                      << ", total=" << ms << "ms"
                      << ", qps=" << qps << std::endl;
        }

        // LIST-GET sequential
        {
            auto start = high_resolution_clock::now();
            std::vector<std::string> out;
            for (int i = 0; i < N; ++i) {
                std::string key = "perf:list:" + std::to_string(i);
                redis.GetData(key, out);
            }
            auto end = high_resolution_clock::now();
            auto ms = duration_cast<milliseconds>(end - start).count();
            double qps = (ms > 0) ? (N * 1000.0 / ms) : 0.0;

            std::cout << "[LIST-GET]   keys=" << N
                      << ", list_len=" << M
                      << ", total=" << ms << "ms"
                      << ", qps=" << qps << std::endl;
        }
    }

    return 0;
}
