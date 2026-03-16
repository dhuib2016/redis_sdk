## Redis_sdk
performance test (2026-03-10)
```
$ ./test_app
[SET]   count=100000, total=16004 ms, qps=6248.44
[GET]   count=100000, total=16385 ms, qps=6103.14
[LIST-SET]   keys=100000, list_len=1000, total=122491 ms, qps=816.387
[LIST-GET]   keys=100000, list_len=1000, total=53749 ms, qps=1860.5
```

(2026-03-16)

```
$ ./test_app
[SET sequential]  count=100000, total=20017ms, qps=4995.75
[SET pipelined]   count=100000, batch=1000, total=424ms, qps=235849
[GET sequential]  count=100000, total=10668ms, qps=9373.83
[GET pipelined]   count=100000, batch=1000, total=347ms, qps=288184
[LIST-SET sequential]  keys=100000, list_len=1000, total=59331ms, qps=1685.46
[LIST-SET pipelined]   keys=100000, list_len=1000, batch=1000, total=37108ms, qps=2694.84
[LIST-GET]   keys=100000, list_len=1000, total=44223ms, qps=2261.27
```
