
#!/bin/bash
make
rm -fr /dev/shm/zfile
./nstore -x20000 -k10000 -e1 -w -y -p0.5 -u
rm -fr /dev/shm/zfile
./nstore -x20000 -k10000 -e2 -w -y -p0.5 -u -i
rm -fr /dev/shm/zfile
./nstore -x20000 -k10000 -e4 -w -y -p0.5 -u -i
rm -fr /dev/shm/zfile
./nstore -x20000 -k10000 -e4 -w -y -p0.8 -u -i
rm -fr /dev/shm/zfile


# On skylake
./nstore -x400000 -k40000 -w -t -p0.4 -e4 -n1 = tpcc-sl00
./nstore -x8000000 -k100000? -w -y -p0.8 -e4 -n1 = ycsb-sl01

