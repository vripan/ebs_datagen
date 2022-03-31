# Performance report
## CPU Info: 
    Name: 11th Gen Intel(R) Core(TM) i7-11850H @ 2.50GHz
    Cores: 8
    Logical processors: 16
    L1 cache: 640 KB
    L2 cache: 10.0 MB
    L3 cache: 24.0 MB
## Benchmark
    publications count: 1000000 
    subscriptions count: 1000000
    configuration: release
    commit: b0ba0a3fd722727e68c4c5865248646170c555d3
    threads:  1 / 20511 ms
              2 / 6472 ms
              4 / 4512 ms
              8 / 3851 ms
             16 / 4915 ms

## Raw output
    $ ebs_datagen.exe --schema schema.json --threads 1 --log info 
        13:14:07.601 -  info - thread 31604 - running ebs data generator
        13:14:07.601 -  info - thread 31604 - schema file "schema.json"
        13:14:07.601 -  info - thread 31604 - running on 1 threads
        13:14:07.601 -  info - thread 31604 - schema loaded, 861 bytes
        13:14:07.602 -  info - thread 31604 - input size: 1000000 publications and 1000000 subscriptions
        13:14:28.114 -  info - thread 31604 - data generation duration 20511 ms

    $ ebs_datagen.exe --schema schema.json --threads 2 --log info 
        13:14:28.121 -  info - thread 16252 - running ebs data generator
        13:14:28.122 -  info - thread 16252 - schema file "schema.json"
        13:14:28.122 -  info - thread 16252 - running on 2 threads
        13:14:28.122 -  info - thread 16252 - schema loaded, 861 bytes
        13:14:28.138 -  info - thread 16252 - input size: 1000000 publications and 1000000 subscriptions
        13:14:34.611 -  info - thread 16252 - data generation duration 6472 ms

    $ ebs_datagen.exe --schema schema.json --threads 4 --log info 
        13:14:34.618 -  info - thread 17012 - running ebs data generator
        13:14:34.619 -  info - thread 17012 - schema file "schema.json"
        13:14:34.619 -  info - thread 17012 - running on 4 threads
        13:14:34.619 -  info - thread 17012 - schema loaded, 861 bytes
        13:14:34.636 -  info - thread 17012 - input size: 1000000 publications and 1000000 subscriptions
        13:14:39.148 -  info - thread 17012 - data generation duration 4512 ms

    $ ebs_datagen.exe --schema schema.json --threads 8 --log info 
        13:14:39.155 -  info - thread 24508 - running ebs data generator
        13:14:39.155 -  info - thread 24508 - schema file "schema.json"
        13:14:39.155 -  info - thread 24508 - running on 8 threads
        13:14:39.155 -  info - thread 24508 - schema loaded, 861 bytes
        13:14:39.172 -  info - thread 24508 - input size: 1000000 publications and 1000000 subscriptions
        13:14:43.024 -  info - thread 24508 - data generation duration 3851 ms

    $ ebs_datagen.exe --schema schema.json --threads 16 --log info 
        13:14:43.030 -  info - thread 12356 - running ebs data generator
        13:14:43.031 -  info - thread 12356 - schema file "schema.json"
        13:14:43.031 -  info - thread 12356 - running on 16 threads
        13:14:43.031 -  info - thread 12356 - schema loaded, 861 bytes
        13:14:43.047 -  info - thread 12356 - input size: 1000000 publications and 1000000 subscriptions
        13:14:47.962 -  info - thread 12356 - data generation duration 4915 ms
