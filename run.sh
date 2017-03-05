#!/bin/bash
action=$1
var=$2
trace=$3

bin="./src/nstore"

if [ "$action" == "-h" ] 
then
	$bin -h
	exit
fi

if [ "$trace" == "--trace" ]
then
	trace="-n1"
	sudo="sudo"
else
	trace="-n0"
fi

if [ "$var" == "--ycsb" ]
then

	if [ "$action" == "--small" ]
	then
		$sudo $time $bin -x1000 -k10000 -w -p0.5 -e2 $trace $var
	elif [ "$action" == "--med" ]
	then
		$sudo $time $bin -x20000 -k50000 -w -p0.5 -e2 $trace $var
	elif [ "$action" == "--large" ]
	then
		$sudo $time $bin -x40000 -k100000 -w -p0.8 -e4 $trace $var
	fi

elif [ "$var" == "--tpcc" ]
then
	if [ "$action" == "--small" ]
	then
		$sudo $time $bin -x10000 -k1000 -w -p0.2 -e2 $trace $var
	elif [ "$action" == "--med" ]
	then
		$sudo $time $bin -x20000 -k2000 -w -p0.2 -e2 $trace $var
	elif [ "$action" == "--large" ]
	then
		$sudo $time $bin -x40000 -k4000 -w -p0.4 -e4 $trace $var
	fi
else
	echo "Invalid workload $var"
fi

#!/bin/bash
#./src/nstore -x20000 -k10000 -e1 -w -y -p0.5 -n1
#./src/nstore -x20000 -k10000 -e2 -w -y -p0.5 -i
#./src/nstore -x20000 -k10000 -e4 -w -y -p0.5 -i
#./src/nstore -x20000 -k10000 -e4 -w -y -p0.8 -i

# On skylake
#./src/nstore -x400000 -k40000 -w -t -p0.4 -e4 -n1 = tpcc-sl00
#./src/nstore -x8000000 -k100000 -w -y -p0.8 -e4 -n1 = ycsb-sl01
