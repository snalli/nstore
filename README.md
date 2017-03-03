# N-Store 

An efficient database system designed for non-volatile memory (NVM).

For details, please read the original paper-

####Let's Talk About Storage & Recovery Methods for Non-Volatile Memory Database Systems.

Joy Arulraj, Andrew Pavlo, and Subramanya R. Dulloor. 2015.  
In Proceedings of the 2015 ACM SIGMOD International Conference on Management of
Data (SIGMOD '15)

# Dependencies:

- **g++ 4.7+** 
- **autoconf** (`apt-get install autoconf libtool`) 

# To build:
        
```
    $./bootstrap
    $./configure
    
    $ cd src/
    $./build
    $ ./run -h                                  [For help]

```

# To run :

~~~
    $ cd src/
    $ ./run --small --ycsb                      [can pass --med or --large for bigger workloads]
    $ ./run --small --tpcc                      [can pass --med or --large for bigger workloads]
~~~

Nstore will create a persistent heap in /dev/shm defined by the macro PERSISTENT_HEAP.
Ensure that you have at least 1.00 GB of space for the heap.

To collect the trace of accesses to NVM,
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
    $ sudo ./run --small --ycsb --trace         [Need to be root to collect trace]
~~~
