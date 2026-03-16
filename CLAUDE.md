# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```bash
# Build the static library (libredis_sdk.a)
make

# Build the test executable
make test

# Run the benchmark/test
./test_app

# Clean build artifacts
make clean
```

The project uses `g++` with `-std=c++17 -O2 -pthread` flags and links against `-lredis++ -lhiredis`.

To run the event test (in `test/eventtest.cpp`), compile it manually linking against the static library:
```bash
g++ -std=c++17 -O2 -pthread -I./include test/eventtest.cpp -L. -lredis_sdk -lredis++ -lhiredis -o event_test
```

## Architecture

This is a C++ Redis SDK wrapper around [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) (`sw::redis` namespace). It provides two namespaces:

- **`msgsdk`** — synchronous client, connection pool, subscriber, and pipeline
- **`RedisHelper`** — asynchronous event-driven pub/sub

### Core Classes

**`msgsdk::RedisClient`** (`include/redis_client.h`, `src/redis_client.cpp`)
Primary Redis wrapper for a single connection. Supports string/list/set/map operations via generic `SetData()`/`GetData()` templates, pub/sub via `publish()`/`subscribe()`, and pipeline via `pipeline()`. Spawns a background `sub_thread_` to consume subscriptions.

**`msgsdk::RedisPool`** (`include/redis_pool.h`, `src/redis_pool.cpp`)
Thread-safe connection pool (default 8 connections). Uses a mutex + condition variable for blocking `acquire()`/`release()`. Call `init()` before use.

**`msgsdk::RedisSubscriber`** (`include/redis_subscriber.h`, `src/redis_subscriber.cpp`)
Lightweight single-channel subscriber. Override `on_message()` in subclasses. Runs a dedicated consumption thread.

**`msgsdk::RedisPipeline`** (`include/redis_pipeline.h`)
Move-only pipeline wrapper for batching commands. Obtain via `RedisClient::pipeline()`, call `exec()` to flush.

**`RedisHelper::RedisEventProcessor`** (`include/RedisEvent.h`, `src/RedisEvent.cpp`)
Producer-consumer pub/sub system. A guard thread reads from Redis and enqueues messages; a worker thread dequeues and calls the abstract `Process()` method. Handles automatic reconnection. Subclass and implement `Process(const Message&)` to use it. See `test/eventtest.cpp` for a usage example.

### Threading Model

- `RedisClient`: 1 background subscriber thread per instance
- `RedisSubscriber`: 1 consumption thread per instance
- `RedisEventProcessor`: 2 threads — guard thread (Redis I/O + reconnection) and worker thread (message processing)
- `RedisPool`: blocking acquire with condition variable, no dedicated thread


## Compact Instructions

When compressing, preserve in priority order:

1. Architecture decisions (NEVER summarize)
2. Modified files and their key changes
3. Current verification status (pass/fail)
4. Open TODOs and rollback notes
5. Tool outputs (can delete, keep pass/fail only)
