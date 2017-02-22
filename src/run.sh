#!/bin/bash
./nstore -x20000 -k10000 -e1 -w -y -p0.5 -n1
./nstore -x20000 -k10000 -e2 -w -y -p0.5 -i
./nstore -x20000 -k10000 -e4 -w -y -p0.5 -i
./nstore -x20000 -k10000 -e4 -w -y -p0.8 -i

# On skylake
./nstore -x400000 -k40000 -w -t -p0.4 -e4 -n1 = tpcc-sl00
./nstore -x8000000 -k100000 -w -y -p0.8 -e4 -n1 = ycsb-sl01
