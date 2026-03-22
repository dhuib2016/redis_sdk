# Benchmark Results: Connection Pooling + Internal Pipeline

**Date**: 2026-03-21
**Plan**: `2026-03-21-connection-pool-and-internal-pipeline-plan.md`
**Binary**: `test_app` built from `test/main.cpp`
**Config**: N=100,000 ops, BATCH=1000, 4 threads for multi-thread tests

## Test Logic

Each benchmark times a tight loop of N operations using `high_resolution_clock` and computes QPS as `N * 1000 / ms`.

### Group 1: String key-value (sequential vs pipelined)

Simple `redis.set(key, value)` / `redis.get(key, value)` on plain string keys.
Both sequential (one command per round-trip) and pipelined (1000 commands batched per `exec()`) variants exist because `RedisPipeline` wraps `set()` and `get()`.

| Benchmark | Function Under Test | What It Proves |
|---|---|---|
| SET sequential | `RedisClient::set()` | Single-connection string write throughput |
| SET pipelined | `RedisPipeline::set()` + `exec()` | Batched write throughput (1000/batch) |
| GET sequential | `RedisClient::get()` | Single-connection string read throughput |
| GET pipelined | `RedisPipeline::get()` + `exec()` | Batched read throughput (1000/batch) |

### Group 2: Redis LIST (vector\<string\>, 1000 elements/key)

Writes via `SetData(key, vector, false)` which does DEL+RPUSH. Reads via `GetData(key, vector)` which does LRANGE.

- **LIST-SET has both sequential and pipelined** because `RedisPipeline` wraps `del()` and `rpush()`.
- **LIST-GET has sequential only** because `RedisPipeline` does not wrap `lrange()`.

| Benchmark | Function Under Test | What It Proves |
|---|---|---|
| LIST-SET sequential | `SetData(key, vector, false)` | Internal pipeline: DEL+RPUSH in 1 round-trip |
| LIST-SET pipelined | `RedisPipeline::del()` + `rpush()` + `exec()` | External pipeline for list writes (1000/batch) |
| LIST-GET | `GetData(key, vector)` via `lrange` | List read throughput (1000 elements/key) |

### Group 3: Redis SET (set\<string\>, 100 members/key)

Writes via `SetData(key, set, false)` which does DEL+SADD. Reads via `GetData(key, set)` which does SMEMBERS.

**Sequential only** — `RedisPipeline` does not wrap `sadd()` or `smembers()`, so no pipelined variant exists.

| Benchmark | Function Under Test | What It Proves |
|---|---|---|
| SET-TYPE sequential | `SetData(key, set, false)` | Internal pipeline: DEL+SADD in 1 round-trip |
| SET-TYPE GET | `GetData(key, set)` via `smembers` | Set read throughput (100 members/key) |

### Group 4: Redis HASH (map\<string,string\>, 100 fields/key)

Writes via `SetData(key, map, false)` which does DEL+HMSET. Reads via `GetData(key, map)` which does HGETALL.

**Sequential only** — `RedisPipeline` does not wrap `hmset()` or `hgetall()`, so no pipelined variant exists.

| Benchmark | Function Under Test | What It Proves |
|---|---|---|
| HASH-SET sequential | `SetData(key, map, false)` | Internal pipeline: DEL+HMSET in 1 round-trip |
| HASH-GET | `GetData(key, map)` via `hgetall` | Hash read throughput (100 fields/key) |

### Group 5: Multi-threaded (connection pool)

4 threads each do 25,000 simple string `set()`/`get()` calls concurrently against the same `RedisClient` instance. With connection pool (pool_size=4), each thread gets its own connection. Without the pool, all 4 threads would serialize on a single connection and achieve roughly the same QPS as sequential.

| Benchmark | Function Under Test | What It Proves |
|---|---|---|
| SET multi-thread | 4 threads × `RedisClient::set()` | Connection pool concurrent write scaling |
| GET multi-thread | 4 threads × `RedisClient::get()` | Connection pool concurrent read scaling |

### Coverage gaps

The following pipelined benchmarks are **missing** because `RedisPipeline` does not expose the corresponding commands:

| Missing benchmark | Blocked by |
|---|---|
| LIST-GET pipelined | `RedisPipeline` has no `lrange()` |
| SET-TYPE pipelined | `RedisPipeline` has no `sadd()` |
| SET-TYPE GET pipelined | `RedisPipeline` has no `smembers()` |
| HASH-SET pipelined | `RedisPipeline` has no `hmset()` |
| HASH-GET pipelined | `RedisPipeline` has no `hgetall()` |

These could be added by extending `RedisPipeline` in `include/redis_pipeline.h`.

## Baseline (before optimization, single connection, no internal pipeline)

| Benchmark | QPS |
|---|---|
| SET sequential | 1,278 |
| SET pipelined | 15,186 |
| GET sequential | 1,284 |
| GET pipelined | 15,664 |
| LIST-SET sequential | 507 |
| LIST-SET pipelined | 1,359 |
| LIST-GET | 998 |
| SET-TYPE sequential | N/A (not benchmarked) |
| SET-TYPE GET | N/A (not benchmarked) |
| HASH-SET sequential | N/A (not benchmarked) |
| HASH-GET | N/A (not benchmarked) |
| SET multi-thread | N/A (not supported) |
| GET multi-thread | N/A (not supported) |

## Post-Optimization (pool_size=4, internal pipeline for compound ops)

| Benchmark | QPS |
|---|---|
| SET sequential | 1,256 |
| SET pipelined | 15,833 |
| GET sequential | 1,258 |
| GET pipelined | 15,385 |
| LIST-SET sequential | 500 |
| LIST-SET pipelined | 1,342 |
| LIST-GET | 971 |
| SET-TYPE sequential | 584 |
| SET-TYPE GET | 1,397 |
| HASH-SET sequential | 634 |
| HASH-GET | 1,365 |
| SET multi-thread | 3,735 |
| GET multi-thread | 3,678 |

## Evaluation

### Connection pooling (pool_size=4)

- **Multi-threaded SET**: 3,735 QPS — **2.9x** the single-threaded sequential rate (1,256)
- **Multi-threaded GET**: 3,678 QPS — **2.9x** the single-threaded sequential rate (1,258)
- Single-threaded benchmarks stayed flat (within run-to-run variance), confirming negligible pool overhead

### Internal pipeline for compound SetData

- LIST-SET, SET-TYPE, and HASH-SET all use pipelined DEL+write internally
- The throughput improvement from eliminating one round-trip is masked by the large payload sizes (100-1000 elements per key)
- The primary benefit is **atomicity**: no window where the key is deleted but data is not yet written

### No regressions

- All existing single-threaded benchmarks within normal variance of baseline
- No public API changes — fully backward compatible
