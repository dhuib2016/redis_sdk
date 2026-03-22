# Redis SDK (msgsdk)

## Project Overview

A C++ Redis client wrapper library built on top of redis-plus-plus / hiredis.
Produces a static library `libredis_sdk.a` consumed by downstream services.

## Project Structure

```
├── include/          # Public headers
│   ├── redis_client.h
│   ├── redis_pipeline.h
│   ├── redis_pool.h
│   ├── redis_subscriber.h
│   └── RedisEvent.h
├── src/              # Implementation files
│   ├── redis_client.cpp
│   ├── redis_pool.cpp
│   ├── redis_subscriber.cpp
│   └── RedisEvent.cpp
├── test/
│   ├── main.cpp      # Benchmarks
│   └── eventtest.cpp
├── Makefile
```

## Build & Run

```bash
make clean && make          # build libredis_sdk.a
make test                   # compile test/main.cpp -> test_app
./test_app                  # run benchmarks
```

## Dependencies

- **Compiler**: g++, C++17, pthread
- **Libraries**: redis-plus-plus (`-lredis++`), hiredis (`-lhiredis`)
- **Install prefix**: `~/.local` (headers in `~/.local/include`, libs in `~/.local/lib64` and `~/.local/lib`)

## Code Conventions

- Read `src/` and `include/` to understand existing patterns before making any changes
- Do not change public method signatures unless explicitly planned
- Follow whatever style conventions are already in the codebase

## Redis Optimization Workflow

When optimizing this library:

1. **Baseline**: delegate to `benchmarker` — run `make clean && make && make test && ./test_app`, capture QPS numbers
2. **Plan**: read and analyze `src/` and `include/`, identify bottlenecks, write a detailed plan to `./plans/<date>-<feature>-plan.md`
3. **Implement**: delegate to `implementer` — pass the plan file path, edits go in `src/` and `include/`
4. **Verify build**: delegate to `benchmarker` — run `make clean && make && make test` to confirm it compiles
5. **Measure**: delegate to `benchmarker` — run `./test_app`, capture post-optimization QPS
6. **Evaluate**: main agent compares before/after QPS, checks for API breakage or regressions

## Subagent Delegation

- Use `implementer` for all code changes under `src/` and `include/`
- Use `benchmarker` for building (`make`), running (`./test_app`), and analyzing output
- Main agent owns planning, code review, and final approval
- Communicate between agents via files in `./plans/` — pass file paths, not inline content
