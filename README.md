## Redis_sdk
performance test (2026-03-10)
```
$ ./test_app
[SET]   count=100000, total=16004 ms, qps=6248.44
[GET]   count=100000, total=16385 ms, qps=6103.14
[LIST-SET]   keys=100000, list_len=1000, total=122491 ms, qps=816.387
[LIST-GET]   keys=100000, list_len=1000, total=53749 ms, qps=1860.5
```
