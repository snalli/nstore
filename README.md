# N-Store 

## Storage architectures

Evaluation of different storage architectures in database systems designed for non-volatile memory (NVM).
We thank Joy Arulraj for allowing us to use this code.
For more information on nstore, please read the original paper-

####Let's Talk About Storage & Recovery Methods for Non-Volatile Memory Database Systems.

Joy Arulraj, Andrew Pavlo, and Subramanya R. Dulloor. 2015.  
In Proceedings of the 2015 ACM SIGMOD International Conference on Management of
Data (SIGMOD '15)

## Dependencies

- **g++ 4.7+** 
- **autoconf** (`apt-get install autoconf libtool`) 

## To build:
        
```
    $./bootstrap
    $./configure
    $./build
    
    $ cd src/
    $ ./run -h                                          [For help]

   -h --help              :  Print help message 
   -x --num-txns          :  Number of transactions 
   -k --num-keys          :  Number of keys 
   -e --num-executors     :  Number of executors 
   -f --fs-path           :  Path for FS 
   -g --gc-interval       :  Group commit interval 
   -a --wal-enable        :  WAL enable (traditional) 
   -w --opt-wal-enable    :  OPT WAL enable 
   -s --sp-enable         :  SP enable (traditional) 
   -c --opt-sp-enable     :  OPT SP enable 
   -m --lsm-enable        :  LSM enable (traditional) 
   -l --opt-lsm-enable    :  OPT LSM enable 
   -y --ycsb              :  YCSB benchmark 
   -t --tpcc              :  TPCC benchmark 
   -d --test              :  TEST benchmark 
   -p --per-writes        :  Percent of writes 
   -u --ycsb-update-one   :  Update one field 
   -q --ycsb_zipf_skew    :  Zipf Skew 
   -z --storage_stats     :  Collect storage stats 
   -o --tpcc_stock-level  :  TPCC stock level only 
   -r --recovery          :  Recovery mode 
   -b --load-batch-size   :  Load batch size 
   -j --test_b_mode       :  Test benchmark mode 
   -i --multi-executors   :  Multiple executors 
   -n --enable-trace      :  E[n]able trace [default:0] (needs sudo)

```

# To run :

~~~
    $ cd src/
    $ ./run --ycsb --small                      [can pass --med or --large for bigger workloads]
    $ ./run --tpcc --small                      [can pass --med or --large for bigger workloads]
~~~

Nstore will create a persistent heap in /dev/shm defined by the macro PERSISTENT_HEAP.

To collect the trace of accesses to persistent memory,
make sure you have debugfs mounted in Linux.

# To run with tracing enabled :
~~~
    $ mount | grep debugfs
    debugfs on /sys/kernel/debug type debugfs (rw,relatime)
    
    $ sudo cd /sys/kernel/debug
    $ sudo echo 128000 > buffer_size_kb         [128MB. Allocate enough buffer to hold traces]
    $ cat trace_pipe                            [Redirect output of pipe to any file for storage]
    
    Go back to nstore/:
    
    $ cd src/
    $ sudo ./run --ycsb --small --trace         [Need to be root to collect trace]
~~~
