# N-Store 

## Storage architectures

Evaluation of different storage architectures in database systems designed for non-volatile memory (NVM).
We thank Joy Arulraj for allowing us to use this code.
For more information on nstore, please read the original paper-

Joy Arulraj, Andrew Pavlo, and Subramanya R. Dulloor. 2015. Let's Talk About
Storage & Recovery Methods for Non-Volatile Memory Database Systems. In
Proceedings of the 2015 ACM SIGMOD International Conference on Management of
Data (SIGMOD '15). ACM, New York, NY, USA, 707-722. DOI:
http://dx.doi.org/10.1145/2723372.2749441

## Dependencies

- **g++ 4.7+** 
- **autoconf** (`apt-get install autoconf libtool`) 


## Easy install and run
Use our install and run scripts to make your lives easier!

We have provided a simple install.py which handles installation and cleans of
the individual workloads. To clean and/or build nstore, simply do:

    python install.py [--build] [--clean]

For running either ycsb or tpcc workload with nstore,
    run.py [-h] [--sim_size SIM_SIZE] [ycsb/tpcc]

Supported simulation sizes: test, small, medium, large

## Manual Setup
        
```
    ./bootstrap
    ./configure
    make
```

## Test

```
./src/n-store -h 
```

##Command line parameters for running nstore
.Command line options : nstore <options> 
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
