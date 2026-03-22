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

(2026-03-22)
```
[SET sequential]  count=100000, total=27926ms, qps=3580.89
[SET pipelined]   count=100000, batch=1000, total=555ms, qps=180180
[GET sequential]  count=100000, total=13840ms, qps=7225.43
[GET pipelined]   count=100000, batch=1000, total=561ms, qps=178253
[LIST-SET sequential]  keys=100000, list_len=1000, total=142021ms, qps=704.121
[LIST-SET pipelined]   keys=100000, list_len=1000, batch=1000, total=44515ms, qps=2246.43
[LIST-GET]   keys=100000, list_len=1000, total=45458ms, qps=2199.83
[SET-TYPE sequential]  keys=100000, set_size=100, total=129806ms, qps=770.38
[SET-TYPE GET]   keys=100000, set_size=100, total=25469ms, qps=3926.34
[HASH-SET sequential]  keys=100000, fields=100, total=151546ms, qps=659.866
[HASH-GET]   keys=100000, fields=100, total=23627ms, qps=4232.45
[SET multi-thread] threads=4, count=100000, total=4448ms, qps=22482
[GET multi-thread] threads=4, count=100000, total=4940ms, qps=20242.9
[POOL TEST] size=1, threads=8, total=15388ms, qps=6498.57
[POOL TEST] size=2, threads=8, total=10786ms, qps=9271.28
[POOL TEST] size=4, threads=8, total=8749ms, qps=11429.9
[POOL TEST] size=8, threads=8, total=5637ms, qps=17739.9
[POOL TEST] size=16, threads=8, total=5592ms, qps=17882.7
```
