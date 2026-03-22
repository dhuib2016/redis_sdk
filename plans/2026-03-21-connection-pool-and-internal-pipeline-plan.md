# Optimization Plan: Connection Pooling + Internal Pipeline Batching

**Date**: 2026-03-21
**Goal**: Improve QPS across all benchmarks by enabling redis-plus-plus built-in connection pooling and reducing round-trips in compound operations.

## Baseline (100k operations)

| Benchmark | QPS |
|---|---|
| SET sequential | 1,278 |
| SET pipelined | 15,186 |
| GET sequential | 1,284 |
| GET pipelined | 15,664 |
| LIST-SET sequential | 507 |
| LIST-SET pipelined | 1,359 |
| LIST-GET sequential | 998 |

## Identified Bottlenecks

### 1. Single connection (no pooling)
`connect()` creates a bare `sw::redis::Redis` with no `ConnectionPoolOptions`. redis-plus-plus supports an internal thread-safe connection pool. With `pool_size > 1`, multiple threads can issue commands concurrently, and even single-threaded use benefits from connection reuse optimizations.

### 2. Compound SetData does 2 round-trips
`SetData(key, vector)` with `append=false` does `DEL` then `RPUSH` as two separate commands — 2 network round-trips. Same for set/map variants. Using an internal pipeline reduces this to 1 round-trip.

### 3. No multi-threaded benchmark
Current benchmarks are single-threaded. Connection pooling's main benefit is concurrent access from multiple threads.

## Changes

### Change 1: Enable ConnectionPoolOptions in connect() and confConnect()

**Files**: `src/redis_client.cpp`, `include/redis_client.h`

In `connect(const std::string& uri)`:
- Parse the URI into `ConnectionOptions` using the existing redis-plus-plus URI constructor
- Create `ConnectionPoolOptions` with `pool_size = 4` (sensible default)
- Construct `Redis` with both options

In `confConnect()`:
- Add `ConnectionPoolOptions` with same pool_size
- Pass to Redis constructor

In `redis_client.h`:
- Add `int pool_size_ = 4;` member for configurability

### Change 2: Internal pipeline for compound SetData operations

**File**: `src/redis_client.cpp`

For `SetData(key, vector, append=false)`:
```cpp
// Before: 2 round trips
redis_->del(key);
redis_->rpush(key, value.begin(), value.end());

// After: 1 round trip via pipeline
auto pipe = redis_->pipeline();
pipe.del(key);
pipe.rpush(key, value.begin(), value.end());
pipe.exec();
```

Apply the same pattern to:
- `SetData(key, set<string>, append=false)` — pipeline DEL + SADD
- `SetData(key, map<string,string>, append=false)` — pipeline DEL + HMSET

### Change 3: Add multi-threaded benchmarks to test/main.cpp

**File**: `test/main.cpp`

Add benchmarks that spawn N threads (e.g., 4) each doing sequential SET/GET operations against the same RedisClient. This demonstrates the connection pool's concurrent throughput. Use the same key-count-per-thread to keep comparison fair.

Add:
- `[SET multi-thread]` — 4 threads × 25000 ops each = 100k total
- `[GET multi-thread]` — same pattern

## Expected Impact

- **Sequential SET/GET**: Modest improvement from pool connection reuse (~5-15%)
- **SetData compound ops**: ~30-50% improvement from eliminating extra round-trip
- **Multi-threaded**: Near-linear scaling with thread count (expected 3-4x over sequential)
- **Pipelined ops**: Minimal change (already batched)

## Risk Assessment

- **API compatibility**: No public API changes. `pool_size_` is private. All existing method signatures preserved.
- **Thread safety**: redis-plus-plus `Redis` with connection pool is already thread-safe.
- **Backward compat**: Default pool_size=4 works for all existing use cases.
